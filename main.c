#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <raylib.h>

// #define DEBUG

#define GRID_SIZE 10

void print_v2(Vector2 v)
{
    printf("vector = (x = %f, y = %f)\n", v.x, v.y);
}

typedef enum Mode {
    EDIT,
    SEARCH
} Mode;

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

typedef struct Grid {
    int x;
    int y;
    int width;
    int height;
    size_t grid_size;
    size_t grid_thick;
    int *state;
} Grid;

void draw_grid_lines(Grid grid, Color color)
{
    for (size_t i = 0; i < grid.grid_size; ++i) {
        DrawLineEx((Vector2) { .x=grid.width/(float) grid.grid_size * i,
                               .y=grid.y
                             },
                   (Vector2) { .x=grid.width/(float) grid.grid_size * i,
                               .y=grid.height-1
                   }, grid.grid_thick, BLACK);
        DrawLineEx((Vector2) { .x=grid.x,
                               .y=grid.height/(float) grid.grid_size * i
                             },
                   (Vector2) { .x=grid.width-1,
                               .y=grid.height/(float) grid.grid_size * i
                   }, grid.grid_thick, BLACK);
    }
}

typedef enum TileState {
    WALL,
    NORMAL,
    PATH_START,
    PATH_END
} TileState;

typedef Vector2 Tile;

void get_tile_selected(Tile *tile, Grid grid)
{
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();

#ifdef DEBUG
    char pos_log[200];
    sprintf(pos_log, "mouse: x=%d, y=%d", mouse_x, mouse_y);
    DrawText(pos_log, 0, 0, 30, YELLOW);
#endif

    if (grid.x <= mouse_x && mouse_x < grid.x + grid.width &&
        grid.y <= mouse_y && mouse_y < grid.y + grid.height) {
        int selection_width = grid.width / grid.grid_size;
        int selection_height = grid.height / grid.grid_size;
        int selection_x = (mouse_x - grid.x) / selection_width;
        int selection_y = (mouse_y - grid.y) / selection_height;

#ifdef DEBUG
    char tile_select[200];
    sprintf(tile_select, "tile: x=%d, y=%d", selection_x, selection_y);
    DrawText(tile_select, 0, grid.height-30, 30, YELLOW);
#endif

        assert(0 <= selection_x && selection_x < grid.grid_size);
        assert(0 <= selection_y && selection_y < grid.grid_size);

        tile->x = selection_x;
        tile->y = selection_y;
    } else {
        tile->x = -1;
        tile->y = -1;
    }
}

void draw_grid_selection(Grid grid)
{

    Tile tile_selected;
    get_tile_selected(&tile_selected, grid);

    if (0 <= tile_selected.x && 0 <= tile_selected.y) {
        int selection_width = grid.width / grid.grid_size;
        int selection_height = grid.height / grid.grid_size;

        char c = (char) 255;
        char a = (char) 50;
        DrawRectangle(tile_selected.x * selection_width + grid.x,
                      tile_selected.y * selection_height + grid.y,
                      selection_width - 1, selection_height - 1,
                      (Color) { .r=c, .g=c, .b=c, .a=a });
    }
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

void draw_grid_state(Grid grid)
{
    // TODO: properly handle drawing based on TileState
    int selection_width = grid.width / grid.grid_size;
    int selection_height = grid.height / grid.grid_size;

    Tile tile_selected;

    char c = (char) 255;
    char a = (char) 255;

    for (size_t i = 0; i < grid.grid_size*grid.grid_size; ++i) {
        if (0 < grid.state[i]) {
            index_to_tile(&tile_selected, grid, i);
            DrawRectangle(tile_selected.x * selection_width + grid.x,
                          tile_selected.y * selection_height + grid.y,
                          selection_width - 1, selection_height - 1,
                          (Color) { .r=c, .g=c, .b=c, .a=a });
        }
    }
}

void draw_grid(Grid grid)
{
    draw_grid_state(grid);
    draw_grid_lines(grid, BLACK);
    draw_grid_selection(grid);
}

void change_tile_state(Tile tile, Grid *grid)
{
    if (tile.x != -1 && tile.y != -1) {
        size_t tile_index = tile_to_index(*grid, tile);

        if (grid->state[tile_index] <= 0) grid->state[tile_index] = 1;
        else grid->state[tile_index] = 0;
    }
}

void set_search_tile(Tile *start_tile, Tile *end_tile, Tile tile_selected, Grid *grid)
{
    // if tile_selected is valid tile
    if (grid->state[tile_to_index(*grid, tile_selected)] != WALL) {
        // if start not set
        if (start_tile->x < 0 && start_tile->y < 0) {
            // if end not set -> set start
            if (end_tile->x < 0 && end_tile->y < 0) {
                start_tile->x = tile_selected.x;
                start_tile->y = tile_selected.y;
                grid->state[tile_to_index(*grid, *start_tile)] = PATH_START;
            } else {
                fprintf(stderr, "Error: 'End' tile can't be set if 'Start' tile isn't set.");
                exit(1);
            }
        } else {
            // if end not set -> set end
            if (end_tile->x < 0 && end_tile->y < 0) {
                end_tile->x = tile_selected.x;
                end_tile->y = tile_selected.y;
                grid->state[tile_to_index(*grid, *end_tile)] = PATH_END;
            } else { // set start && unset end
                start_tile->x = tile_selected.x;
                start_tile->y = tile_selected.y;
                grid->state[tile_to_index(*grid, *start_tile)] = PATH_START;

                end_tile->x = -1;
                end_tile->y = -1;
                grid->state[tile_to_index(*grid, *end_tile)] = NORMAL;
            }
        }
    } else {
        printf("Selected tile is not a valid search tile!\n");
    }
}

void print_grid_state(Grid grid)
{
    printf("\n");
    for (size_t y = 0; y < grid.grid_size; ++y) {
        for (size_t x = 0; x < grid.grid_size; ++x) {
            printf("%d ", 0 < grid.state[y * grid.grid_size + x]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(void)
{
    // init window
    int factor = 40;
    int width = 16*factor;
    int height = 9*factor;
    SetTargetFPS(60);
    InitWindow(width, height, "ray-platformer");

    // main objects
    int state[GRID_SIZE * GRID_SIZE] = {0};
    Grid grid = {
        .x = 0,
        .y = 0,
        .width = width,
        .height = height,
        .grid_size = GRID_SIZE,
        .grid_thick = 1,
        .state = state
    };
    int mouse_x, mouse_y;
    Tile tile_selected;

    // MODES
    // edit map mode
    int edit_mode = 0;
    ModeText edit_mt = MODE_TXT(EDIT);
    
    // search mode
    int search_mode = 0;
    ModeText search_mt = MODE_TXT(SEARCH);
    Tile start_tile = {
        .x = -1,
        .y = -1
    };
    Tile end_tile = {
        .x = -1,
        .y = -1
    };

    // event loop
    while(!WindowShouldClose())
    {
        mouse_x = GetMouseX();
        mouse_y = GetMouseY();
        BeginDrawing();
            ClearBackground(BLUE);
            draw_grid(grid);
            if (edit_mode == 1) {
                DrawText(edit_mt.title, width/2 - edit_mt.title_len/2, 0, edit_mt.font_size, GREEN);
            }
            if (search_mode == 1) {
                DrawText(search_mt.title, width/2 - search_mt.title_len/2, 0, search_mt.font_size, GREEN);
            }
        EndDrawing();

        // INPUT
        // if in edit mode and click on grid tile, change tile state
        if (edit_mode && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            get_tile_selected(&tile_selected, grid);
            change_tile_state(tile_selected, &grid);
        }

        // if in search mode and click on grid tile, set target tile
        if (search_mode && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            get_tile_selected(&tile_selected, grid);
            set_search_tile(&start_tile, &end_tile, tile_selected, &grid);
        }

        // activate/deactivate modes
        if (IsKeyPressed(KEY_S)) search_mode = (search_mode + 1) % 2; // alternate between 0 and 1 each time
        if (IsKeyPressed(KEY_E)) edit_mode = (edit_mode + 1) % 2;
    }

    CloseWindow();

    return 0;
}