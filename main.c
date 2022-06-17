#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>

void coverArea(int grid_width, int grid_height, int grid_cell_width, int grid_cell_height, int window_width, int window_height, SDL_Renderer *renderer) {
    for (int x = 0; x < window_width; x += grid_cell_width) {
        for (int y = 0; y < window_height; y += grid_cell_height) {
            SDL_Surface* surface = SDL_LoadBMP("grass.bmp");
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
    }
    SDL_RenderPresent(renderer);
}

void addTexture(int x, int y, int grid_cell_width, int grid_cell_height, SDL_Renderer *renderer) {
    SDL_Surface* surface = SDL_LoadBMP("grass.bmp");
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

int main(int argc, char *argv[]) {
    int grid_width = 29;
    int grid_height = 23;
    int grid_cell_width = 36;
    int grid_cell_height = 36;

    // + 1 so that the last grid lines fit in the screen.
    int window_width = (grid_width * grid_cell_width) + 1;//originally 1045
    int window_height = (grid_height * grid_cell_height) + 1;//originally 829
    //29x23 grid
    // Place the grid cursor in the middle of the screen.
    SDL_Rect grid_cursor = {
        .x = (grid_width - 1) / 2 * grid_cell_width,
        .y = (grid_height - 1) / 2 * grid_cell_height,
        .w = grid_cell_width,
        .h = grid_cell_height,
    };

    // Dark theme.
    SDL_Color grid_line_color = {44, 44, 44, 255}; // Dark grey
    SDL_Color grid_cursor_color = {255, 255, 255, 255}; // White

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialize SDL: %s",
                     SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;
    if (SDL_CreateWindowAndRenderer(window_width, window_height, SDL_WINDOW_RESIZABLE, &window,
                                    &renderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Create window and renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetWindowTitle(window, "SDL Grid");
    SDL_DisplayMode current;

    SDL_bool quit = SDL_FALSE;

    //testing placing textures on grid
    coverArea(grid_width, grid_height, grid_cell_width, grid_cell_height, window_width, window_height, renderer);

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                    case SDLK_UP:
                        if (grid_cursor.y - grid_cell_height >= 0) {
                            addTexture(grid_cursor.x, grid_cursor.y, grid_cell_width, grid_cell_height, renderer);
                            grid_cursor.y -= grid_cell_height;
                        }
                        break;
                    case SDLK_s:
                    case SDLK_DOWN:
                        if (grid_cursor.y + grid_cell_height + 5 < window_height) {
                            addTexture(grid_cursor.x, grid_cursor.y, grid_cell_width, grid_cell_height, renderer);
                            grid_cursor.y += grid_cell_height;
                        }
                        break;
                    case SDLK_a:
                    case SDLK_LEFT:
                        if (grid_cursor.x - grid_cell_width >= 0) {
                            addTexture(grid_cursor.x, grid_cursor.y, grid_cell_width, grid_cell_height, renderer);
                            grid_cursor.x -= grid_cell_width;
                        }
                        break;
                    case SDLK_d:
                    case SDLK_RIGHT:
                        if (grid_cursor.x + grid_cell_width + 10 < window_width) {
                            addTexture(grid_cursor.x, grid_cursor.y, grid_cell_width, grid_cell_height, renderer);
                            grid_cursor.x += grid_cell_width;
                        }
                        break;
                    case SDLK_e:
                        SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g, grid_line_color.b, grid_line_color.a);
                        SDL_Rect range = {
                            .x = grid_cursor.x - grid_cell_width,
                            .y = grid_cursor.y - grid_cell_height,
                            .w = grid_cell_width,
                            .h = grid_cell_height,
                        };
                        for (int y = 0; y < 3; y++) {
                            for (int x = 0; x < 3; x++) {
                                SDL_RenderFillRect(renderer, &range);
                                range.x += grid_cell_width;
                            }
                            range.x -= 3 * grid_cell_width;
                            range.y += grid_cell_width;
                        }
                        SDL_RenderPresent(renderer);
                        break;
                }
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    grid_cell_height = (int)((double)(event.window.data2 - 1)/grid_height);
                    grid_cell_width = (int)((double)(event.window.data1 - 1)/grid_width);
                    window_height = event.window.data2;
                    window_width = event.window.data1;

                    grid_cursor.x = (grid_width - 1) / 2 * grid_cell_width;
                    grid_cursor.y = (grid_height - 1) / 2 * grid_cell_height;
                    grid_cursor.w = grid_cell_width;
                    grid_cursor.h = grid_cell_height;

                    coverArea(grid_width, grid_height, grid_cell_width, grid_cell_height, window_width, window_height, renderer);
                }
                break;
            case SDL_QUIT:
                quit = SDL_TRUE;
                break;
            }
        }

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

        // Draw player.
        SDL_SetRenderDrawColor(renderer, grid_cursor_color.r, grid_cursor_color.g, grid_cursor_color.b, grid_cursor_color.a);
        SDL_RenderFillRect(renderer, &grid_cursor);

        SDL_RenderPresent(renderer);
    }
    //destroy resources before quitting
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}