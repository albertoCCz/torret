#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <raylib.h>

// #define DEBUG

#define GRID_SIZE 40 // bug: when GRID_SIZE = {24, 25, 50},
                     // grid_lines do not match with tiles and break

void print_v2(Vector2 v)
{
    printf("vector = (x = %f, y = %f)\n", v.x, v.y);
}

typedef enum Mode {
    NONE,
    EDIT,
    PATH
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


typedef enum TileState {
    WALL,
    NORMAL,
    PATH_START,
    PATH_END
} TileState;

typedef struct Grid {
    int x;
    int y;
    int width;
    int height;
    size_t grid_size;
    size_t grid_thick;
    TileState *state;
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

typedef Vector2 Tile;

bool tile_equals(Tile t1, Tile t2)
{
    return (t1.x == t2.x && t1.y == t2.y);
}

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
    int selection_width = grid.width / grid.grid_size;
    int selection_height = grid.height / grid.grid_size;

    Tile tile_selected;

    char c = (char) 255;
    char a = (char) 255;
    Color tile_color;
    
    for (size_t i = 0; i < grid.grid_size*grid.grid_size; ++i) {
	switch (grid.state[i]) {
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
	}
	index_to_tile(&tile_selected, grid, i);
	DrawRectangle(tile_selected.x * selection_width + grid.x,
		      tile_selected.y * selection_height + grid.y,
		      selection_width - 1, selection_height - 1,
		      tile_color);
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

	//grid->state[tile_index] = PATH_START;
        if (grid->state[tile_index] == WALL) grid->state[tile_index] = NORMAL;
        else grid->state[tile_index] = WALL;
    }
}

void set_search_tile(Tile *start_tile, Tile *end_tile, Tile tile_selected, Grid *grid)
{
    if (current_mode == EDIT) {
	if (tile_equals(tile_selected, *start_tile)) {
	    grid->state[tile_to_index(*grid, *start_tile)] = NORMAL;
	    grid->state[tile_to_index(*grid, *end_tile)] = NORMAL;
	    
	    // unset start and end tiles
	    start_tile->x = -1;
	    start_tile->y = -1;

	    end_tile->x = -1;
	    end_tile->y = -1;
	} else if (tile_equals(tile_selected, *end_tile)) {
	    grid->state[tile_to_index(*grid, *end_tile)] = NORMAL;
	    
	    // unset end tile
	    end_tile->x = -1;
	    end_tile->y = -1;
	}
    } else if (current_mode == PATH) {
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
}

void print_grid_state(Grid grid)
{
    printf("\n");
    for (size_t y = 0; y < grid.grid_size; ++y) {
        for (size_t x = 0; x < grid.grid_size; ++x) {
	    switch (grid.state[x + grid.grid_size * y]) {
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
    int factor = 40;
    int width = 16*factor;
    int height = 9*factor;
    SetTargetFPS(60);
    InitWindow(width, height, "Torret");

    // main objects
    TileState *state = calloc(GRID_SIZE * GRID_SIZE, sizeof(TileState));
    memset(state, WALL, sizeof(state)/sizeof(state[0]));
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
    ModeText edit_mt = MODE_TXT(EDIT);
    
    // search mode
    ModeText path_mt = MODE_TXT(PATH);
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
            if (current_mode == EDIT) {
                DrawText(edit_mt.title, width/2 - edit_mt.title_len/2, 0, edit_mt.font_size, GREEN);
            }
            if (current_mode == PATH) {
                DrawText(path_mt.title, width/2 - path_mt.title_len/2, 0, path_mt.font_size, GREEN);
            }
        EndDrawing();

        // INPUT
        // if in edit mode and click on grid tile, change tile state
        if (current_mode == EDIT && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            get_tile_selected(&tile_selected, grid);
            change_tile_state(tile_selected, &grid);
	    set_search_tile(&start_tile, &end_tile, tile_selected, &grid);
        }

        // if in search mode and click on grid tile, set target tile
        if (current_mode == PATH && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            get_tile_selected(&tile_selected, grid);
            set_search_tile(&start_tile, &end_tile, tile_selected, &grid);
        }

        // activate/deactivate modes
        if (IsKeyPressed(KEY_P)) {
	    if (current_mode == PATH) current_mode = NONE;
	    else current_mode = PATH;   
	}
        if (IsKeyPressed(KEY_E)) {
	    if (current_mode == EDIT) current_mode = NONE;
	    else current_mode = EDIT;
	}
    }

    free(state);
    CloseWindow();

    return 0;
}
