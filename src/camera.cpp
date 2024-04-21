#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <set>

// Tiles per second, assuming 60 fps
const static float MOVE_SPEED = 1.5f;
const static float MOUSE_ROTATION_SPEED = 0.001f;
const static float KEY_ROTATION_SPEED = 0.03f;
const static float CAMERA_BOUND = 0.05f;

Camera::Camera(Input& input) : i(input) {
    reset();
}
Camera::~Camera() {}

void Camera::update(Maze& m) {
    static int framesSinceWon;
    static glm::vec2 winPos;
    // Sloppy little animation for when player hits end tile
    if (endAnim) {
        // Want to take 60 frames to get to middle of end tile
        // Interpolate position from first position player hit end tile
        // at to middle of end tile, and accelerate upwards for fun
        // while renderer plays fade-out effect
        static const auto end = m.getEnd();
        static const glm::vec2 middleEnd = {end.x + 0.5f, end.y + 0.5f};
        if (framesSinceWon < 60) {
            float t = (float) framesSinceWon/60.0f;
            pos = {(1.0f - t) * winPos + t * middleEnd, pos.z};
        }

        looking = glm::rotate(looking, framesSinceWon * 0.0003f, up);
        view = glm::lookAt(pos, pos + looking, up);
        pos.z += framesSinceWon * 0.0001f;

        framesSinceWon++;
        processRotations();
        view = glm::lookAt(pos, pos + looking, up);
        return;
    }

    processRotations();
    glm::vec3 movement = processMovement();
    if (collision)
        movement = processCollision(m, movement);

    pos += movement;
    view = glm::lookAt(pos, pos + looking, up);

    if (m.isEnd({(int) pos.x, (int) pos.y})) {
        endAnim = true;
        framesSinceWon = 0;
        winPos = {pos.x, pos.y};
    }

}

glm::mat4 Camera::getView() {
    return view;
}

glm::vec2 Camera::getPos() {
    return glm::vec2(pos.x, pos.y);
}

glm::vec3 Camera::processMovement() {
    // Divide by fps to get tiles per second
    const float speed = (MOVE_SPEED / 60.0f);

    glm::vec3 proposedMovement = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 forwards = speed * glm::normalize(
            glm::vec3(looking.x, looking.y, 0.0f));
    glm::vec3 sideways = speed * glm::cross(looking, up);
    if (i.getKey('w'))
        proposedMovement += forwards;
    if (i.getKey('s'))
        proposedMovement -= forwards;
    if (i.getKey('a'))
        proposedMovement -= sideways;
    if (i.getKey('d'))
        proposedMovement += sideways;
    if (i.getJust('c'))
        collision = !collision;
    return proposedMovement;
}

glm::vec3 Camera::processCollision(Maze& m, glm::vec3 proposedMovement) {
    Tile* current;
    glm::vec3 nextPos;
    float dist, currentDist;
    std::set<Face*> faces;

    int gridSizeX = m.getGrid().size();
    int gridSizeY = m.getGrid()[0].size();
    int lowerX = (int) pos.x >= 2 ? (int) pos.x - 2 : 0;
    int upperX = (int) pos.x < gridSizeX - 2 ? (int) pos.x + 2 : gridSizeX - 1;
    int lowerY = (int) pos.y >= 2 ? (int) pos.y - 2 : 0;
    int upperY = (int) pos.y < gridSizeY - 2 ? (int) pos.y + 2 : gridSizeY - 1;

    for (int i = lowerX; i <= upperX; ++i)
        for (int j = lowerY; j <= upperY; ++j)
            for (int f : m.facesAt(i, j))
                faces.insert(&m.mesh[f]);

    for (Face* f : faces) {
        nextPos = pos + proposedMovement;

        if (lineAABBCollision(nextPos, *f)) {
            // Collision resolution - find the right position player
            // should be at through some linear algebra
            dist = distFromPointToLine(
                    nextPos.x, nextPos.y,
                    f->points[0].x, f->points[0].y,
                    f->points[1].x, f->points[1].y);
            dist = fabs(dist - CAMERA_BOUND);

            currentDist = distFromPointToLine(
                    pos.x, pos.y,
                    f->points[0].x, f->points[0].y,
                    f->points[1].x, f->points[1].y);

            glm::vec3 relFromWall = f->normal * currentDist;
            //if (glm::dot(relFromWall, f.normal) < 0)
            if (currentDist < 0.045)
                continue;

            proposedMovement += f->normal * dist;
        }
    }
    return proposedMovement;
}

void Camera::processRotations() {
    glm::vec2 movement = i.getMovement();

    // Horizontal movement
    if (i.getKey('e'))
        looking = glm::rotate(looking, -KEY_ROTATION_SPEED, up);
    if (i.getKey('q'))
        looking = glm::rotate(looking, KEY_ROTATION_SPEED, up);
    if (movement.x != 0)
        looking = glm::rotate(looking, -MOUSE_ROTATION_SPEED * movement.x,
                              up);
    // Vertical movement
    if (movement.y != 0) {
        glm::vec3 proposed;
        glm::vec3 horizLooking = glm::normalize(glm::vec3(looking.x, looking.y, 0.0f));
        proposed = glm::rotate(looking, MOUSE_ROTATION_SPEED * movement.y, 
                               glm::cross(up, looking));

        // Cap vertical camera rotation
        float dotProposed = glm::dot(proposed, up);
        if (fabs(dotProposed) < 0.9f) {
            looking = proposed;
        } else if (dotProposed <= -0.9f) {
            looking = glm::rotate(horizLooking, asinf(0.9f), 
                                  glm::cross(up, looking));
        } else {
            looking = glm::rotate(horizLooking, asinf(-0.9f), 
                                  glm::cross(up, looking));
        }
    }
}

// 2D collision of line (2D representation of a wall face in the maze)
// and an AABB (axis-aligned bounding box - the camera).
bool Camera::lineAABBCollision(glm::vec3 newPos, Face& f) {
    float myMinX, myMaxX, myMinY, myMaxY;
    float itMinX, itMaxX, itMinY, itMaxY;
    myMinX = newPos.x - CAMERA_BOUND;
    myMaxX = newPos.x + CAMERA_BOUND;
    myMinY = newPos.y - CAMERA_BOUND;
    myMaxY = newPos.y + CAMERA_BOUND;

    itMinX = std::min(f.points[0].x, f.points[1].x);
    itMaxX = std::max(f.points[0].x, f.points[1].x);
    itMinY = std::min(f.points[0].y, f.points[1].y);
    itMaxY = std::max(f.points[0].y, f.points[1].y);

    return (myMinX <= itMaxX && myMaxX >= itMinX) &&
        (myMinY <= itMaxY && myMaxY >= itMinY);
}

float Camera::distFromPointToLine(
        float x0, float y0,
        float x1, float y1, 
        float x2, float y2) {
    glm::vec2 v(y2 - y1, -(x2 - x1));
    glm::vec2 r(x1 - x0, y1 - y0);
    v = glm::normalize(v);
    return fabs(glm::dot(v, r));
}

void Camera::reset() {
    pos = glm::vec3(1.5f, 1.5f, 1.7f);
    up = glm::vec3(0.0f, 0.0f, 1.0f);
    looking = glm::vec3(0.0f, 1.0f, 0.0f);

    view = glm::lookAt(pos, pos + looking, up);
    collision = true;
    endAnim = false;
}
