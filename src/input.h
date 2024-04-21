#ifndef INPUT_H
#define INPUT_H

/*
 * Input - small class to wrap GLUT's input
 */

enum class Mouse : int {
    Left,
    Right,
    Middle
};

class Input {
public:
    Input();
    ~Input() {}

    bool getKey(unsigned char key);  // If a key is pressed
    bool getJust(unsigned char key); // If a key has just been pressed
    bool getBtn(Mouse btn);          // Mouse button state
    bool hasMoved();                 // Mouse movement state
    glm::vec2 getMousePos();
    glm::vec2 getMovement();

private:
    glm::vec2 lastMousePos;
};

#endif
