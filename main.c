#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef struct Army {
    int ranks[17];
    int soldiers;
    int frontLine;
} Army;

typedef struct Player {
    int x;
    int y;
    SDL_bool blocking;
    SDL_bool mobile;
    int facing;
} Player;

typedef struct Point {
    int x;
    int y;
    SDL_bool dir;//SDL_TRUE when moving left or up
    struct Point *next;
} Point;

SDL_bool quit = SDL_FALSE;
#define grid_width 17
#define grid_height 10
int grid_cell_width = 40;
int grid_cell_height = 40;
// + 1 so that the last grid lines fit in the screen.
int window_width = grid_width * 40;//originally 1045
int window_height = grid_height * 40;//originally 829
//Player:0 | Friendly NPC:1 | Hostile NPC: 2 | grass: 3
int map[17][10];
Player player;
Army legion;
Army gauls;

SDL_Renderer *renderer;

void loadMenu() {
    SDL_Surface* surface = SDL_LoadBMP("grass.bmp");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect tile = {
        .x = 0,
        .y = 0,
        .w = window_width,
        .h = window_height,
    };
    SDL_RenderCopy(renderer, texture, NULL, &tile);

    int menuSelection = 0;
    SDL_bool quitMenu = SDL_FALSE;
    while(!quitMenu) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_RETURN:
                            switch (menuSelection) {
                                case 0:
                                    quitMenu = SDL_TRUE;
                                    break;
                                case 1:
                                    quitMenu = SDL_TRUE;
                                    quit = SDL_TRUE;
                                    break;
                            }
                            break;
                        case SDLK_w:
                        case SDLK_UP:
                            if (menuSelection != 0) {
                                menuSelection--;
                            }
                            break;
                        case SDLK_s:
                        case SDLK_DOWN:
                            if (menuSelection != 1) {
                                menuSelection++;
                            }
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

        if (menuSelection == 0) {
            surface = SDL_LoadBMP("grass.bmp");
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            tile.x = window_width/3,
            tile.y = window_height/2,
            tile.w = window_width/3,
            tile.h = grid_cell_height,
            SDL_RenderCopy(renderer, texture, NULL, &tile);
        } else {
            surface = SDL_LoadBMP("grass_attack.bmp");
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            tile.x = window_width/3,
            tile.y = window_height/2,
            tile.w = window_width/3,
            tile.h = grid_cell_height,
            SDL_RenderCopy(renderer, texture, NULL, &tile);
        }

        if (menuSelection == 1) {
            surface = SDL_LoadBMP("grass.bmp");
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            tile.x = window_width/3,
            tile.y = window_height/1.5,
            tile.w = window_width/3,
            tile.h = grid_cell_height,
            SDL_RenderCopy(renderer, texture, NULL, &tile);
        } else {
            surface = SDL_LoadBMP("grass_attack.bmp");
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            tile.x = window_width/3,
            tile.y = window_height/1.5,
            tile.w = window_width/3,
            tile.h = grid_cell_height,
            SDL_RenderCopy(renderer, texture, NULL, &tile);
        }

        SDL_RenderPresent(renderer);
    }
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void addTexture(int x, int y, const char* file) {
    SDL_Surface* surface = SDL_LoadBMP(file);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_Rect tile = {
        .x = x * grid_cell_width,
        .y = y * grid_cell_height,
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

void block() {
    player.blocking = !player.blocking;
    if (player.blocking) {
        addTexture(player.x, player.y, "grass_player_blocking.bmp");
        player.mobile = SDL_FALSE;
    } else {
        addTexture(player.x, player.y, "grass_player.bmp");
        player.mobile = SDL_TRUE;
    }
    SDL_RenderPresent(renderer);
}

void attack() {
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
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
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
            if (rand() > RAND_MAX/3) {
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
            map[x][y] = map[x][y - 1];//bring map values down
            map[x][y - 1] = 3;//replace old value with grass
            if (map[x][y] == 2) {
                addTexture(x, y, "grass_gaul.bmp");
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void animateMove(Point *vertical, Point *horizontal, const char* file) {
    for (int i = 0; i <= 4; i++) {
        Point *vertIndex = vertical;
        Point *horIndex = horizontal;
        while(vertIndex != NULL || horIndex != NULL) {
            if (vertIndex != NULL) {
                addTextureNoScale(vertIndex->x, vertIndex->y, "grass.bmp");
                addTextureNoScale(vertIndex->x, vertIndex->dir ? vertIndex->y - i * grid_cell_height/4: vertIndex->y + i * grid_cell_height/4, file);
            }

            if (horIndex != NULL) {
                addTextureNoScale(horIndex->x, horIndex->y, "grass.bmp");
                addTextureNoScale(horIndex->dir ? horIndex->x - i * grid_cell_width/4: horIndex->x + i * grid_cell_width/4, horIndex->y, file);
            }
            if (vertIndex != NULL) {vertIndex = vertIndex->next;}
            if (horIndex != NULL) {horIndex = horIndex->next;}
        }
        SDL_RenderPresent(renderer);
        battleDelay(30);
        free(vertIndex);
        free(horIndex);
    }
}

//does not work
void addPoint(Point *head, int x, int y, SDL_bool dir) {
    Point *p = malloc(sizeof *p);
    p->x = x;
    p->y = y;
    p->dir = dir;
    p->next = head;
    head = p;
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
                p->dir = SDL_FALSE;
                p->next = down;
                down = p;
                map[x][y] = 2;
                map[x][y - 1] = 3;
            }
        }
        animateMove(down, NULL, "grass_gaul.bmp");
    }
    SDL_RenderPresent(renderer);
}


void romanGapFill() {
    for (int y = legion.frontLine; y < legion.frontLine + 3; y++) {
        Point *down = NULL;
        Point *left = NULL;
        for (int x = 0; x < grid_width; x++) {
            if (map[x][y] == 3 && map[x][y + 1] == 1) {//if there's an empty space infront move to occupy
                Point *p = malloc(sizeof *p);
                p->x = x * grid_cell_width;
                p->y = (y + 1) * grid_cell_height;
                p->dir = SDL_TRUE;
                p->next = down;
                down = p;
                map[x][y] = 1;
                map[x][y + 1] = 3;
            } else if (y == legion.frontLine + legion.ranks[x] - 1 && map[x][y] == 1 && x > 0 && legion.ranks[x - 1] < legion.ranks[x] - 1 && map[x - 1][y] == 3) {//shift left
                Point *p = malloc(sizeof *p);
                p->x = x * grid_cell_width;
                p->y = y * grid_cell_height;
                p->dir = SDL_TRUE;
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
                p->dir = SDL_FALSE;
                p->next = left;
                map[x + 1][y] = 1;
                map[x][y] = 3;
                legion.ranks[x]--;
                legion.ranks[x + 1]++;
            }
        }
        animateMove(down, left, "grass_roman.bmp");
    }
    SDL_RenderPresent(renderer);
}


void addGroundcover() {
    SDL_Surface* surface = SDL_LoadBMP("grass.bmp");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_Rect tile = {
        .w = grid_cell_width,
        .h = grid_cell_height,
    };
    
    SDL_RenderCopy(renderer, texture, NULL, &tile);
    for (int x = 0; x < grid_width; x++) {
        tile.x = x * grid_cell_width;
        for (int y = 0; y < grid_height; y++) {
            tile.y = y * grid_cell_height;
            map[x][y] = 3;
            SDL_RenderCopy(renderer, texture, NULL, &tile);
        }
    }
    SDL_DestroyTexture(texture);
    SDL_RenderPresent(renderer);
}


void initBattleField() {
    addGroundcover();
    loadRomanRanks();
    player.mobile = SDL_FALSE;
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
    SDL_bool gaulAttacking[grid_width] = {SDL_FALSE};//index is SDL_TRUE when its section of line is attacking
    SDL_bool romanAttacking[grid_width] = {SDL_FALSE};
    for (int x = 0; x < grid_width; x++) {
        if (map[x][legion.frontLine] == 1 && map[x][gauls.frontLine] == 2) {
            if (rand()%101 > 80) {
                addTexture(x, gauls.frontLine, "grass_attack.bmp");
                gaulAttacking[x] = SDL_TRUE;
            }
            if (rand()%101 > 80) {
                addTexture(x, legion.frontLine, "grass_attack.bmp");
                romanAttacking[x] = SDL_TRUE;
            }
            SDL_RenderPresent(renderer);
        }
    }

    battleDelay(500);

    for (int x = 0; x < grid_width; x++) {
        if (gaulAttacking[x] && gaulAttacking[x]) {
            if (rand() > RAND_MAX/2) {
                romanDeath(x);
                romanGapFill();
            } else {
                gaulDeath(x);
            }
        } else if (gaulAttacking[x]) {
            if (rand() > RAND_MAX/2 && gaulAttacking[x]) {
                romanDeath(x);
                romanGapFill();

            } else {
                addTexture(x, gauls.frontLine, "grass_gaul.bmp");//changes texture back
            }
        } else if (romanAttacking[x]) {
            if (rand() > RAND_MAX/2) {
                gaulDeath(x);
                gaulGapFill();
            } else {
                addTexture(x, legion.frontLine, "grass_roman.bmp");
            }
        }
        SDL_RenderPresent(renderer);
    }
}


void battle() {
    initBattleField();
    gaulAdvance();
    //battleDelay(1000);
    romanAdvance();
    //battleDelay(1000);
    gaulAdvance();
    //battleDelay(1000);
    gaulGapFill();
    //battleDelay(1000);
    while (gauls.soldiers > 10 && !quit) {
        simulateBattlefield();
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window;
    if (SDL_CreateWindowAndRenderer(window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Create window and renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetWindowTitle(window, "SDL Grid");


    player.blocking = SDL_FALSE;
    player.x = 8;
    player.y = 7;

    loadMenu();
    battle();

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
                        SDL_SetRenderDrawColor(renderer, 44, 44, 44, 255);
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

        SDL_RenderPresent(renderer);
    }

    //destroy resources before quitting
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}