#include "minimap.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <map>

#include "maze.h"

static const Color PATH_COLOR(150, 150, 150, 255);
static const Color WALL_COLOR(0, 0, 0, 0);
static const Color FLOOR_COLOR(255, 255, 255, 255);
static const Color ENTRANCE_COLOR(0, 200, 50, 255);
static const Color EXIT_COLOR(0, 255, 255, 255);
static const Color EXPLORED(110, 20, 20, 255);

// Map tile types to colors
static std::map<Type, Color> COLOR_MAP = {
    {Type::Wall, WALL_COLOR},
    {Type::Floor, FLOOR_COLOR},
    {Type::Path, FLOOR_COLOR},
    {Type::Entrance, ENTRANCE_COLOR},
    {Type::Exit, EXIT_COLOR}
};

static float BASE_VERTS[] = {
/*  Color               TexCoords   */
    1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
    1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,   0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,   0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,   0.0f, 0.0f
};

Minimap::Minimap(Maze& m, int screenW, int screenH) {
    pathStatus = hidden = false;
    reset(m);
    reshape(screenW, screenH); 
}

// Must clear up memory allocated to textures
Minimap::~Minimap() {
    delete[] texture;
    delete[] localTexture;
}

void Minimap::reset(Maze& m) {
    maze = &m;
    TileGrid tiles = m.getGrid();
    mazeW = tiles.size();
    mazeH = tiles[0].size();
    // Align texture size to nearest power of 2
    texW = pow(2, ceil(log2(mazeW)));
    texH = pow(2, ceil(log2(mazeH)));
    mapW = 32;
    mapH = 32;
    lastPos = {1, 1};
    visited.clear();
    floorPoints.clear();

    texture = new unsigned char[texW*texH*4];
    localTexture = new unsigned char[mapW*mapH*4];
    Color color;

    for (int i = texH - 1; i >= 0; --i) {
        for (int j = 0; j < texW; ++j) {
            if (j < mazeW && i < mazeH) {
                color = COLOR_MAP[tiles[j][i].type];
                if (tiles[j][i].type == Type::Floor)
                    floorPoints.push_back({j, i});
            }
            texture[4*(i*texW + j)] = color.x;
            texture[4*(i*texW + j)+1] = color.y;
            texture[4*(i*texW + j)+2] = color.z;
            texture[4*(i*texW + j)+3] = color.w;
        }
    }

    needUpdate = true;
}

void Minimap::togglePath() {
    // Toggle color in tile -> color map
    Color color = pathStatus ? FLOOR_COLOR : PATH_COLOR;
    COLOR_MAP[Type::Path] = color;
    pathStatus = !pathStatus;
    for (auto& v : floorPoints)
        set(v.x, v.y, color.x, color.y, color.z);

    color = EXPLORED;
    for (auto& v : visited)
        set(v.x, v.y, color.x, color.y, color.z);
}

void Minimap::update(glm::vec2 pos) {
    if (glm::ivec2(pos.x, pos.y) == glm::ivec2(lastPos.x, lastPos.y))
        return;
    visited.push_back({(int) pos.x, (int) pos.y});
    auto& color = EXPLORED;

    // Must update previous & current positions
    set((int) lastPos.x, (int) lastPos.y, color.x, color.y, color.z);
    set((int) pos.x, (int) pos.y, 255, 0, 0);
    lastPos = pos;
    needUpdate = true; // Make sure update is propagated to renderer
}

// Shadow size in texture coordinates
float Minimap::getShadowSize() {
    // Scale number of pixels to minimap size on screen
    int shadowPixels = (float) pxPerTile / 3.0f;
    // Map pixel shadow size to texture size
    return ((float) shadowPixels) *
        ((1.0f / (float) mapW) / (float) pxPerTile);
}

// Update local minimap texture if necessary. 
unsigned char* Minimap::getTexture() {
    if (needUpdate) {
        // Scrolling minimap - only shows 32x32 area around player
        const int offX = (int) (std::max(0, mapW - mazeW) / 2);
        const int offY = (int) (std::max(0, mapH - mazeH) / 2);
        const int midPosX = std::max(1, std::min(mazeW - mapW + 1,
                    (int) lastPos.x + 1 - mapW / 2));
        const int midPosY = std::max(1, std::min(mazeH - mapH + 1,
                    (int) lastPos.y + 1 - mapH / 2));
        const int topLeft = 4*(((int)midPosY - 1)*texW + (int)midPosX - 1);

        // Update row-by-row within square around player, but not past
        // edges
        for (int i = 0; i < mapH; ++i) {
            memcpy(localTexture + 4*i*mapW,
                    texture + topLeft + 4*i*texW,
                    mapW * 4 * sizeof(unsigned char));
        }

        needUpdate = false;
    }

    return localTexture;
}

void Minimap::reshape(int w, int h) {
    loadBaseVerts();
    pxPerTile = (float) w / 100.0f; // Scale minimap size to screen width
    glm::mat4 proj = glm::ortho(0.0f, (float) w, 0.0f, (float) h);
    float scaleX = std::min(1.0f, (float) mazeW / (float) mapW);
    float scaleY = std::min(1.0f, (float) mazeH / (float) mapH);
    glm::mat4 scale = glm::scale(glm::mat4(), 
        glm::vec3(scaleX * pxPerTile * mapW, 
                  scaleY * pxPerTile * mapH, 1.0f));
    glm::mat4 trans = 
        glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::vec4 scaledPoint;
    scale = proj * trans * scale;

    // Reshape minimap model to still draw on screen as square
    for (int i = 0; i < vertices.size(); i += 5) {
        scaledPoint = scale * glm::vec4(
                vertices[i], vertices[i+1], vertices[i+2], 1.0f);
        vertices[i] = scaledPoint.x;
        vertices[i+1] = scaledPoint.y;
        vertices[i+2] = scaledPoint.z;
        if (vertices[i+3] > 0.1f)
            vertices[i+3] = scaleX;
        if (vertices[i+4] > 0.1f)
            vertices[i+4] = scaleY;
    }
}

Color Minimap::set(int x, int y, int r, int g, int b) {
    int loc = 4*(y*texW + x);
    // don't change transparent tiles (walls)
    if (texture[loc+3] != 0) {
        texture[loc+0] = r;
        texture[loc+1] = g;
        texture[loc+2] = b;
    }
    return {texture[loc+0], texture[loc+1],
            texture[loc+2], texture[loc+3]};
}

void Minimap::loadBaseVerts() {
    vertices.resize(sizeof(BASE_VERTS)/sizeof(float));
    for (int i = 0; i < vertices.size(); ++i) {
        vertices[i] = BASE_VERTS[i];
    }
}

std::vector<float> Minimap::getVertices() {
    return vertices;
}

int Minimap::getWidth() {
    return mapW;
}

int Minimap::getHeight() {
    return mapH;
}

void Minimap::toggle() {
    hidden = !hidden;
}

bool Minimap::enabled() {
    return !hidden;
}

bool Minimap::needsUpdate() {
    return needUpdate;
}

