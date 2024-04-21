#ifndef WORLD_H
#define WORLD_H

/*
 * World - contains a maze, camera, minimap. Handles some option toggling
 * with input. Purely header since it is so small.
 */

#include <glm/glm.hpp>
#include "maze.h"
#include "camera.h"
#include "minimap.h"

class World {
public:
    World(int w, int h, int mazeW, int mazeH) : 
        maze(mazeW, mazeH),
        camera(input),
        minimap(maze, w, h) {}
    ~World() {}

    void tick() {
        if (input.getJust('m'))
            minimap.toggle();
        if (input.getJust('p'))
            minimap.togglePath();
        if (input.getJust('z'))
            exit(0);
        camera.update(maze);
        minimap.update(camera.getPos());
    }

    void reset() {
        maze.reset();
        camera.reset();
        minimap.reset(maze);
    }

    glm::mat4 getView() {
        return camera.getView();
    }

    glm::vec2 getPos() {
        return camera.getPos();
    }

    Maze& getMaze() {
        return maze;
    }

    Minimap& getMinimap() {
        return minimap;
    }

private:
    Maze maze;
    Input input;
    Camera camera;
    Minimap minimap;
};

#endif
