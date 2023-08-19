#include <raylib.h>

typedef struct Tile {
    int x;
    int y;
} Tile;

typedef enum TileState {
    WALL,
    NORMAL,
    PATH_START,
    PATH_END,
    PATH
} TileState;

typedef struct Map {
    int w;
    int h;
    TileState *state;
} Map;

typedef struct Grid {
    int x;
    int y;
    int w;
    int h;
    size_t grid_size;
    size_t grid_thick;
    Map* map;
} Grid;
