#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glew.h>
#include <GL/freeglut.h> 
#endif

#include <glm/glm.hpp>

#include <iostream>
#include <map>
#include "input.h"

static std::map<unsigned char, bool> keys;
static std::map<unsigned char, bool> just;
static std::map<Mouse, bool> mouseButtons;
static glm::vec2 mousePos;
static glm::vec2 mouseOffset;

static void keyCallback(unsigned char key, int x, int y) {
    if (!keys[key]) {
        keys[key] = just[key] = true;
    }
}

static void keyUpCallback(unsigned char key, int x, int y) {
    if (keys[key]) {
        keys[key] = just[key] = false;
    }
}

static void mouseCallback(int button, int state, int x, int y) {
    switch (button) {
        case GLUT_LEFT_BUTTON:
            mouseButtons[Mouse::Left] = !state;
            break;
        case GLUT_RIGHT_BUTTON:
            mouseButtons[Mouse::Right] = !state;
            break;
        case GLUT_MIDDLE_BUTTON:
            mouseButtons[Mouse::Middle] = !state;
            break;
        default: break;
    }
}

static void mouseMoveCallback(int x, int y) {
    int cX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    int cY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
    if (cX != x || cY != y) {
        mouseOffset += glm::vec2(x, y) - glm::vec2(cX, cY);
        glutWarpPointer(cX, cY);
    }
}

bool Input::getKey(unsigned char key) {
    return keys[key];
}

bool Input::getJust(unsigned char key) {
    bool result = just[key];
    just[key] = false;
    return result;
}

bool Input::getBtn(Mouse btn) {
    return mouseButtons[btn];
}

bool Input::hasMoved() {
    return mouseOffset.x != 0 || mouseOffset.y != 0;
}

glm::vec2 Input::getMousePos() {
    return mousePos;
}

// Returns mouse movement since this was last called
// (0, 0) is top left corner
glm::vec2 Input::getMovement() {
    glm::vec2 offset = mouseOffset;
    mouseOffset = glm::vec2();
    return offset;
}

Input::Input() {
    int cX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    int cY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
    glutWarpPointer(cX, cY);
    mousePos = glm::vec2(cX, cY);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    glutKeyboardFunc(keyCallback);
    glutKeyboardUpFunc(keyUpCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(mouseMoveCallback);
    glutPassiveMotionFunc(mouseMoveCallback);
}
