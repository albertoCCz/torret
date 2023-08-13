#include <stdio.h>
#include <stdlib.h>

#include "grid.h"

/* typedef enum TileState { */
/*     WALL, */
/*     NORMAL, */
/*     PATH_START, */
/*     PATH_END */
/* } TileState; */

/* typedef struct Grid { */
/*     int x; */
/*     int y; */
/*     int width; */
/*     int height; */
/*     size_t grid_size; */
/*     size_t grid_thick; */
/*     TileState *state; */
/* } Grid; */

int
save_grid(Grid grid, char* file_name)
{
    FILE* file = fopen(file_name, "wb");
    if (file != NULL) {
	size_t count = fwrite(&grid.x,    sizeof(grid.x),          1, file);
	count += fwrite(&grid.y,          sizeof(grid.y),          1, file);
	count += fwrite(&grid.width,      sizeof(grid.width),      1, file);
	count += fwrite(&grid.height,     sizeof(grid.height),     1, file);
	count += fwrite(&grid.grid_size,  sizeof(grid.grid_size),  1, file);
	count += fwrite(&grid.grid_thick, sizeof(grid.grid_thick), 1, file);
	count += fwrite(grid.state,       sizeof(*grid.state),     grid.grid_size * grid.grid_size, file);

	if (count == 0) printf("PERSIST: [%s] Didn't write anything to file\n", file_name);
	else printf("PERSIST: [%s] Wrote %zu items to file\n", file_name, count);

	fclose(file);
	return 0;
    } else {
	printf("PERSIST: [%s] Couldn't open file to save grid\n", file_name);
	return 1;
    }
}

int
load_grid(Grid* grid, char* file_name)
{
    FILE* file = fopen(file_name, "rb");
    if (file != NULL) {
	size_t count =	fread(&grid->x,			sizeof(grid->x),          1, file);
	count +=	fread(&grid->y,			sizeof(grid->y),          1, file);
	count +=	fread(&grid->width,		sizeof(grid->width),      1, file);
	count +=	fread(&grid->height,		sizeof(grid->height),     1, file);
	count +=	fread(&grid->grid_size,		sizeof(grid->grid_size),  1, file);
	count +=	fread(&grid->grid_thick,	sizeof(grid->grid_thick), 1, file);
	count +=	fread(grid->state,		sizeof(*grid->state),     grid->grid_size * grid->grid_size, file);

	if (count == 0) printf("PERSIST: [%s] Didn't read anything from file\n", file_name);
	else printf("PERSIST: [%s] Read %zu items from file\n", file_name, count);

	fclose(file);

	return 0;
    } else {
	printf("PERSIST: [%s] Couldn't open file to load grid\n", file_name);
	return 1;
    }
}
