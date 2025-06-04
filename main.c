#include <SDL2/SDL_error.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
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
    SDL_Texture* texture;
    int jump;
};

struct Platform {
    float width;
    float height;
    b2BodyId bodyid;
};

struct Camera {
    b2Vec2 position;
};

struct Game {
    int platforms_count;
    enum State state;
    struct Player player;
    struct Platform* platforms;
    b2WorldId worldid;
    struct Camera camera;
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

b2Vec2 Transform_coordinates(b2Vec2 vec) {
    vec.y = SCREEN_HEIGHT - vec.y;
    vec.x -= game.camera.position.x - SCREEN_WIDTH / 2.0;
    vec.y += game.camera.position.y - SCREEN_HEIGHT / 2.0;
    return vec;
}

void Render() {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 1);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 2, 2, 2, 1);
    b2Vec2 player_position = Transform_coordinates(b2Body_GetPosition(game.player.bodyid));
    b2Rot player_rotation = b2Body_GetRotation(game.player.bodyid);
    float player_angle = - 180 / M_PI * b2Rot_GetAngle(player_rotation);
    SDL_Rect player_rect = {
        .x = player_position.x - game.player.radius,
        .y = player_position.y - game.player.radius,
        .w = game.player.radius * 2.0,
        .h = game.player.radius * 2.0,
    };
    SDL_Point player_center = {
        .x = game.player.radius,
        .y = game.player.radius,
    };
    SDL_RenderCopyEx(renderer, game.player.texture, NULL, &player_rect, player_angle, &player_center, SDL_FLIP_NONE);
    for(int i = 0; i < game.platforms_count; i++) {
        b2Vec2 platform_position = Transform_coordinates(b2Body_GetPosition(game.platforms[i].bodyid));
        SDL_FRect rect = {
            .x = platform_position.x - game.platforms[i].width / 2,
            .y = platform_position.y - game.platforms[i].height / 2,
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
            switch(event.key.keysym.sym) {
                case SDLK_ESCAPE: game.state = EXIT; break;
                case SDLK_a: game.player.direction++; break;
                case SDLK_d: game.player.direction--; break;
                case SDLK_SPACE: game.player.jump = 1; break;
            }
        }
        if(event.type == SDL_KEYUP && !event.key.repeat) {
            switch(event.key.keysym.sym) {
                case SDLK_a: game.player.direction--; break;
                case SDLK_d: game.player.direction++; break;
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
    worlddef.gravity = (b2Vec2){0.0f, -200.0f};
    game.worldid = b2CreateWorld(&worlddef);


    game.platforms_count = 12;
    game.platforms = malloc(sizeof(struct Platform) * game.platforms_count);
    game.platforms[0] = create_platform(1500, 25, 3000, 50);
    game.platforms[1] = create_platform(0, 200, 100, 50);
    game.platforms[2] = create_platform(300, 450, 100, 50);
    game.platforms[3] = create_platform(500, 300, 100, 50);
    game.platforms[4] = create_platform(700, 150, 100, 50);
    game.platforms[5] = create_platform(900, 300, 100, 50);
    game.platforms[6] = create_platform(1550, 450, 1000, 50);
    game.platforms[7] = create_platform(1750, 775, 50, 400);
    game.platforms[8] = create_platform(2050, 625, 50, 400);
    game.platforms[9] = create_platform(2000, 600, 70, 50);
    game.platforms[10] = create_platform(1800, 700, 70, 50);
    game.platforms[11] = create_platform(2300, 700, 200, 50);

    game.player.radius = 32;
    SDL_Surface* sprite = IMG_Load("player.png");
    game.player.texture = SDL_CreateTextureFromSurface(renderer, sprite);
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){10, 300};
    game.player.bodyid = b2CreateBody(game.worldid, &bodyDef);
    b2Circle circle = {
        .center = {
            .x = 0,
            .y = 0,
        },
        .radius = game.player.radius,
    };
    b2ShapeDef shape = b2DefaultShapeDef();
    shape.density = 1.0;
    shape.material.friction = 0.5f;
    b2CreateCircleShape(game.player.bodyid, &shape, &circle);

    game.state = RUNNING;
    float time_step = TARGET_FRAME_TIME / 1000.0;
    int substep_count = 4;
    while(game.state == RUNNING) {
        Uint32 start_time = SDL_GetTicks();
        Handle_events();
        b2Vec2 player_velocity = b2Body_GetLinearVelocity(game.player.bodyid);
        if(game.player.jump) {
            b2RayResult ray = b2World_CastRayClosest(game.worldid, b2Body_GetPosition(game.player.bodyid),(b2Vec2){0, - game.player.radius - 1}, (b2QueryFilter){0x0001, 0xffff});
            if(ray.hit) {
                player_velocity.y += 250;
            }
            game.player.jump = 0;
        }
        if(b2Body_GetPosition(game.player.bodyid).y < 0) {
            break;
        }
        player_velocity.x -= 8 * game.player.direction;
        b2Body_SetLinearVelocity(game.player.bodyid, player_velocity);
        b2World_Step(game.worldid, time_step, substep_count);
        game.camera.position = b2Body_GetPosition(game.player.bodyid);
        if(game.camera.position.y <= SCREEN_HEIGHT / 2.0) {
            game.camera.position.y = SCREEN_HEIGHT / 2.0;
        }
        Render();
        Uint32 end_time = SDL_GetTicks();
        Uint32 frame_time = end_time - start_time;
        if(frame_time < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frame_time);
        }
    }
    return 0;
}
