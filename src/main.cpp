#include <iostream>
#include <string>
#include <cctype>

#include "window.h"
#include "world.h"
#include "renderer.h"

/* Doesn't do much, everything is handled in the other files */

static const int WIDTH = 1920;
static const int HEIGHT = 1080;

void print_usage() {
    std::cout << "Please give 0 or 2 arguments: 0 for default "
        << "size maze, 2 to specify width & height of maze. " 
        << "Non-square mazes work fine. 20x20 maze by default."
        << "\n\n./maze width height\n"
        << "\twidth: integer - width of maze (>= 1)\n"
        << "\theight: integer - height of maze (>= 1)\n\n";
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    int mazeW, mazeH;
    {   // Scope so s1, s2 don't pollute memory entire time
        std::string::size_type s1, s2;
        switch (argc) {
            case 1:
                mazeW = mazeH = 10;
                break;
            case 3:
                if (!isdigit(argv[1][0]) || !isdigit(argv[2][0]))
                    print_usage();
                mazeW = std::stoi(std::string(argv[1]), &s1);
                mazeH = std::stoi(std::string(argv[2]), &s2);
                if     (s1 != std::string(argv[1]).length() ||
                        s2 != std::string(argv[2]).length() ||
                        mazeW <= 1 ||
                        mazeH <= 1)
                    print_usage();
                break;
            default: print_usage();
        }
    }

    Window window(WIDTH, HEIGHT);
    window.init(&argc, argv);
    World world(WIDTH, HEIGHT, mazeW, mazeH);
    // Renderer is singleton because GLUT, initialised/started here
    Renderer::getInstance().start(&world, WIDTH, HEIGHT);
    return 0;
}
