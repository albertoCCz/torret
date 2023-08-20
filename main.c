#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <raylib.h>

#include "grid.h"
#include "persist.h"
#include "search.h"


#define GRID_SIZE 40 // bug: when GRID_SIZE = {24, 25, 50},
                     // grid_lines do not match with tiles and break

void print_v2(Vector2 v)
{
    printf("vector = (x = %f, y = %f)\n", v.x, v.y);
}

typedef enum Mode {
    MODE_NONE,
    MODE_EDIT,
    MODE_PATH
} Mode;

// current_mode global
Mode current_mode;

typedef struct ModeText {
    char *title;
    int font_size;
    int title_len;
} ModeText;

#define MODE_TXT(MODE) \
    { \
        .title = #MODE " mode", \
        .font_size = 20, \
        .title_len = MeasureText((const char *) #MODE " mode", 20) \
    };

bool tile_equals(Tile t1, Tile t2)
{
    return (t1.x == t2.x && t1.y == t2.y);
}

void index_to_tile(Tile *tile, Grid grid, size_t index)
{
    tile->x = index % grid.grid_size;
    tile->y = index / grid.grid_size;
}

size_t tile_to_index(Grid grid, Tile tile)
{
    return (size_t) tile.y * grid.grid_size + (size_t) tile.x;
}

float get_tile_width(Grid grid)
{
    return (grid.w - grid.grid_size * grid.grid_thick) / (float) grid.grid_size;
}

float get_tile_height(Grid grid)
{
    return (grid.h - grid.grid_size * grid.grid_thick) / (float) grid.grid_size;
}

void get_tile_selected(Tile *tile, int mouse_x, int mouse_y, Grid grid)
{
    if (grid.x <= mouse_x && mouse_x < grid.x + grid.w &&
        grid.y <= mouse_y && mouse_y < grid.y + grid.h) {
        float selection_width = get_tile_width(grid);
        float selection_height = get_tile_height(grid);

        float tile_x, tile_y;
        for (tile_x = 0; tile_x < grid.grid_size; ++tile_x) {
            if ((selection_width + (float) grid.grid_thick) * (tile_x + 1) > mouse_x) break;
        }
        for (tile_y = 0; tile_y < grid.grid_size; ++tile_y) {
            if ((selection_height + (float) grid.grid_thick) * (tile_y + 1) > mouse_y) break;
        }

        assert(0 <= tile_x && tile_x < grid.grid_size);
        assert(0 <= tile_y && tile_y < grid.grid_size);

        tile->x = tile_x;
        tile->y = tile_y;
    } else {
        tile->x = -1;
        tile->y = -1;
    }
}

void draw_grid_lines(Grid grid, Color color)
{
    float tile_width  = get_tile_width(grid);
    float tile_height = get_tile_height(grid);
    for (size_t i = 0; i < grid.grid_size; ++i) {
        DrawLineEx((Vector2) { .x=(tile_width + grid.grid_thick) * i,
                               .y=grid.y
                             },
                   (Vector2) { .x=(tile_width + grid.grid_thick) * i,
                               .y=grid.h-1
                   }, grid.grid_thick, BLACK);
        DrawLineEx((Vector2) { .x=grid.x,
                               .y=(tile_height + grid.grid_thick) * i
                             },
                   (Vector2) { .x=grid.w-1,
                               .y=(tile_height + grid.grid_thick) * i
                   }, grid.grid_thick, BLACK);
    }
}

void draw_grid_selection(Tile tile_selected, Grid grid)
{
    if (0 <= tile_selected.x && 0 <= tile_selected.y) {
        float selection_width  = get_tile_width(grid);
        float selection_height = get_tile_height(grid);

        char c = (char) 255;
        char a = (char) 50;
        DrawRectangle(tile_selected.x * (selection_width + grid.grid_thick),
                      tile_selected.y * (selection_height + grid.grid_thick),
                      selection_width + 1, selection_height + 1,
                      (Color) { .r=c, .g=c, .b=c, .a=a });
    }
}

void draw_grid_state(Grid grid)
{
    float selection_width  = get_tile_width(grid);
    float selection_height = get_tile_height(grid);

    Tile tile;
    Color tile_color;
    
    for (size_t i = 0; i < grid.grid_size*grid.grid_size; ++i) {
        switch (grid.map->state[i]) {
        case (WALL):
            tile_color = BLUE;
            break;
        case (NORMAL):
            tile_color = WHITE;
            break;
        case (PATH_START):
            tile_color = GREEN;
            break;
        case (PATH_END):
            tile_color = RED;
            break;
        case (PATH):
            tile_color = YELLOW;
            break;
        }
        index_to_tile(&tile, grid, i);
        DrawRectangle(tile.x * (selection_width + grid.grid_thick),
                      tile.y * (selection_height + grid.grid_thick),
                      selection_width + 1, selection_height + 1,
                      tile_color);
    }
}

void draw_grid(Tile tile_selected, Grid grid)
{
    draw_grid_state(grid);
    draw_grid_lines(grid, BLACK);
    draw_grid_selection(tile_selected, grid);
}

bool tile_in_map(Tile t, Map map)
{
    return 0 <= t.x && t.x < map.w && 0 <= t.y && t.y < map.h;
}

TileState get_tile_state(Tile t, Grid grid)
{
    return grid.map->state[tile_to_index(grid, t)]; 
}

void change_tile_state(Tile tile, Grid *grid)
{
    if (tile.x != -1 && tile.y != -1) {
        size_t tile_index = tile_to_index(*grid, tile);

	    if (grid->map->state[tile_index] == WALL) grid->map->state[tile_index] = NORMAL;
        else grid->map->state[tile_index] = WALL;
    }
}

void set_search_tile(Tile *start_tile, Tile *end_tile, Tile tile_selected, Grid *grid)
{
    if (current_mode == MODE_EDIT) {
        if (tile_equals(tile_selected, *start_tile)) {
            grid->map->state[tile_to_index(*grid, *start_tile)] = NORMAL;
            grid->map->state[tile_to_index(*grid, *end_tile)] = NORMAL;
	    
            // unset start and end tiles
            start_tile->x = -1;
            start_tile->y = -1;

            end_tile->x = -1;
            end_tile->y = -1;
        } else if (tile_equals(tile_selected, *end_tile)) {
            grid->map->state[tile_to_index(*grid, *end_tile)] = NORMAL;
	    
            // unset end tile
            end_tile->x = -1;
            end_tile->y = -1;
        }
    } else if (current_mode == MODE_PATH) {
        // if tile_selected is valid tile
        if (grid->map->state[tile_to_index(*grid, tile_selected)] != WALL) {
            // if start not set
            if (start_tile->x < 0 && start_tile->y < 0) {
                // if end not set -> set start
                if (end_tile->x < 0 && end_tile->y < 0) {
                    start_tile->x = tile_selected.x;
                    start_tile->y = tile_selected.y;
                    grid->map->state[tile_to_index(*grid, *start_tile)] = PATH_START;
                } else {
                    fprintf(stderr, "Error: 'End' tile can't be set if 'Start' tile isn't set.");
                    exit(1);
                }
            } else {
                // if end not set -> set end
                if (end_tile->x < 0 && end_tile->y < 0) {
                    end_tile->x = tile_selected.x;
                    end_tile->y = tile_selected.y;
                    grid->map->state[tile_to_index(*grid, *end_tile)] = PATH_END;
                } else { // set start && unset end
                    grid->map->state[tile_to_index(*grid, *start_tile)] = NORMAL;
                    grid->map->state[tile_to_index(*grid, *end_tile)] = NORMAL;

                    start_tile->x = tile_selected.x;
                    start_tile->y = tile_selected.y;
                    grid->map->state[tile_to_index(*grid, *start_tile)] = PATH_START;

                    end_tile->x = -1;
                    end_tile->y = -1;
                }
            }
        } else {
            printf("Selected tile is not a valid search tile!\n");
        }
    }
}

void clean_path_in_map(Tile* path, size_t path_size, Grid grid)
{
    size_t index;
    for (size_t i = 0; i < path_size; ++i) {
        index = tile_to_index(grid, path[i]);
        if (grid.map->state[index] != PATH_START &&
            grid.map->state[index] != PATH_END   &&
            grid.map->state[index]) grid.map->state[index] = NORMAL;
    }
}

typedef enum SetPathMode {
    SET_PATH_NEW,
    SET_PATH_REMOVE
} SetPathMode;

void set_path_in_map(Tile* path, size_t path_size, SetPathMode path_mode, Tile* start_tile, Tile* end_tile, Grid grid)
{
    if (path_mode == SET_PATH_NEW) {
        for (size_t i = 0; i < path_size; ++i) {
            if (!node_is_equal(*start_tile, path[i]) && !node_is_equal(*end_tile, path[i])) {
                grid.map->state[tile_to_index(grid, path[i])] = PATH;
            }
        }
    } else if (path_mode == SET_PATH_REMOVE) {
        start_tile->x = -1;
        start_tile->y = -1;
        
        end_tile->x = -1;
        end_tile->y = -1;
        
        for (size_t i = 0; i < path_size; ++i) {
            grid.map->state[tile_to_index(grid, path[i])] = NORMAL;
            path[i] = (Tile) { .x=-1, .y=-1 };
        }
    }
}

void print_grid_state(Grid grid)
{
    printf("\n");
    for (size_t y = 0; y < grid.grid_size; ++y) {
        for (size_t x = 0; x < grid.grid_size; ++x) {
            switch (grid.map->state[x + grid.grid_size * y]) {
            case (WALL):
                printf("W ");
                break;
            case (NORMAL):
                printf("N ");
                break;
            case (PATH_START):
                printf("S ");
                break;
            case (PATH_END):
                printf("E ");
                break;
            }
        }
        printf("\n");
    }
    printf("\n");
}

int main(void)
{
    // init window
    int factor = 60;
    int width = 16*factor;
    int height = 9*factor;
    SetTargetFPS(60);
    InitWindow(width, height, "Torret");

    // main objects
    TileState *state = (TileState*)malloc(GRID_SIZE * GRID_SIZE * sizeof(TileState));
    memset(state, (int)WALL, GRID_SIZE * GRID_SIZE * sizeof(TileState));
    Map map = {
        .w = GRID_SIZE,
        .h = GRID_SIZE,
        .state = state
    };

    Grid grid = {
        .x = 0,
        .y = 0,
        .w = width,
        .h = height,
        .grid_size = GRID_SIZE,
        .grid_thick = 1,
        .map = (Map*) &map
    };

    int mouse_x, mouse_y;
    Tile tile_selected;
    
    Tile start_tile = {
        .x = -1,
        .y = -1
    };
    Tile end_tile = {
        .x = -1,
        .y = -1
    };

    size_t path_size = GRID_SIZE * GRID_SIZE;
    Tile *path = (Tile*) malloc(path_size * sizeof(Tile));
    for (size_t i = 0; i < path_size; ++i) path[i] = (Tile) { .x=-1, .y=-1 };
    
    // MODES
    // edit map mode
    ModeText edit_mt = MODE_TXT(EDIT);
    
    // search mode
    ModeText path_mt = MODE_TXT(PATH);

    // event loop
    while(!WindowShouldClose())
    {
        // get selected tile
      	mouse_x = GetMouseX();
        mouse_y = GetMouseY();
        get_tile_selected(&tile_selected, mouse_x, mouse_y, grid);

        // search path
        if (tile_in_map(start_tile, map)) {
            if (tile_in_map(end_tile, map)) {
                clean_path_in_map(path, path_size, grid);
                if (a_star(path, path_size, start_tile, end_tile, map) && get_tile_state(end_tile, grid) == NORMAL) {
                    fprintf(stderr, "Path form (%d,%d) to (%d,%d) couldn't be found...\n",
                            start_tile.x, start_tile.y,
                            end_tile.x,   end_tile.y);
                }
                set_path_in_map(path, path_size, SET_PATH_NEW, &start_tile, &end_tile, grid);
            } else if (tile_in_map(tile_selected, map)) {
                clean_path_in_map(path, path_size, grid);
                if (a_star(path, path_size, start_tile, tile_selected, map) && get_tile_state(tile_selected, grid) == NORMAL) {
                    fprintf(stderr, "Path form (%d,%d) to (%d,%d) couldn't be found...\n",
                            start_tile.x, start_tile.y,
                            tile_selected.x,   tile_selected.y);
                }
                set_path_in_map(path, path_size, SET_PATH_NEW, &start_tile, &tile_selected, grid);
            }
        }

        // draw
        BeginDrawing();
            ClearBackground(BLUE);
            draw_grid(tile_selected, grid);
            if (current_mode == MODE_EDIT) {
                DrawText(edit_mt.title, width/2 - edit_mt.title_len/2, 0, edit_mt.font_size, GREEN);
            }
            if (current_mode == MODE_PATH) {
                DrawText(path_mt.title, width/2 - path_mt.title_len/2, 0, path_mt.font_size, GREEN);
            }
        EndDrawing();

        // Handle input
        if (current_mode == MODE_EDIT) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                change_tile_state(tile_selected, &grid);
                set_search_tile(&start_tile, &end_tile, tile_selected, &grid);
            }

            if (IsKeyPressed(KEY_S)) {    // Save Map
                char* file_name = "./maps/map001.map";
                save_map(*grid.map, file_name);
                // save_grid(grid, file_name);
            }
            if (IsKeyPressed(KEY_L)) {    // Load Map
                char* file_name = "./maps/map001.map";
                load_map(grid.map, file_name);
                // load_grid(&grid, file_name);
            }	    
        }

        if (current_mode == MODE_PATH) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                set_search_tile(&start_tile, &end_tile, tile_selected, &grid);
            }

            if (IsKeyPressed(KEY_R)) {
                set_path_in_map(path, path_size, SET_PATH_REMOVE, &start_tile, &end_tile, grid);
            }
        }

        // activate/deactivate modes
        if (IsKeyPressed(KEY_P)) {
            if (current_mode == MODE_PATH) current_mode = MODE_NONE;
            else current_mode = MODE_PATH;   
        }
        if (IsKeyPressed(KEY_E)) {
            if (current_mode == MODE_EDIT) current_mode = MODE_NONE;
            else current_mode = MODE_EDIT;
        }
    }

    free(state);
    free(path);
    CloseWindow();

    return 0;
}
