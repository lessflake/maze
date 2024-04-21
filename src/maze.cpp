#include "maze.h"

#include <stack>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <cmath>
#include <time.h>

Maze::Maze(int width, int height) {
    srand((long) time(0));
    tiles.resize(width*2 + 1);
    for (int i = 0; i < tiles.size(); i++)
        tiles[i].resize(height*2 + 1);

    build();
}

void Maze::build() {
    for (int i = 0; i < tiles.size(); i++)
        for (int j = 0; j < tiles[i].size(); j++)
            tiles[i][j].type = Type::Wall;

    end = {tiles.size() - 2, tiles[0].size() - 2};
    tiles[1][1].type = Type::Entrance;
    tiles[end.x][end.y].type = Type::Exit;

    winState = false;
    exitFound = false;
    DFS(1, 1);

    // Put optimal path into maze grid as found by DFS
    auto exit = glm::ivec2(tiles.size() - 2, tiles[0].size() - 2);
    auto entrance = glm::ivec2(1, 1);
    auto& current = exit;
    blendAdjacent(current, path[current], Type::Path);
    current = path[current];
    while (current != entrance) {
        tiles[current.x][current.y].type = Type::Path;
        blendAdjacent(current, path[current], Type::Path);
        current = path[current];
    }
    path.clear();

    // Construct collision mesh
    // For every cube, find the side faces that do not face other tiles
    // Four faces per cube to be considered
    // Up vector is Z.
    faceLookup.clear();
    Tile *t, *facing;
    Face f;
    int s;
    for (int i = 0; i < tiles.size(); ++i) { 
        for (int j = 0; j < tiles[0].size(); ++j) {
            if (tiles[i][j].type != Type::Wall)
                continue;
            t = &tiles[i][j];

            if (i > 0)
                addFace(i, j, -1, 0);
            if (j > 0)
                addFace(i, j, 0, -1);
            if (i < tiles.size() - 1)
                addFace(i, j, 1, 0);
            if (j < tiles[i].size() - 1)
                addFace(i, j, 0, 1);
        }
    }
}

// Add face to collision mesh given its x/y offsets from the center
// tile (tX, tY)
void Maze::addFace(int tX, int tY, int offX, int offY) {
    assert(abs(offX) <= 1 && abs(offY) <= 1);
    if (tiles[tX + offX][tY + offY].type != Type::Wall) {
        int sX = offX > 0 ? 1 : 0;
        int sY = offY > 0 ? 1 : 0;
        Dir dir = offX > 0 ? Dir::East  :
                  offY > 0 ? Dir::North :
                  offX < 0 ? Dir::West  :
                             Dir::South;
        mesh.push_back({
            glm::vec2(tX, tY),
            glm::vec3(tX + sX, tY + sY, 0.0f),
            glm::vec3(tX + sX + abs(offY), tY + sY + abs(offX), 0.0f),
            glm::vec3(offX, offY, 0.0f),
            dir
        });
        lookupInsert(tX + offX, tY + offY, mesh.size() - 1);
    }
}

TileGrid& Maze::getGrid() {
    return tiles;
}

Tile& Maze::getTile(int x, int y) {
    return tiles[x][y];
}

// DFS jumps 2 tiles to leave tiles to be walls. This function is
// to set the tile inbetween the jump.
void Maze::blendAdjacent(glm::ivec2 a, glm::ivec2 b, Type t) {
    assert((a.x + b.x) % 2 == 0 && (a.y + b.y) % 2 == 0);
    if (a.x != b.x) {
        tiles[(a.x + b.x)/2][a.y].type = t;
    } else {
        tiles[a.x][(a.y + b.y)/2].type = t;
    }
}

void Maze::lookupInsert(int x, int y, int i) {
    int key = y*tiles.size() + x;
    if (faceLookup.find(key) == faceLookup.end()) {
        faceLookup[key] = std::vector<int>({i});
    } else faceLookup[key].push_back(i);
}

std::vector<int> Maze::facesAt(int x, int y) {
    int key = y*tiles.size() + x;
    if (faceLookup.find(key) != faceLookup.end())
        return faceLookup[y*tiles.size() + x];
    else return std::vector<int>();
}

// Returns vector of adjacent (north, south, east, west) tiles from
// a given tile
std::vector<glm::ivec2> Maze::getAdjacents(glm::ivec2 dbt) {
        std::vector<glm::ivec2> adjacent;
        if (dbt.x < tiles.size() - 2)
            adjacent.push_back({dbt.x + 2, dbt.y});
        if (dbt.y < tiles[0].size() - 2)
            adjacent.push_back({dbt.x, dbt.y + 2});
        if (dbt.x > 1)
            adjacent.push_back({dbt.x - 2, dbt.y});
        if (dbt.y > 1)
            adjacent.push_back({dbt.x, dbt.y - 2});

        std::vector<glm::ivec2> randomisedAdjacents;
        while (!adjacent.empty()) {
            int rNum = rand() % adjacent.size();
            randomisedAdjacents.push_back(adjacent[rNum]);
            adjacent.erase(adjacent.begin() + rNum);
        }

        return randomisedAdjacents;
}

// Randomised iterative DFS implementation
void Maze::DFS(int startX, int startY) {
    std::stack<glm::ivec2> consider({{startX, startY}});
    bool exitF = false;
    while (!consider.empty()) {
        auto top = consider.top();
        consider.pop();
        auto& tile = tiles[top.x][top.y];
        if (tile.type == Type::Wall || tile.type == Type::Entrance) {
            if (tile.type != Type::Entrance) {
                blendAdjacent(top, path[top], Type::Floor);
                tile.type = Type::Floor;
            }

            auto adjacents = getAdjacents(top);
            for (auto& a : adjacents) {
                auto& aTile = tiles[a.x][a.y].type;
                if (aTile == Type::Wall || aTile == Type::Exit) {
                    path[a] = glm::ivec2(top);
                    consider.push(a);
                }
            }
        } 
    }
}

void Maze::reset() {
    build();
}

bool Maze::isEnd(glm::ivec2 point) {
    if (point == end)
        winState = true;
    return point == end;
}

glm::ivec2 Maze::getEnd() {
    return end;
}

bool Maze::won() {
    return winState;
}
