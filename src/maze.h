#ifndef MAZE_H
#define MAZE_H

/*
 * Maze - generates maze using a DFS, constructs collision mesh.
 * Keeps track of player win state.
 */

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <cstdint>

// Direction a wall in maze is facing
enum class Dir {
    North,
    East,
    South,
    West
};

struct Face {
    glm::vec2 tilePos;
    glm::vec3 points[2];
    glm::vec3 normal;
    Dir dir;
};

enum class Type : uint8_t {
    Wall = 0,
    Floor,
    Entrance,
    Exit,
    Path
};

struct Tile {
    Type type;
};

// Need to hash glm::ivec2 for std::unordered_map
struct hashivec2 {
    std::size_t operator() (const glm::ivec2& a) const {
        return std::hash<float>()(a.x) ^ std::hash<float>()(a.y);
    }
};

typedef std::vector<Tile> TileRow;
typedef std::vector<TileRow> TileGrid;
typedef std::vector<Face> CollisionMesh;

class Maze {
    public:
        Maze(int width, int height);
        ~Maze() {}

        TileGrid& getGrid();
        Tile& getTile(int x, int y);
        CollisionMesh mesh;
        std::vector<int> facesAt(int x, int y);
        bool isEnd(glm::ivec2 point);
        glm::ivec2 getEnd();
        bool won();
        void reset();

    private:
        void blendAdjacent(glm::ivec2 a, glm::ivec2 b, Type t);
        void DFS(int startX, int startY);
        void addFace(int tX, int tY, int offX, int offY);
        void lookupInsert(int x, int y, int i);
        std::vector<glm::ivec2> getAdjacents(glm::ivec2 dbt);
        glm::ivec2 end;
        void build();

        TileGrid tiles;
        bool exitFound;
        bool winState;
        std::unordered_map<glm::ivec2, glm::ivec2, hashivec2> path;
        std::unordered_map<int, std::vector<int>> faceLookup;
};

#endif
