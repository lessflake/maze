#ifndef RENDERER_H
#define RENDERER_H

/* 
 * Renderer - handles all OpenGL rendering.
 */

#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glew.h>
#include <GL/freeglut.h> 
#endif

#include <glm/glm.hpp>
#include <unordered_map>

#include "shader.h"
#include "world.h"

/* Simple enum since there aren't many models */
enum class Model {
    North,
    East,
    South,
    West,
    Floor,
    Minimap,
    Screen
};

// Specify hashing a model enum (for std::unordered_map<Model, GLint>)
namespace std {
    template <>
    struct hash<Model> {
        size_t operator() (const Model &t) const {
            return size_t(t);
        }
    };
}

class Renderer {
public:
    // Renderer has to be singleton to interface with GLUT properly
    static Renderer& getInstance();
    Renderer(Renderer const&) = delete;
    void operator=(Renderer const&) = delete;

    /* Start renderer given screen width and screen height */
    void start(World* w, int sW, int sH);

    /* GLUT callbacks call these */
    void displayCall();
    void reshapeCall(int w, int h);
    void idleCall();

private:
    // Since GLUT handles display/idle as a callback, must call back to
    // the world instance to update everything
    World* world;

    /* Containers of VBOs, VAOs, models */
    std::vector<GLuint> vbos;
    std::vector<GLuint> vaos;
    std::unordered_map<Model, GLint> modelMap;
    /* Framebuffer to draw to, to do postprocessing on */
    GLuint fbo;
    /* Maze textures, and one for whole screen framebuffer */
    GLuint textures[3];
    GLuint screenTexture;

    Shader mazeShader;   // Shader for walls, floors of maze
    Shader mapShader;    // for minimap
    Shader portalShader; // for end blue portal
    Shader screenShader; // for screen framebuffer (for fade effect)

    glm::mat4 projection;

    Renderer(); // Renderer is singleton, so private constructor

    void loadTexture(int index, unsigned char* data, int w, int h, 
                     bool alpha = false);
    // Add model to map, so VAO can be looked up by enum
    void registerModel(Model m, std::vector<GLfloat> data);
    // Reload model's vertices
    void reloadModel(Model m, std::vector<GLfloat> data);
    // Generate framebuffer to off-screen render to
    void genFramebuffer(int screenW, int screenH);
    // Set up VAO/VBO/texture for minimap
    void genMinimap();
    // Reload minimap's texture if it needs updating
    void updateMinimap();
    // Set current VAO to one mapped to by modelMap
    void setModel(Model m);

    // For actually rendering the scene
    void drawToFramebuffer();
    void drawMaze();
    void drawExit();
    void drawMinimap();
    void drawScene();
};

#endif
