#ifndef WINDOW_H
#define WINDOW_H

/*
 * Window - initialise GLUT window and an OpenGL context
 */

#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glew.h>
#include <GL/freeglut.h> 
#endif

class Window {
public:
    int width;
    int height;

    Window(int width, int height) : width(width), height(height) {}
    ~Window() {}

    bool init(int *argc, char *argv[]);

private:
    void makeWindow(int *argc, char *argv[]);
    bool initGL();
};

#endif
