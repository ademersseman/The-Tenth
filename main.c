#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>  

typedef struct Army {
    int ranks[4][17];
} Army;

typedef struct Player {
    int x;
    int y;
    bool blocking;
    bool mobile;
} Player;


int grid_width = 17;
int grid_height = 10;
int grid_cell_width = 40;
int grid_cell_height = 40;
int map[17][10];
Army legion;

SDL_Renderer *renderer;
void addTexture(int x, int y, const char* file) {
    SDL_Surface* surface = SDL_LoadBMP(file);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_Rect tile = {
        .x = x,
        .y = y,
        .w = grid_cell_width,
        .h = grid_cell_height,
    };
    SDL_RenderCopy(renderer, texture, NULL, &tile);
    SDL_DestroyTexture(texture);
}


void loadRanks() {
    for(int x = 0; x < 4; x++) {
        for(int y = 0; y < 7; y++) {
            legion.ranks[x][y] = 1;
            map[x][y] = 2;//code for roman soldier NPC
        }
    }
    for(int x = 0; x < 17 * grid_cell_width; x += grid_cell_width) {
        for(int y = 6 * grid_cell_height; y < 10 * grid_cell_height; y += grid_cell_height) {
            addTexture(x, y, "grass_roman.bmp");
        }
    }
}

void initBattleField(int window_width, int window_height) {
    for (int x = 0; x < window_width; x += grid_cell_width) {
        for (int y = 0; y < window_height; y += grid_cell_height) {
            addTexture(x, y, "grass.bmp");
        }
    }
    loadRanks();
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
    // + 1 so that the last grid lines fit in the screen.
    int window_width = (grid_width * grid_cell_width) + 1;//originally 1045
    int window_height = (grid_height * grid_cell_height) + 1;//originally 829
    //29x23 grid
    // Place the grid cursor in the middle of the screen.

    // Dark theme.
    SDL_Color grid_line_color = {44, 44, 44, 255}; // Dark grey

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialize SDL: %s",
                     SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window;
    if (SDL_CreateWindowAndRenderer(window_width, window_height, SDL_WINDOW_RESIZABLE, &window,
                                    &renderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Create window and renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetWindowTitle(window, "SDL Grid");
    SDL_DisplayMode current;

    SDL_bool quit = SDL_FALSE;

    Player player;
    player.blocking = false;
    player.x = (grid_width - 1) / 2 * grid_cell_width;
    player.y = (grid_height - 1) / 2 * grid_cell_height;

    //testing placing textures on grid
    initBattleField(window_width, window_height);

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                    case SDLK_UP:
                        if (player.y - grid_cell_height >= 0) {
                            addTexture(player.x, player.y, "grass.bmp");
                            player.y -= grid_cell_height;
                        }
                        break;
                    case SDLK_s:
                    case SDLK_DOWN:
                        if (player.y + 1.5 * grid_cell_height  < window_height) {
                            addTexture(player.x, player.y, "grass.bmp");
                            player.y += grid_cell_height;
                        }
                        break;
                    case SDLK_a:
                    case SDLK_LEFT:
                        if (player.x - grid_cell_width >= 0) {
                            addTexture(player.x, player.y, "grass.bmp");
                            player.x -= grid_cell_width;
                        }
                        break;
                    case SDLK_d:
                    case SDLK_RIGHT:
                        if (player.x + 1.5 * grid_cell_width < window_width) {
                            addTexture(player.x, player.y, "grass.bmp");
                            player.x += grid_cell_width;
                        }
                        break;
                    case SDLK_e:
                        SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g, grid_line_color.b, grid_line_color.a);
                        SDL_Rect range = {
                            .x = player.x - grid_cell_width,
                            .y = player.y - grid_cell_height,
                            .w = grid_cell_width,
                            .h = grid_cell_height,
                        };
                        for (int y = 0; y < 3; y++) {
                            for (int x = 0; x < 3; x++) {
                                SDL_RenderFillRect(renderer, &range);
                                range.x += grid_cell_width;
                            }
                            range.x -= 3 * grid_cell_width;
                            range.y += grid_cell_height;
                        }
                        SDL_RenderPresent(renderer);
                        break;
                    case SDLK_q:
                    player.blocking = !player.blocking;
                    break;
                }
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    grid_cell_height = (int)((event.window.data2 - 1)/(double)grid_height);
                    grid_cell_width = (int)((event.window.data1 - 1)/(double)grid_width);
                    window_height = event.window.data2;
                    window_width = event.window.data1;

                    player.x = (grid_width - 1) / 2 * grid_cell_width;
                    player.y = (grid_height - 1) / 2 * grid_cell_height;

                    initBattleField(window_width, window_height);
                }
                break;
            case SDL_QUIT:
                quit = SDL_TRUE;
                break;
            }
        }

        // Draw player.
        addTexture(player.x + 1, player.y + 1, "grass_player.bmp");

        // Draw grid lines.
        SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g, grid_line_color.b, grid_line_color.a);

        //draw vertical grid lines
        for (int x = 0; x < 1 + grid_width * grid_cell_width; x += grid_cell_width) {
            SDL_RenderDrawLine(renderer, x, 0, x, window_height);
        }
        //draw horizontal grid lines
        for (int y = 0; y < 1 + grid_height * grid_cell_height; y += grid_cell_height) {
            SDL_RenderDrawLine(renderer, 0, y, window_width, y);
        }

        SDL_RenderPresent(renderer);
    }

    //destroy resources before quitting
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}