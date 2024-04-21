#ifndef MINIMAP_H
#define MINIMAP_H

/*
 * Minimap - displays in bottom left hand corner of screen,
 * toggled with 'm' hotkey. Walls show as transparent. Player
 * position is marked, along with exit. Hotkey 'p' toggles
 * showing optimal path to maze exit. Tiles player has visited
 * are marked red.
 *
 * Minimap is held as a texture drawn onto a 2D quad. Drop shadow
 * is applied via shader.
 */

#include <glm/glm.hpp>
#include <vector>

#include "maze.h"

typedef glm::ivec4 Color;

class Minimap {
    public:
        Minimap(Maze& m, int screenW, int screenH);
        ~Minimap();

        void togglePath();
        void update(glm::vec2 pos);
        void reshape(int w, int h);
        void reset(Maze& m);
        void toggle();
        bool enabled();

        std::vector<float> getVertices();
        unsigned char* getTexture();
        float getShadowSize();
        bool needsUpdate();
        int getWidth();
        int getHeight();

    private:
        /* Width/height of minimap squares when drawn to screen */
        int pxPerTile;

        /* Width and height of minimap texture - set to 32 */
        int mapW;
        int mapH;
        /* Width and height of backing maze, for convenience */
        int mazeW;
        int mazeH;

        /* Width and height of entire map texture, scaled to nearest *
         * power of two to play friendly with OpenGL texture loading */
        int texW;
        int texH;

        Maze* maze;             // Backing maze
        glm::vec2 lastPos;      // Last position player was at
        unsigned char* texture; // Texture of maze
        /* Minimap texture (tiles within range only) */
        unsigned char* localTexture; 
        std::vector<float> vertices;
        std::vector<glm::ivec2> floorPoints; // For toggling optimal path
        std::vector<glm::ivec2> visited;     // Tiles visited by player

        /* Is minimap hidden? - toggled with 'm' */
        bool hidden;
        /* Is optimal path shown? - toggled with 'p' */
        bool pathStatus;
        /* Has map changed? -> Push update to renderer */
        bool needUpdate;

        void loadBaseVerts();   // Load base minimap quad
        /* Set pixel (x, y) on minimap texture to color (r, g, b) */
        Color set(int x, int y, int r, int g, int b);
};

#endif
