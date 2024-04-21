#ifndef CAMERA_H
#define CAMERA_H

/*
 * Camera - handles player movement, outputs view matrix to renderer
 */

#include <glm/glm.hpp>
#include <iostream>

#include "input.h"
#include "maze.h"

class Camera {
public:
    Camera(Input&);
    ~Camera();

    void update(Maze& m);
    void reset();
    glm::mat4 getView();
    glm::vec2 getPos();

private:
    Input& i;
    glm::mat4 view;
    glm::vec3 pos;
    glm::vec3 up;
    glm::vec3 looking;
    bool collision;
    bool endAnim;

    glm::vec3 processMovement();
    glm::vec3 processCollision(Maze& m, glm::vec3 proposedMovement);
    void processRotations();
    bool lineAABBCollision(glm::vec3 newPos, Face& f);
    float distFromPointToLine(
            float x0, float y0,
            float x1, float y1, 
            float x2, float y2);
};

#endif
