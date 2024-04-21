#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <vector>
#include <cstdlib>
#include <map>

#include "cube_vertices.h"
#include "minimap.h"

static void display();
static void reshape(int w, int h);
static void idle();

Renderer& Renderer::getInstance() {
    static Renderer instance;
    return instance;
}

void Renderer::start(World* w, int sW, int sH) {
    projection = glm::perspective(
        glm::radians(60.0f), 
        (float) sW / 
        (float) sH,
        0.01f,
        10.0f);
    world = w;
    genMinimap();
    glutMainLoop();
}

void Renderer::displayCall() {
    drawToFramebuffer();
    drawScene();
    glutSwapBuffers();
}

void Renderer::idleCall() {
    world->tick();
    if (world->getMinimap().needsUpdate())
        updateMinimap();
    glutPostRedisplay();
}

void Renderer::drawMaze() {
    auto& m = world->getMaze();
    auto& grid = m.getGrid();
    auto view = world->getView();
    auto pos = world->getPos();

    const int gridSizeX = grid.size();
    const int gridSizeY = grid[0].size();

    const int rSize = 10;
    // Representing the bound of tiles to render - rSize square around
    // camera
    const int lowerX = (int) pos.x >= rSize ? (int) pos.x - rSize : 0;
    const int upperX = (int) pos.x < gridSizeX - rSize ? 
                       (int) pos.x + rSize : gridSizeX - 1;
    const int lowerY = (int) pos.y >= rSize ? (int) pos.y - rSize : 0;
    const int upperY = (int) pos.y < gridSizeY - rSize ?
                       (int) pos.y + rSize : gridSizeY - 1;

    mazeShader.use();
    glm::vec3 normal;
    /* Supply uniforms to shader for lighting */
    GLuint nLoc = glGetUniformLocation(mazeShader.program, "normal");
    GLuint lLoc = glGetUniformLocation(mazeShader.program, "lightPos");
    glUniform3fv(lLoc, 1, glm::value_ptr(glm::vec3(pos, 1.7f)));
    GLuint eLoc = glGetUniformLocation(mazeShader.program, "endPos");
    glUniform3fv(eLoc, 1, glm::value_ptr(glm::vec3(m.getEnd(), 1.7f)));
    GLuint tLoc = glGetUniformLocation(mazeShader.program, "time");
    glUniform1i(tLoc, glutGet(GLUT_ELAPSED_TIME));

    for (int i = lowerX; i <= upperX; ++i) {
        for (int j = lowerY; j <= upperY; ++j) {
            /* Rendering walls */
            if (grid[i][j].type == Type::Wall)
                continue;
            // Every wall face at current tile
            std::vector<int> faces = m.facesAt(i, j);

            // Set face's position according to tile positions
            glm::mat4 model;
            model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.0f));
            model = glm::translate(model, 
                ((float) 1.0f) * glm::vec3((float) i, (float) j, 1.0f));

            // Bind appropriate texture, VAO and set normal for face
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            for (auto& f : faces) {
                switch (m.mesh[f].dir) {
                    case Dir::North:
                        normal = glm::vec3(0.0f, 1.0f, 0.0f);
                        setModel(Model::South);
                        break;
                    case Dir::East:
                        normal = glm::vec3(1.0f, 0.0f, 0.0f);
                        setModel(Model::West);
                        break;
                    case Dir::South:
                        normal = glm::vec3(0.0f, -1.0f, 0.0f);
                        setModel(Model::North);
                        break;
                    case Dir::West:
                        normal = glm::vec3(-1.0f, 0.0f, 0.0f);
                        setModel(Model::East);
                        break;
                }
                glUniform3fv(nLoc, 1, glm::value_ptr(normal));

                glm::mat4 upperWalls = model;
                glm::vec3 offset = {0.0f, 0.0f, 1.0f};
                for (int i = 0; i < 5; ++i) {
                    mazeShader.updateMVP(upperWalls, view, projection);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    upperWalls = glm::translate(upperWalls, offset);
                }
            }

            /* Rendering floors */
            setModel(Model::Floor);
            glBindTexture(GL_TEXTURE_2D, textures[1]);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
            mazeShader.updateMVP(model, view, projection);
            normal = glm::vec3(0.0f, 0.0f, 1.0f);
            glUniform3fv(nLoc, 1, glm::value_ptr(normal));

            glDrawArrays(GL_TRIANGLES, 0, 6);

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

/* Drawing portal at end of maze */
void Renderer::drawExit() {
    auto& grid = world->getMaze().getGrid();
    auto view = world->getView();
    auto pos = world->getPos();
    const int gridSizeX = grid.size();
    const int gridSizeY = grid[0].size();

    /* Have to sort each face by distance from player so that *
     * transparency is rendered correctly                     */

    std::map<float, Model> sorted;
    glm::vec2 sidePos = {-1.5f + gridSizeX, -1.5f + gridSizeY};
    std::vector<glm::vec2> sideOffsets = {
        sidePos + glm::vec2{0.0f, 0.5f}, // north
        sidePos + glm::vec2{0.5f, 0.0f}, // east
        sidePos + glm::vec2{0.0f, -0.5f}, // south
        sidePos + glm::vec2{-0.5f, 0.0f} // west
    };
    glm::mat4 model = glm::translate(glm::mat4(), 
            glm::vec3(-1.5f + gridSizeX , -1.5f + gridSizeY, 1.0f));
    glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(0.875f, 0.875f, 1.0f));
    model = model * scale;

    for (GLuint i = 0; i < sideOffsets.size(); ++i) {
        GLfloat distance = glm::length(pos - sideOffsets[i]);
        sorted[distance] = (Model) i;
    }

    // Want faces of portal to be visible from both sides
    glDisable(GL_CULL_FACE);
    portalShader.use();
    portalShader.updateMVP(model, view, projection);
    portalShader.setUniform1f("time", glutGet(GLUT_ELAPSED_TIME));
    // Loop through in reverse order of distance to render transparency
    // correctly
    for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
        setModel(it->second);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glEnable(GL_CULL_FACE);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::drawMinimap() {
    setModel(Model::Minimap);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    mapShader.use();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

/* Draws scene off-screen to a framebuffer */
void Renderer::drawToFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    drawMaze();
    drawExit();
    if (world->getMinimap().enabled())
        drawMinimap();

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* Draws framebuffer to screen */
void Renderer::drawScene() {
    auto& m = world->getMaze();
    glClear(GL_COLOR_BUFFER_BIT);
    screenShader.use();

    // Postprocessing effect - fade out when player has reached
    // end of maze, reset maze and fade back in after a set time
    // 60 fps, so 5 second fade out and 2.5 second fade in
    static int endState = 0;
    if (m.won() || endState != 0) {
        static float framesSinceWon = 0.0f;
        if (endState == 0) {
            screenShader.setUniform1f("time", framesSinceWon);
            screenShader.setUniform1f("fade", 2.0);
            ++framesSinceWon;
            if (framesSinceWon >= 300.0f) {
                world->reset();
                framesSinceWon = 0.0f;
                endState++;
            }
        } else if (endState == 1) {
            screenShader.setUniform1f("time", framesSinceWon);
            screenShader.setUniform1f("fade", 1.0);
            framesSinceWon++;
            if (framesSinceWon >= 150.0f)
                endState++;
        } else {
            screenShader.setUniform1f("fade", 0.0);
            framesSinceWon = 0;
            endState = 0;
        }
    }

    setModel(Model::Screen);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void Renderer::registerModel(Model m, std::vector<GLfloat> data) {
    static int registeredModels = 0;
    int next = registeredModels;
    glBindBuffer(GL_ARRAY_BUFFER, vbos[next]);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), 
        data.data(), GL_STATIC_DRAW);

    glBindVertexArray(vaos[next]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
        5 * sizeof(GLfloat), 
        (GLvoid*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
        5 * sizeof(GLfloat), 
        (GLvoid*) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    modelMap[m] = vaos[next];
    registeredModels++;
}

void Renderer::reloadModel(Model m, std::vector<GLfloat> data) {
    glBindBuffer(GL_ARRAY_BUFFER, modelMap[m]);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat),
        data.data(), GL_STATIC_DRAW);
}

inline void Renderer::setModel(Model m) {
    glBindVertexArray(modelMap[m]);
}

// Framebuffer needs texture & renderbuffer object to be made
void Renderer::genFramebuffer(int screenW, int screenH) {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenW, screenH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenW, screenH);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Renderer::Renderer() :
        mazeShader("src/shaders/maze.vert", "src/shaders/maze.frag"),
        mapShader("src/shaders/minimap.vert", "src/shaders/minimap.frag"),
        portalShader("src/shaders/end.vert", "src/shaders/end.frag"),
        screenShader("src/shaders/post.vert", "src/shaders/post.frag")
{
    std::vector<std::vector<GLfloat>> models = {north, east, south, west, top};
    vbos.resize(8);
    vaos.resize(8);
    glGenBuffers(vbos.size(), &vbos[0]);
    glGenVertexArrays(vaos.size(), &vaos[0]);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);

    genFramebuffer(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

    // Model vertices defined in cube_vertices.h header
    std::unordered_map<Model, int> modelMap;
    registerModel(Model::North, north);
    registerModel(Model::East, east);
    registerModel(Model::Floor, top);
    registerModel(Model::South, south);
    registerModel(Model::West, west);
    registerModel(Model::Screen, screenQuad);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(3, textures);

    /* Generating 2 16x16 RGB textures to use for wall and floor */
    int w, h, n;
    w = h = 16;
    n = 3; // Color channels

    unsigned char* wall = new unsigned char[w*h*n];
    unsigned char* floor = new unsigned char[w*h*n];

    /* Setting pixels with random offsets on both textures */
    for (int i = 0; i < h*w*3; i += 3) {
        wall[i]   = rand() % 30;
        wall[i+1] = 140 - (rand() % 60);
        wall[i+2] = rand() % 30;

        floor[i]   = 130 + ((rand() % 30));
        floor[i+1] = 102 + ((rand() % 30));
        floor[i+2] = 68 + ((rand() % 30));
    }

    Renderer::loadTexture(0, wall, w, h);
    Renderer::loadTexture(1, floor, w, h);

    delete[] wall;
    delete[] floor;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
}

void Renderer::loadTexture(int index, unsigned char* data, int w, int h, bool alpha) {
    auto texComps = alpha ? GL_RGBA8 : GL_RGB8;
    auto comps = alpha ? GL_RGBA : GL_RGB;

    glBindTexture(GL_TEXTURE_2D, textures[index]);
    glTexStorage2D(GL_TEXTURE_2D, 4, texComps, w, h);
    glTexSubImage2D(GL_TEXTURE_2D, 
                    0, 0, 0, w, h, 
                    comps, GL_UNSIGNED_BYTE, 
                    data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::genMinimap() {
    Minimap& m = world->getMinimap();
    std::vector<GLfloat> verts = m.getVertices();
    registerModel(Model::Minimap, verts);
    Renderer::loadTexture(2, m.getTexture(), m.getWidth(), m.getHeight(), true);
    mapShader.setUniform1f("shadowSize", m.getShadowSize());
}

void Renderer::updateMinimap() {
    Minimap& m = world->getMinimap();
    Renderer::loadTexture(2, m.getTexture(), m.getWidth(), m.getHeight(), true);
}

void Renderer::reshapeCall(int w, int h) {
    glViewport(0, 0, w, h);
    projection = glm::perspective(
        glm::radians(60.0f), 
        (float) w / 
        (float) h,
        0.01f,
        10.0f);
    genFramebuffer(w, h);
    auto& m = world->getMinimap();
    m.reshape(w, h);
    reloadModel(Model::Minimap, m.getVertices());
    // Need to reload shadow size since it changes with screen size
    mapShader.setUniform1f("shadowSize", m.getShadowSize());
}

// GLUT's required static functions

static void display() {
    Renderer::getInstance().displayCall();
}

static void reshape(int w, int h) {
    Renderer::getInstance().reshapeCall(w, h);
}

static void idle() {
    Renderer::getInstance().idleCall();
}
