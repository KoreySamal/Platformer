#include <SDL2/SDL_error.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;
const Uint32 TARGET_FRAME_TIME = 1000 / 15;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

enum State {
    RUNNING,
    EXIT,
};

struct Player {
    float x_center;
    float y_center;
    float radius;
    int direction;
};

struct Platform {
    SDL_FRect rect;
};

struct Game {
    enum State state;
    struct Player player;
    struct Platform* platforms;
    int platforms_count;
};

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

void Render(struct Game* game) {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 1);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 2, 2, 2, 1);
    Draw_circle(game->player.x_center, game->player.y_center, game->player.radius);
    for(int i = 0; i < game->platforms_count; i++) {
        SDL_RenderFillRectF(renderer, &game->platforms[i].rect);
    }
    SDL_RenderPresent(renderer);
}

void Physics(struct Game* game) {
    game->player.y_center += 5;
    for(int i = 0; i < game->platforms_count; i++) {
        if(
            game->player.x_center + game->player.radius >= game->platforms[i].rect.x &&
            game->player.x_center - game->player.radius <= game->platforms[i].rect.x + game->platforms[i].rect.w
        ) {
            if(
                game->player.y_center + game->player.radius >= game->platforms[i].rect.y &&
                game->player.y_center - game->player.radius <= game->platforms[i].rect.y + game->platforms[i].rect.h
            ) {
                game->player.y_center = game->platforms[i].rect.y - game->player.radius;
            }
        }
    }
}

void Handle_events(struct Game* game) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
            game->state = EXIT;
        }
        if(event.type == SDL_KEYDOWN && !event.key.repeat) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: game->state = EXIT; break;
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
    struct Game game;
    game.platforms_count = 2;
    game.platforms = malloc(sizeof(struct Platform) * game.platforms_count);
    game.platforms[0].rect.x = 0;
    game.platforms[0].rect.y = 750;
    game.platforms[0].rect.w = 1000;
    game.platforms[0].rect.h = 50;

    game.platforms[1].rect.x = 400;
    game.platforms[1].rect.y = 600;
    game.platforms[1].rect.w = 200;
    game.platforms[1].rect.h = 50;
    game.player = (struct Player) {
        .x_center = SCREEN_WIDTH / 2.0,
        .y_center = SCREEN_HEIGHT / 2.0,
        .radius = 20,
    };
    game.state = RUNNING;
    while(game.state == RUNNING) {
        Uint32 start_time = SDL_GetTicks();
        Handle_events(&game);
        Physics(&game);
        Render(&game);
        Uint32 end_time = SDL_GetTicks();
        Uint32 frame_time = end_time - start_time;
        if(frame_time < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frame_time);
        }
    }
    return 0;
}
