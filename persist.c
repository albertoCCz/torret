#include <stdio.h>
#include <stdlib.h>

#include "grid.h"

int
save_grid(Grid grid, const char* file_name)
{
    FILE* file = fopen(file_name, "wb");
    if (file != NULL) {
        size_t count =  fwrite(&grid.x,                sizeof(grid.x),           1, file);
        count +=        fwrite(&grid.y,                sizeof(grid.y),           1, file);
        count +=        fwrite(&grid.w,                sizeof(grid.w),           1, file);
        count +=        fwrite(&grid.h,                sizeof(grid.h),           1, file);
        count +=        fwrite(&grid.grid_size,        sizeof(grid.grid_size),   1, file);
        count +=        fwrite(&grid.grid_thick,       sizeof(grid.grid_thick),  1, file);
        count +=        fwrite(grid.map->state,        sizeof(*grid.map->state), grid.grid_size * grid.grid_size, file);

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
load_grid(Grid* grid, const char* file_name)
{
    FILE* file = fopen(file_name, "rb");
    if (file != NULL) {
        size_t count = fread(&grid->x,               sizeof(grid->x),           1, file);
        count +=       fread(&grid->y,               sizeof(grid->y),           1, file);
        count +=       fread(&grid->w,               sizeof(grid->w),           1, file);
        count +=       fread(&grid->h,               sizeof(grid->h),           1, file);
        count +=       fread(&grid->grid_size,       sizeof(grid->grid_size),   1, file);
        count +=       fread(&grid->grid_thick,      sizeof(grid->grid_thick),  1, file);
        count +=       fread(grid->map->state,       sizeof(*grid->map->state), grid->grid_size * grid->grid_size, file);

        if (count == 0) printf("PERSIST: [%s] Didn't read anything from file\n", file_name);
        else printf("PERSIST: [%s] Read %zu items from file\n", file_name, count);

        fclose(file);

        return 0;
    } else {
        printf("PERSIST: [%s] Couldn't open file to load grid\n", file_name);
        return 1;
    }
}

int
save_map(Map map, const char* file_name)
{
    
    FILE* file = fopen(file_name, "wb");
    if (file != NULL) {
        size_t count = fwrite(&map.w, sizeof(map.w), 1, file);
        count        = fwrite(&map.h, sizeof(map.h), 1, file);
        count        = fwrite(map.state, sizeof(*map.state), map.w * map.h, file);

        if (count == 0) printf("PERSIST: [%s] Didn't write anything to file\n", file_name);
        else printf("PERSIST: [%s] Wrote %zu items to file\n", file_name, count);

        fclose(file);
        return 0;
    } else {
        printf("PERSIST: [%s] Couldn't open file to save map\n", file_name);
        return 1;
    }
}

int
load_map(Map* map, const char* file_name)
{
    
    FILE* file = fopen(file_name, "rb");
    if (file != NULL) {
        size_t count = fread(&map->w, sizeof(map->w), 1, file);
        count        = fread(&map->h, sizeof(map->h), 1, file);
        count        = fread(map->state, sizeof(*map->state), map->w * map->h, file);

        if (count == 0) printf("PERSIST: [%s] Didn't read anything from file\n", file_name);
        else printf("PERSIST: [%s] Read %zu items to file\n", file_name, count);

        fclose(file);
        return 0;
    } else {
        printf("PERSIST: [%s] Couldn't open file to load map\n", file_name);
        return 1;
    }
}
