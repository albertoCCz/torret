#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <raylib.h>
#include "search.h"

// #define DEBUG

#define GRID_SIZE 10

void print_v2(Vector2 v)
{
    printf("vector = (x = %f, y = %f)\n", v.x, v.y);
}

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

void get_tile_selected(Vector2 *tile, Grid grid)
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

    Vector2 tile_selected;
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

void index_to_tile(Vector2 *tile, Grid grid, size_t index)
{
    tile->x = index % grid.grid_size;
    tile->y = index / grid.grid_size;
}

void tile_to_index(size_t *index, Grid grid, Vector2 tile)
{
    *index = (size_t) tile.y * grid.grid_size + (size_t) tile.x;
}

void draw_grid_state(Grid grid)
{
    int selection_width = grid.width / grid.grid_size;
    int selection_height = grid.height / grid.grid_size;

    Vector2 tile_selected;

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

void change_tile_state(Vector2 tile, Grid *grid)
{
    size_t tile_index;
    tile_to_index(&tile_index, *grid, tile);

    if (grid->state[tile_index] <= 0) grid->state[tile_index] = 1;
    else grid->state[tile_index] = 0;
}

int main(void)
{
    // init window
    int factor = 40;
    int width = 16*factor;
    int height = 9*factor;
    InitWindow(width, height, "ray-platformer");

    // setup grid
    int state[GRID_SIZE];
    for (int i = 0; i < GRID_SIZE; i++) {
        state[i] = 0;
    }
    Grid grid = {
        .x = 0,
        .y = 0,
        .width = width,
        .height = height,
        .grid_size = GRID_SIZE,
        .grid_thick = 1,
        .state = state
    };

    // other declarations
    int mouse_x, mouse_y;
    Vector2 tile_selected;

    // event loop
    while(!WindowShouldClose())
    {
        mouse_x = GetMouseX();
        mouse_y = GetMouseY();
        BeginDrawing();
            ClearBackground(BLUE);
            draw_grid(grid);
        EndDrawing();
 
        // if click on grid tile, change tile state
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
            && grid.x <= mouse_x && mouse_x < grid.x + grid.width
            && grid.y <= mouse_y && mouse_y < grid.y + grid.height) {
                get_tile_selected(&tile_selected, grid);
                change_tile_state(tile_selected, &grid);
        }

    }

    CloseWindow();

    return 0;
}