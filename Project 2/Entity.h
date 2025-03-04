#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"


class Entity {
public:
    
    glm::vec3 player_position;
    glm::vec3 player_movement;
    float speed;
    
    GLuint textureID;
    
    Entity(glm::vec3 vec);
    
    void Update(float deltaTime);
    void Render(ShaderProgram *program);
    void changeMovementY(int n);
    void changeMovementX(int n);
    float getX() const;
    float getY() const;
};
