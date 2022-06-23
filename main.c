#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct Army {
    int ranks[17];
    int soldiers;
    int frontLine;
} Army;

typedef struct Player {
    int x;
    int y;
    bool blocking;
    bool mobile;
    int facing;
} Player;

typedef struct Point {
    int x;
    int y;
    bool dir;//true when moving left or up
    struct Point *next;
} Point;

int grid_width = 17;
int grid_height = 10;
int grid_cell_width = 40;
int grid_cell_height = 40;
//Player:0 | Friendly NPC:1 | Hostile NPC: 2 | grass: 3
int map[17][10];
Player player;
Army legion;
Army gauls;

SDL_Renderer *renderer;
void addTexture(int x, int y, const char* file) {
    SDL_Surface* surface = SDL_LoadBMP(file);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_Rect tile = {
        .x = x * grid_cell_width + 1,
        .y = y * grid_cell_height + 1,
        .w = grid_cell_width,
        .h = grid_cell_height,
    };
    SDL_RenderCopy(renderer, texture, NULL, &tile);
    SDL_DestroyTexture(texture);
}

//for entering raw points
void addTextureNoScale(int x, int y, const char* file) {
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

void reverse(Point *ptr) {
    Point *prev = NULL;
    Point *curr;
    Point *next = NULL;

    curr = ptr;
    while(curr != NULL) {
        next = curr->next;

        curr->next = prev;

        prev = curr;
        curr = next;
    }
    ptr = prev;
}

void block() {
    player.blocking = !player.blocking;
    if (player.blocking) {
        addTexture(player.x, player.y, "grass_player_blocking.bmp");
        player.mobile = false;
    } else {
        addTexture(player.x, player.y, "grass_player.bmp");
        player.mobile = true;
    }
    SDL_RenderPresent(renderer);
}

bool attack() {
    switch(player.facing) {
        case 0:
            addTexture(player.x, player.y, "grass_attack.bmp");
            if (map[player.x][player.y - 1] == 2) {
                gauls.ranks[player.x]--;
                gauls.soldiers--;
                map[player.x][gauls.frontLine] = 3;
                addTexture(player.x, gauls.frontLine, "grass.bmp");
            }
            SDL_RenderPresent(renderer);
            break;
    }
}

//interpret player controls during delay
void battleDelay(Uint32 delay) {
    Uint32 time = SDL_GetTicks() + delay;
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), time)) {
        SDL_Event event;
        while (SDL_PollEvent(&event) && event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_q:
                    block();
                    break;
                case SDLK_e:
                    attack();
                    break;
            }
            break;
        }
    }
}
void loadRomanRanks() {
    legion.soldiers = 51;
    legion.frontLine = 7;
    for (int x = 0; x < grid_width; x++) {
        legion.ranks[x] = 3;
        for(int y = legion.frontLine; y < grid_height; y++) {
            map[x][y] = 1;//code for roman soldier NPC
            addTexture(x, y, "grass_roman.bmp");
        }
    }
    SDL_RenderPresent(renderer);
}

//loads gaul horde
void loadGaulHorde() {
    gauls.frontLine = 3;
    gauls.soldiers = 0;
    for (int x = 0; x < grid_width; x++) {
        gauls.ranks[x] = 0;
        for (int y = 0; y <= gauls.frontLine; y++) {
            if (rand() % 3 != 0) {
                gauls.ranks[x]++;
                gauls.soldiers++;
                map[x][y] = 2;
                addTexture(x, y, "grass_gaul.bmp");
            }
        }
    }
    SDL_RenderPresent(renderer);
}

//shift gauls down one
void gaulAdvance() {
    gauls.frontLine++;
    for (int y = gauls.frontLine; y > gauls.frontLine - 4; y--) {
        for (int x = 0; x < grid_width; x++) {
            addTexture(x, (y - 1), "grass.bmp");
            map[x][y] = map[x][y - 1];
            map[x][y - 1] = 3;
            if (map[x][y] == 2) {
                addTexture(x, y, "grass_gaul.bmp");
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void animateGaul(Point *up) {
    Point *curr = up;
    while(curr != NULL) {
        for (int i = 1; i <= 4; i++) {
            addTextureNoScale(curr->x, curr->y, "grass.bmp");
            addTextureNoScale(curr->x, curr->dir ? curr->y - i * grid_cell_height/4: curr->y + i * grid_cell_height/4, "grass_gaul.bmp");
            battleDelay(20);
            SDL_RenderPresent(renderer);
        }
        curr = curr->next;
    }
}

//if a gaul sees a space infront he moves
void gaulGapFill() {
    for (int y = gauls.frontLine; y > gauls.frontLine - 4; y--) {
        Point *down = NULL;
        for (int x = 0; x < grid_width; x++) {
            if (map[x][y] == 3 && map[x][y - 1] == 2) {
                Point *p = malloc(sizeof *p);
                p->x = x * grid_cell_width;
                p->y = (y - 1) * grid_cell_height;
                p->dir = false;
                p->next = down;
                down = p;
                map[x][y] = 2;
                map[x][y - 1] = 3;
            }
        }
        //reverse(down);
        animateGaul(down);
    }
    SDL_RenderPresent(renderer);
}


void animateRoman(Point *up, Point *left) {
    Point *curr = up;
    while(curr != NULL) {
        for (int i = 1; i <= 4; i++) {
            addTextureNoScale(curr->x, curr->y, "grass.bmp");
            addTextureNoScale(curr->x, curr->dir ? curr->y - i * grid_cell_height/4: curr->y + i * grid_cell_height/4, "grass_roman.bmp");
            battleDelay(20);
            SDL_RenderPresent(renderer);
        }
        curr = curr->next;
    }
    curr = left;
    while(curr != NULL) {
        for (int i = 1; i <= 4; i++) {
            addTextureNoScale(curr->x, curr->y, "grass.bmp");
            addTextureNoScale(curr->dir ? curr->x - i * grid_cell_width/4: curr->x + i * grid_cell_width/4, curr->y, "grass_roman.bmp");
            battleDelay(20);
            SDL_RenderPresent(renderer);
        }
        curr = curr->next;
    }
}

void RomanGapFill() {
    Point *down;
    Point *left;
    for (int y = legion.frontLine; y < legion.frontLine + 3; y++) {
        Point *down = NULL;
        Point *left = NULL;
        for (int x = 0; x < grid_width; x++) {
            if (map[x][y] == 3 && map[x][y + 1] == 1) {//if there's an empty space infront move to occupy
                Point *p = malloc(sizeof *p);
                p->x = x * grid_cell_width;
                p->y = (y + 1) * grid_cell_height;
                p->dir = true;
                p->next = down;
                down = p;
                map[x][y] = 1;
                map[x][y + 1] = 3;
            } else if (y == legion.frontLine + legion.ranks[x] - 1 && map[x][y] == 1 && x > 0 && legion.ranks[x - 1] < legion.ranks[x] - 1 && map[x - 1][y] == 3) {//shift left
                Point *p = malloc(sizeof *p);
                p->x = x * grid_cell_width;
                p->y = y * grid_cell_height;
                p->dir = true;
                p->next = left;
                left = p;
                map[x - 1][y] = 1;
                map[x][y] = 3;
                legion.ranks[x]--;
                legion.ranks[x - 1]++;
            } else if (y == legion.frontLine + legion.ranks[x] - 1 && map[x][y] == 1 && x < grid_width -1 && legion.ranks[x + 1] < legion.ranks[x] - 1 && map[x + 1][y] == 3) {//shift right
                Point *p = malloc(sizeof *p);
                p->x = x * grid_cell_width;
                p->y = y * grid_cell_height;
                p->dir = false;
                p->next = left;
                map[x + 1][y] = 1;
                map[x][y] = 3;
                legion.ranks[x]--;
                legion.ranks[x + 1]++;
            }
        }
        animateRoman(down, left);
    }
    SDL_RenderPresent(renderer);
}


void addGroundcover(int window_width, int window_height) {
    for (int x = 0; x < grid_width; x++) {
        for (int y = 0; y < grid_height; y++) {
            map[x][y] = 3;
            addTexture(x, y, "grass.bmp");
        }
    }
    SDL_RenderPresent(renderer);
}


void initBattleField(int window_width, int window_height) {
    addGroundcover(window_width, window_height);
    loadRomanRanks();
    player.mobile = false;
    player.y = grid_height - legion.ranks[8];
    player.facing = 0;
    loadGaulHorde();
    SDL_RenderPresent(renderer);
}


void romanAdvance() {
    legion.frontLine--;
    player.y --;
    for (int x = 0; x < grid_width; x++) {
        for (int y = legion.frontLine; y < legion.frontLine + legion.ranks[x]; y++) {
            map[x][y] = 1;
            addTexture(x, y, "grass_roman.bmp");
        }
        map[x][legion.frontLine + legion.ranks[x]] = 3;
        addTexture(x, (legion.frontLine + legion.ranks[x]), "grass.bmp");
    }
    map[player.x][player.y] = 0;
    addTexture(player.x, player.y, "grass_player.bmp");
    SDL_RenderPresent(renderer);
}

//swaps player to back of line NEEDS UPDATE TO MAP
void manipleSwap() {
    for (int x = 0; x < legion.ranks[8] - 1; x++) {
        addTexture(player.x, player.y, "grass_roman.bmp");
        player.y++;
        addTexture(player.x, player.y, "grass_player.bmp");
        SDL_RenderPresent(renderer);
        SDL_Delay(1000);
    }
}


//decerments rank, soldiers, changes map, texutre of opp
void romanDeath(int rank) {
    legion.ranks[rank]--;
    legion.soldiers--;
    map[rank][legion.frontLine] = 3;
    addTexture(rank, legion.frontLine, "grass.bmp");
    addTexture(rank, gauls.frontLine, "grass_gaul.bmp");
}

void gaulDeath(int rank) {
    gauls.ranks[rank]--;
    gauls.soldiers--;
    map[rank][gauls.frontLine] = 3;
    addTexture(rank, gauls.frontLine, "grass.bmp");
    addTexture(rank, legion.frontLine, "grass_roman.bmp");
}

void simulateBattlefield() {
    bool gaulAttacking[grid_width];
    bool romanAttacking[grid_width];
    for (int x = 0; x < grid_width; x++) {
        gaulAttacking[x] = false;//index is true when its section of line is attacking
        romanAttacking[x] = false;
        if (map[x][legion.frontLine] == 1 && map[x][gauls.frontLine] == 2) {
            if (rand() > 22000) {
                addTexture(x, gauls.frontLine, "grass_attack.bmp");
                gaulAttacking[x] = true;
            }
            if (rand() > 22000) {
                addTexture(x, legion.frontLine, "grass_attack.bmp");
                romanAttacking[x] = true;
            }
            SDL_RenderPresent(renderer);
        }
    }

    battleDelay(1000);

    for (int x = 0; x < grid_width; x++) {
        if (gaulAttacking[x] && gaulAttacking[x]) {
            if (rand() > 16000) {
                romanDeath(x);
            } else {
                gaulDeath(x);
            }
        } else if (gaulAttacking[x]) {
            if (rand() > 16000 && gaulAttacking[x]) {
                romanDeath(x);
            } else {
                addTexture(x, gauls.frontLine, "grass_gaul.bmp");
            }
        } else if (romanAttacking[x]) {
            if (rand() > 16000) {
                gaulDeath(x);
            } else {
                addTexture(x, legion.frontLine, "grass_roman.bmp");
            }
        }
        SDL_RenderPresent(renderer);
    }
}


void battle(int window_width, int window_height) {
    initBattleField(window_width, window_height);
    gaulAdvance();
    //battleDelay(1000);
    romanAdvance();
    //battleDelay(1000);
    gaulAdvance();
    //battleDelay(1000);
    gaulGapFill();
    //battleDelay(1000);
    while (gauls.soldiers > 10) {
        simulateBattlefield();
        battleDelay(1000);
        RomanGapFill();
        battleDelay(1000);
        gaulGapFill();
        battleDelay(1000);
    }
}

int main(int argc, char *argv[]) {
    // + 1 so that the last grid lines fit in the screen.
    int window_width = grid_width * grid_cell_width + 1;//originally 1045
    int window_height = grid_height * grid_cell_width + 1;//originally 829

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

    player.blocking = false;
    player.x = 8;
    player.y = 7;

    //testing placing textures on grid
    battle(window_width, window_height);

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                    case SDLK_UP:
                        if (player.y - 1 >= 0) {
                            addTexture(player.x, player.y, "grass.bmp");
                            player.y --;
                            player.facing = 0;
                        }
                        break;
                    case SDLK_s:
                    case SDLK_DOWN:
                        if (player.y + 1  < grid_height) {
                            addTexture(player.x, player.y, "grass.bmp");
                            player.y++;
                            player.facing = 2;
                        }
                        break;
                    case SDLK_a:
                    case SDLK_LEFT:
                        if (player.x - 1 >= 0) {
                            addTexture(player.x, player.y, "grass.bmp");
                            player.x --;
                            player.facing = 3;
                        }
                        break;
                    case SDLK_d:
                    case SDLK_RIGHT:
                        if (player.x + 1 < grid_width) {
                            addTexture(player.x, player.y, "grass.bmp");
                            player.x++;
                            player.facing = 1;
                        }
                        break;
                    case SDLK_f:
                        SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g, grid_line_color.b, grid_line_color.a);
                        SDL_Rect range = {
                            .x = player.x * grid_cell_width - grid_cell_width,
                            .y = player.y * grid_cell_height - grid_cell_height,
                            .w = grid_cell_width,
                            .h = grid_cell_height,
                        };
                        for (int y = 0; y < 3; y++) {
                            for (int x = 0; x < 3; x++) {
                                SDL_RenderFillRect(renderer, &range);
                                range.x ++;
                            }
                            range.x -= 3;
                            range.y ++;
                        }
                        SDL_RenderPresent(renderer);
                        break;
                    case SDLK_q:
                        block();
                        break;
                    case SDLK_e:
                        attack();
                        break;
                }
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    grid_cell_height = (int)((event.window.data2 - 1)/(double)grid_height);
                    grid_cell_width = (int)((event.window.data1 - 1)/(double)grid_width);
                    window_height = event.window.data2;
                    window_width = event.window.data1;
                }
                break;
            case SDL_QUIT:
                quit = SDL_TRUE;
                break;
            }
        }


        // Draw player.
        addTexture(player.x, player.y, "grass_player.bmp");

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