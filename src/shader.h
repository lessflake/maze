#ifndef SHADER_H
#define SHADER_H

/*
 * Shader wrapper: Mostly taken from https://learnopengl.com/ - simple
 * wrapper that works and I do not do anything so crazy with shaders that I
 * need anything more complex. Loads shader from file, compiles and provides
 * use function, and some functions to set uniforms for convenience.
 */

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    GLuint program;

    Shader(const GLchar* vPath, const GLchar* fPath) {
        std::string vCodeStr, fCodeStr;
        std::ifstream vFile, fFile;
        vFile.exceptions(std::ifstream::badbit);
        fFile.exceptions(std::ifstream::badbit);

        try {
            vFile.open(vPath);
            fFile.open(fPath);
            std::stringstream vStream, fStream;
            vStream << vFile.rdbuf();
            fStream << fFile.rdbuf();
            vFile.close();
            fFile.close();
            vCodeStr = vStream.str();
            fCodeStr = fStream.str();
        } catch (std::ifstream::failure e) {
            std::cerr << "Failed to load shader files\n";
        }

        const GLchar* vCode = vCodeStr.c_str();
        const GLchar* fCode = fCodeStr.c_str();

        GLuint vertex, fragment;
        compileShader(vertex, vCode, GL_VERTEX_SHADER);
        compileShader(fragment, fCode, GL_FRAGMENT_SHADER);

        this->program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);

        GLint success;
        GLchar infoLog[512];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cout << "Shader linker failure: " << infoLog << '\n';
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void use() {
        glUseProgram(program);
    }

    void setUniform1f(std::string uniformName, float val) {
        glUseProgram(program);
        GLint uniLoc = glGetUniformLocation(program, uniformName.c_str());
        glUniform1f(uniLoc, val);
    }

    void updateMVP(glm::mat4 model, glm::mat4 view, glm::mat4 proj) {
        GLint modelLoc = glGetUniformLocation(program, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        GLint viewLoc = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        GLint projLoc = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    }

private:
    void compileShader(GLuint& handle, const GLchar* code, GLenum type) {
        GLint success;
        GLchar infoLog[512];

        handle = glCreateShader(type);
        glShaderSource(handle, 1, &code, NULL);
        glCompileShader(handle);
        glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
        
        if (!success) {
            glGetShaderInfoLog(handle, 512, NULL, infoLog);
            std::cerr << "Shader compilation failure: " << infoLog << '\n';
        }
    }
};

#endif
