#include <SDL2/SDL_error.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <box2d/collision.h>
#include <box2d/id.h>
#include <box2d/math_functions.h>
#include <box2d/types.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <box2d/box2d.h>

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;
const Uint32 TARGET_FRAME_TIME = 1000 / 24;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

enum State {
    RUNNING,
    EXIT,
};

struct Player {
    float radius;
    int direction;
    b2BodyId bodyid;
};

struct Platform {
    float width;
    float height;
    b2BodyId bodyid;
};

struct Game {
    enum State state;
    struct Player player;
    struct Platform* platforms;
    int platforms_count;
    b2WorldId worldid;
} game;

void Draw_circle(float x_center, float y_center, float radius) {
    for(float y = y_center - radius; y <= y_center + radius; y++) {
        for(float x = x_center - radius; x <= x_center + radius; x++) {
            float distance = hypotf(x - x_center, y - y_center);
            if(distance <= radius) {
                SDL_RenderDrawPointF(renderer, x, y);
            }
        }
    }
}

void Render() {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 1);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 2, 2, 2, 1);
    b2Vec2 player_position = b2Body_GetPosition(game.player.bodyid);
    Draw_circle(player_position.x, SCREEN_HEIGHT - player_position.y, game.player.radius);
    for(int i = 0; i < game.platforms_count; i++) {
        b2Vec2 platform_position = b2Body_GetPosition(game.platforms[i].bodyid);
        SDL_FRect rect = {
            .x = platform_position.x - game.platforms[i].width / 2,
            .y = SCREEN_HEIGHT - (platform_position.y + game.platforms[i].height / 2),
            .w = game.platforms[i].width,
            .h = game.platforms[i].height,
        };
        SDL_RenderFillRectF(renderer, &rect);
    }
    SDL_RenderPresent(renderer);
}

struct Platform create_platform(float x, float y, float width, float height) {
    struct Platform platform;
    b2BodyDef bodydef = b2DefaultBodyDef();
    bodydef.position = (b2Vec2){x, y};
    platform.bodyid = b2CreateBody(game.worldid, &bodydef);
    platform.width = width;
    platform.height = height;
    b2Polygon bodybox = b2MakeBox(width / 2.0, height / 2.0);
    b2ShapeDef shapedef = b2DefaultShapeDef();
    b2CreatePolygonShape(platform.bodyid, &shapedef, &bodybox);
    return platform;
}

void Handle_events() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
            game.state = EXIT;
        }
        if(event.type == SDL_KEYDOWN && !event.key.repeat) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: game.state = EXIT; break;
            }
        }
    }
}

void Init() {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("не удалось инициализировать SDL: %s\n", SDL_GetError());
    }
    window = SDL_CreateWindow("Platformer", 500, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(window == NULL) {
        printf("Ошибка, не удалось создать окно, %s\n", SDL_GetError());
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(renderer == NULL) {
        printf("Ошибка, не удалось создать рендерер, %s\n", SDL_GetError());
    }
    if(TTF_Init() != 0) {
        printf("Ошибка, не удалось инициализировать шрифт %s\n", TTF_GetError());
    }
}

int main(int argc, char* argv[]) {
    Init();

    b2WorldDef worlddef = b2DefaultWorldDef();
    worlddef.gravity = (b2Vec2){0.0f, -25.0f};
    game.worldid = b2CreateWorld(&worlddef);


    game.platforms_count = 2;
    game.platforms = malloc(sizeof(struct Platform) * game.platforms_count);
    game.platforms[0] = create_platform(500, 25, 1000, 50);
    game.platforms[1] = create_platform(200, 300, 200, 50);

    game.player.radius = 20;
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0};
    game.player.bodyid = b2CreateBody(game.worldid, &bodyDef);
    b2Polygon dynamic_box = b2MakeBox(game.player.radius, game.player.radius);
    b2ShapeDef shape = b2DefaultShapeDef();
    shape.density = 1.0;
    shape.material.friction = 0.3f;
    b2CreatePolygonShape(game.player.bodyid, &shape, &dynamic_box);

    game.state = RUNNING;
    float time_step = TARGET_FRAME_TIME / 1000.0;
    int substep_count = 4;
    while(game.state == RUNNING) {
        Uint32 start_time = SDL_GetTicks();
        Handle_events();
        b2World_Step(game.worldid, time_step, substep_count);
        Render();
        Uint32 end_time = SDL_GetTicks();
        Uint32 frame_time = end_time - start_time;
        if(frame_time < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frame_time);
        }
    }
    return 0;
}
