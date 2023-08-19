#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "grid.h"

#define ARRAY_LEN(x) (sizeof(x)/sizeof((x)[0]))

void print_int_2d(char *label, int *v, size_t w, size_t h)
{
    printf("%s:\n", label);
    for (size_t j = 0; j < h; ++j) {
        for (size_t i = 0; i < w; ++i) {
            printf(" %d ", v[i + j*w]);
        }
        printf("\n");
    }
}

void print_node_1d(char *label, Tile *nodes, size_t nodes_size)
{
    printf("%s:\n", label);
    for (size_t i = 0; i < nodes_size; ++i) {
        printf("(%d,%d) ", nodes[i].x, nodes[i].y);
    }
    printf("\n");
}

void print_path(char *label, Tile *path, size_t nodes_size)
{
    printf("%s:\n", label);
    for (size_t i = 0; i < nodes_size; ++i) {
        if (!(path[i].x == -1 && path[i].y == -1)) {
            printf("(%.0f,%.0f) ", path[i].x, path[i].y);
        }
    }
    printf("\n");
}

void print_node(char *label, Tile node)
{
    printf("%s: (%d,%d)\n", label, node.x, node.y);
}

void print_debug(Tile current, Tile *open_set, size_t open_set_size, Tile *neighbors, int *g_score, int *f_score, Map map)
{
    printf("\n");
    print_int_2d("g_score", g_score, map.w, map.h);
    print_int_2d("f_score", f_score, map.w, map.h);
    print_node_1d("open_set", open_set, open_set_size);
    print_node("Current", current);
    print_node_1d("neighbors", neighbors, 4);
}

size_t node_to_index(Tile node, Map map)
{
    return node.x + node.y * map.w;
}

Tile index_to_node(size_t index, Map map)
{
    return (Tile) {
            .x = index % map.w,
            .y = index / map.w
        };
}

bool node_is_equal(Tile n1, Tile n2)
{
    return n1.x == n2.x && n1.y == n2.y;
}

void insert_char_into_map_str(char *buffer, size_t x, size_t y, size_t w, char c)
{
    buffer[3*(y*w + x) + 0 + y] = ' ';
    buffer[3*(y*w + x) + 1 + y] = c;
    buffer[3*(y*w + x) + 2 + y] = ' ';
}

void print_map(Map map, Tile start, Tile goal, Tile *path, size_t path_size)
{
    char map_buffer[(map.w * map.h * 3) + map.h + 1];
    Tile node;

    // build map string
    for (size_t y = 0; y < map.h; ++y) {
        for (size_t x = 0; x < map.w; ++x) {
            node = index_to_node(x + y*map.w, map);
            if (node_is_equal(node, start)) {
                insert_char_into_map_str(map_buffer, x, y, map.w, 'S');
            } else if (node_is_equal(node, goal)) {
                insert_char_into_map_str(map_buffer, x, y, map.w, 'G');
            } else if (map.state[x + y * (size_t) map.w] == NORMAL) {
                insert_char_into_map_str(map_buffer, x, y, map.w, '.');
            } else if (map.state[x + y * (size_t) map.w] == WALL) {
                insert_char_into_map_str(map_buffer, x, y, (size_t) map.w, 'B');
            }
        }
        map_buffer[3*(y*map.w + map.w) + y] = '\n';
    }

    // include path chars in map string
    if (path != NULL) {
        size_t index;
        for (size_t i = 0; i < path_size; ++i) {
            index = path[i].y * (size_t) map.w + path[i].x;
            node = index_to_node(index, map);
            if (map.state[index] == NORMAL
                && !node_is_equal(node, start)
                && !node_is_equal(node, goal)) insert_char_into_map_str(map_buffer, path[i].x, path[i].y, map.w, 'X');
        }
    }
    
    printf("%s", map_buffer);
}

void initialize_map_state(Map *map, TileState state)
{
    for (size_t y = 0; y < map->h; ++y) {
        for (size_t x = 0; x < map->w; ++x) {
            map->state[(size_t) x + (size_t) y * map->w] = state;
        }
    }
}

bool node_in_set(Tile node, Tile *set, size_t set_size)
{
    for (size_t i = 0; i < set_size; ++i) {
        if (node_is_equal(node, set[i])) {
            return true;
        }
    }
    return false;
}

void node_remove_from_set(Tile node, Tile *set, size_t set_size)
{
    for (size_t i = 0; i < set_size; ++i) {
        if (node_is_equal(node, set[i])) {
            set[i].x = -1;
            set[i].y = -1;
            break;
        }
    }
}

void node_append_to_set(Tile node, Tile *set, size_t set_size)
{
    bool set_is_full = 1;
    for (size_t i = 0; i < set_size; ++i) {
        if (node_is_equal((Tile) {.x = -1, .y = -1}, set[i])) {
            set[i] = node;
            set_is_full = false;
            break;
        }
    }
    if (set_is_full) {
        fprintf(stderr, "Can't append node (%d,%d) because set is full", node.x, node.y);
        EXIT_FAILURE;
    }
}

bool set_is_empty(Tile *set, size_t set_size)
{
    bool is_empty = true;
    for (size_t i = 0; i < set_size; ++i) {
        if (!node_is_equal(set[i], (Tile) {.x=-1, .y=-1})) {
            is_empty = false;
            break;
        }
    }
    return is_empty;
}

void initialize_set(Tile *set, size_t set_size, Tile node)
{
    for (size_t i = 0; i < set_size; ++i) {
        set[i] = node;
    }
}

void initialize_int_set(int *set, size_t set_size, int value)
{
    for (size_t i = 0; i < set_size; ++i) {
        set[i] = value;
    }
}

void get_neighbors(Tile *neighbors, size_t neighbors_size, Tile node, Map map)
{
    initialize_set(neighbors, neighbors_size, (Tile) {.x=-1, .y=-1});
    int range[3] = {-1, 0, 1};
    for (size_t j = 0; j < ARRAY_LEN(range); ++j ) {
        for (size_t i = 0; i < ARRAY_LEN(range); ++i) {
            if (abs(range[i]) + abs(range[j]) == 1) {
                Tile neighbor = {
                    .x = node.x + range[i],
                    .y = node.y + range[j]
                };
                if (0 <= neighbor.x && neighbor.x < map.w &&
                    0 <= neighbor.y && neighbor.y < map.h &&
                    map.state[node_to_index(neighbor, map)] != WALL) node_append_to_set(neighbor, neighbors, neighbors_size);
            }
        }
    }
}

size_t d(Tile n1, Tile n2)
{
    return abs(n1.x - n2.x) + abs(n1.y - n2.y);
}

size_t h(Tile current, Tile goal)
{
    return d(current, goal);
}

void build_path(Tile *path, size_t path_size, Tile start, Tile current, Tile *came_from, Map map)
{
    initialize_set(path, path_size, (Tile) {.x=-1, .y=-1});
    Tile step_node = current;
    while (!node_is_equal(step_node, start)) {
        node_append_to_set(step_node, path, path_size);
        step_node = came_from[node_to_index(step_node, map)];
    }
    node_append_to_set(start, path, path_size);
}

Tile get_current(int *f_score, Tile* open_set, size_t open_set_size, Map map)
{
    Tile node, current;
    int score = map.w * map.h;
    int tentative_score;
    for (size_t i = 0; i < open_set_size; ++i) {
        node = open_set[i];
        if (0 <= node.x && node.x < map.w &&
            0 <= node.y && node.y < map.h) {
                tentative_score = f_score[node_to_index(node, map)];
                if (tentative_score < score) {
                    current = node;
                    score = tentative_score;
                }
            }
    }
    return current;
}

int a_star(Tile *path, size_t path_size, Tile start, Tile goal, Map map)
{
    // initialize open_set just containing start.
    Tile open_set[path_size];
    initialize_set(open_set, path_size, (Tile) {.x=-1, .y=-1});
    open_set[node_to_index(start, map)] = start;

    // declare came_from to keep track of path.
    Tile came_from[path_size];
    initialize_set(came_from, path_size, (Tile) {.x=-1, .y=-1});

    // initialize g_score with g_score(start) = 0.
    int g_score[path_size];
    initialize_int_set(g_score, path_size, path_size);
    g_score[node_to_index(start, map)] = 0;

    // initialize f_score with f_score(start) = g_score(start) + h(start).
    int f_score[path_size];
    initialize_int_set(f_score, path_size, path_size);
    f_score[node_to_index(start, map)] = g_score[node_to_index(start, map)] + h(start, goal);

    Tile current;
    Tile neighbors[4];
    initialize_set(neighbors, ARRAY_LEN(neighbors), (Tile) {.x=-1, .y=-1});
    while (!set_is_empty(open_set, ARRAY_LEN(open_set))) {
        current = get_current(f_score, open_set, ARRAY_LEN(open_set), map);

        // print_debug(current, open_set, ARRAY_LEN(open_set), neighbors, g_score, f_score, map);
        // return EXIT_FAILURE;

        if (node_is_equal(current, goal)) {
            build_path(path, path_size, start, current, came_from, map);
            return 0;
        }

        node_remove_from_set(current, open_set, ARRAY_LEN(open_set));
        get_neighbors(neighbors, ARRAY_LEN(neighbors), current, map);
        for (size_t i = 0; i < ARRAY_LEN(neighbors) && !node_is_equal(neighbors[i], (Tile){.x=-1, .y=-1}); ++i) {
            int tentative_score = g_score[node_to_index(current, map)] + d(current, neighbors[i]);
            if (tentative_score < g_score[node_to_index(neighbors[i], map)]) {
                came_from[node_to_index(neighbors[i], map)] = current;
                g_score[node_to_index(neighbors[i], map)] = tentative_score;
                f_score[node_to_index(neighbors[i], map)] = tentative_score + h(neighbors[i], goal);
                if (!node_in_set(neighbors[i], open_set, ARRAY_LEN(open_set))) {
                    node_append_to_set(neighbors[i], open_set, ARRAY_LEN(open_set));
                }
            }
        }
    }
    // path wasn't found
    return 1;
}
