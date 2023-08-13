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
