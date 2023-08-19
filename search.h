#ifndef _SEARCH
#define _SEARCH

void print_map(Map map, Tile start, Tile goal, Tile *path, size_t path_size);
bool node_is_equal(Tile n1, Tile n2);
int a_star(Tile *path, size_t path_size, Tile start, Tile goal, Map map);

#endif  // _SEARCH
