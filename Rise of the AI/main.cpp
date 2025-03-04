#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

#define PLATFORM_COUNT 14
#define ENEMY_COUNT 3

struct GameState {
    Entity player;
    Entity platforms[PLATFORM_COUNT];
    Entity enemies[ENEMY_COUNT];
    Entity missionComplete;
    Entity missionFail;
};

GameState state;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("AI!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    state.player.entityType = PLAYER;
    state.player.isStatic = false;
    state.player.width = 1.0f;
    state.player.position = glm::vec3(-4, 3, 0);
    state.player.acceleration = glm::vec3(0, -9.81f, 0);
    
    state.missionFail.textureID = LoadTexture("mission-fail.png");
    state.missionComplete.textureID = LoadTexture("mission-success.png");
    
    state.player.textureID = LoadTexture("me.png");
    
    GLuint evilTextureID = LoadTexture("evil.png");
    GLuint spiderTextureID = LoadTexture("spider-pixilart.png");
    
    state.enemies[0].entityType = ENEMY;
    state.enemies[0].textureID = spiderTextureID;
    state.enemies[0].isStatic = false;
    state.enemies[0].position = glm::vec3(1,-2.25f,0);
    state.enemies[0].aiState = IDLE;
    state.enemies[0].aiType = Entity::WALKER;
    
    state.enemies[1].entityType = ENEMY;
    state.enemies[1].textureID = evilTextureID;
    state.enemies[1].isStatic = false;
    state.enemies[1].position = glm::vec3(2, -2.25f, 0);
    state.enemies[1].aiState = IDLE;
    state.enemies[1].aiType = Entity::WALKER;
    
    state.enemies[2].entityType = ENEMY;
    state.enemies[2].textureID = spiderTextureID;
    state.enemies[2].isStatic = false;
    state.enemies[2].position = glm::vec3(3, -2.25f, 0);
    state.enemies[2].aiState = IDLE;
    state.enemies[2].aiType = Entity::WALKER;
    
    GLuint dirtTextureID = LoadTexture("dirt.png");
    GLuint grassTextureID = LoadTexture("grass.png");
    
    for (int i = 0; i < 11; i++)
    {
        state.platforms[i].textureID = dirtTextureID;
        state.platforms[i].position = glm::vec3(i - 5.0f, -3.25f, 0);
    }
    
    state.platforms[11].textureID = grassTextureID;
    state.platforms[11].position = glm::vec3(-5.0f, -2.25f, 0);
    
    state.platforms[12].textureID = grassTextureID;
    state.platforms[12].position = glm::vec3(-4.0f, -2.25f, 0);
    
    state.platforms[13].textureID = grassTextureID;
    state.platforms[13].position = glm::vec3(5.0f, -2.25f, 0);
    
    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        state.player.Jump();
                        break;
                        
                }
                break;
        }
    }
    
    state.player.velocity.x = 0;
    
    // Check for pressed/held keys below
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if (keys[SDL_SCANCODE_LEFT])
    {
        state.player.velocity.x = -3.0f;
    }
    else if  (keys[SDL_SCANCODE_RIGHT])
    {
        state.player.velocity.x = 3.0f;
    }
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    
    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }
    
    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player.Update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT);
        
        deltaTime -= FIXED_TIMESTEP;
        for (int i = 0; i < ENEMY_COUNT; i++)
        {
            state.enemies[i].Update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT);
        }
    }
    
    accumulator = deltaTime;
}


void Render() {
    int count = 0;
    glClear(GL_COLOR_BUFFER_BIT);
    if (state.player.lost) {
        state.missionFail.Render(&program);
    } else {
        state.player.Render(&program);
        for (int i = 0; i < ENEMY_COUNT; i++)
        {
           if ((not(state.enemies[i].lost)) && (not(state.enemies[i].collidedTop))) {
                state.enemies[i].Render(&program);
            } else {
                count += 1;
            }
        }
        if (count == ENEMY_COUNT) {
            state.missionComplete.Render(&program);
        }
    }
    
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        state.platforms[i].Render(&program);
    }
    
    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
