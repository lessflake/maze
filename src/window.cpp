#include "window.h"

#include <iostream>

/* Just a small wrapper around GLUT windows */

bool Window::init(int *argc, char *argv[]) {
    makeWindow(argc, argv);
    return initGL();
}

void Window::makeWindow(int *argc, char *argv[]) {
    glutInitContextVersion(4, 5);
    glutInitWindowSize(width, height);
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Maze");
}

bool Window::initGL() {
#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
        return false;
#endif

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return true;
}
