#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define ARRAY_LEN(x) (sizeof(x)/sizeof((x)[0]))

#define MAP_WIDTH  5
#define MAP_HEIGHT 5

typedef struct {
    int x;
    int y;
} Node;

typedef enum {
    EMPTY,
    WALL
} State;

typedef struct {
    const size_t w;
    const size_t h;
    State *state;
} Map;

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

void print_node_1d(char *label, Node *nodes, size_t nodes_size)
{
    printf("%s:\n", label);
    for (size_t i = 0; i < nodes_size; ++i) {
        printf("(%d,%d) ", nodes[i].x, nodes[i].y);
    }
    printf("\n");
}

void print_path(char *label, Node *path, size_t nodes_size)
{
    printf("%s:\n", label);
    for (size_t i = 0; i < nodes_size; ++i) {
        if (!(path[i].x == -1 && path[i].y == -1)) {
            printf("(%d,%d) ", path[i].x, path[i].y);
        }
    }
    printf("\n");
}

void print_node(char *label, Node node)
{
    printf("%s: (%d,%d)\n", label, node.x, node.y);
}

void print_debug(Node current, Node *open_set, size_t open_set_size, Node *neighbors, int *g_score, int *f_score, Map map)
{
    printf("\n");
    print_int_2d("g_score", g_score, map.w, map.h);
    print_int_2d("f_score", f_score, map.w, map.h);
    print_node_1d("open_set", open_set, open_set_size);
    print_node("Current", current);
    print_node_1d("neighbors", neighbors, 4);
}

size_t node_to_index(Node node, Map map)
{
    return node.x + node.y * map.w;
}

Node index_to_node(size_t index, Map map)
{
    return (Node) {
            .x = index % map.w,
            .y = index / map.w
        };
}

bool node_is_equal(Node n1, Node n2)
{
    return n1.x == n2.x && n1.y == n2.y;
}

void print_map(Map map, Node start, Node goal)
{
    Node node;
    for (size_t y = 0; y < map.h; ++y) {
        for (size_t x = 0; x < map.w; ++x) {
            node = index_to_node(x + y*map.w, map);
            if (node_is_equal(node, start)) {
                printf(" S ");
            } else if (node_is_equal(node, goal)) {
                printf(" G ");
            } else if (map.state[(size_t) x + (size_t) y * map.w] == EMPTY) {
                printf(" · ");
            } else if (map.state[(size_t) x + (size_t) y * map.w] == WALL) {
                printf(" B ");
            }
        }
        printf("\n");
    }
}

void initialize_map_state(Map *map, State state)
{
    for (size_t y = 0; y < map->h; ++y) {
        for (size_t x = 0; x < map->w; ++x) {
            map->state[(size_t) x + (size_t) y * map->w] = state;
        }
    }
}

bool node_in_set(Node node, Node *set, size_t set_size)
{
    for (size_t i = 0; i < set_size; ++i) {
        if (node_is_equal(node, set[i])) {
            return true;
        }
    }
    return false;
}

void node_remove_from_set(Node node, Node *set, size_t set_size)
{
    for (size_t i = 0; i < set_size; ++i) {
        if (node_is_equal(node, set[i])) {
            set[i].x = -1;
            set[i].y = -1;
            break;
        }
    }
}

void node_append_to_set(Node node, Node *set, size_t set_size)
{
    bool set_is_full = 1;
    for (size_t i = 0; i < set_size; ++i) {
        if (node_is_equal((Node) {.x = -1, .y = -1}, set[i])) {
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

bool set_is_empty(Node *set, size_t set_size)
{
    bool is_empty = true;
    for (size_t i = 0; i < set_size; ++i) {
        if (!node_is_equal(set[i], (Node) {.x=-1, .y=-1})) {
            is_empty = false;
            break;
        }
    }
    return is_empty;
}

void initialize_set(Node *set, size_t set_size, Node node)
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

void get_neighbors(Node *neighbors, size_t neighbors_size, Node node, Map map)
{
    initialize_set(neighbors, neighbors_size, (Node) {.x=-1, .y=-1});
    int range[3] = {-1, 0, 1};
    for (size_t j = 0; j < ARRAY_LEN(range); ++j ) {
        for (size_t i = 0; i < ARRAY_LEN(range); ++i) {
            if (abs(range[i]) + abs(range[j]) == 1) {
                Node neighbor = {
                    .x = node.x + range[i],
                    .y = node.y + range[j]
                };
                if (0 <= neighbor.x && neighbor.x < map.w &&
                    0 <= neighbor.y && neighbor.y < map.h) {
                        node_append_to_set(neighbor, neighbors, neighbors_size);
                }
            }
        }
    }
}

size_t d(Node n1, Node n2)
{
    return abs(n1.x - n2.x) + abs(n1.y - n2.y);
}

size_t h(Node current, Node goal)
{
    return d(current, goal);
}

void build_path(Node *path, size_t path_size, Node start, Node current, Node *came_from, Map map)
{
    initialize_set(path, path_size, (Node) {.x=-1, .y=-1});
    Node step_node = current;
    while (!node_is_equal(step_node, start)) {
        node_append_to_set(step_node, path, path_size);
        step_node = came_from[node_to_index(step_node, map)];
    }
    node_append_to_set(start, path, path_size);
}

Node get_current(int *f_score, Node* open_set, size_t open_set_size, Map map)
{
    Node node, current;
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

Node *a_star(Node *path, size_t path_size, Node start, Node goal, Map map)
{
    // initialize open_set just containing start.
    Node open_set[MAP_WIDTH * MAP_HEIGHT];
    initialize_set(open_set, MAP_WIDTH * MAP_HEIGHT, (Node) {.x=-1, .y=-1});
    open_set[node_to_index(start, map)] = start;

    // declare came_from to keep track of path.
    Node came_from[MAP_WIDTH * MAP_HEIGHT];
    initialize_set(came_from, MAP_WIDTH * MAP_HEIGHT, (Node) {.x=-1, .y=-1});

    // initialize g_score with g_score(start) = 0.
    int g_score[MAP_WIDTH * MAP_HEIGHT];
    initialize_int_set(g_score, MAP_WIDTH * MAP_HEIGHT, MAP_WIDTH * MAP_HEIGHT);
    g_score[node_to_index(start, map)] = 0;

    // initialize f_score with f_score(start) = g_score(start) + h(start).
    int f_score[MAP_WIDTH * MAP_HEIGHT];
    initialize_int_set(f_score, MAP_WIDTH * MAP_HEIGHT, MAP_WIDTH * MAP_HEIGHT);
    f_score[node_to_index(start, map)] = g_score[node_to_index(start, map)] + h(start, goal);

    Node current;
    Node neighbors[4];
    while (!set_is_empty(open_set, ARRAY_LEN(open_set))) {
        current = get_current(f_score, open_set, ARRAY_LEN(open_set), map);

        // print_debug(current, open_set, ARRAY_LEN(open_set), neighbors, g_score, f_score, map);
        // return EXIT_FAILURE;

        if (node_is_equal(current, goal)) {
            build_path(path, path_size, start, current, came_from, map);
            return path;
        }

        node_remove_from_set(current, open_set, ARRAY_LEN(open_set));
        get_neighbors(neighbors, ARRAY_LEN(neighbors), current, map);
        for (size_t i = 0; i < ARRAY_LEN(neighbors); ++i) {
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
    fprintf(stderr, "Goal node (%d,%d) is unrecheable from start node (%d,%d)",
            goal.x, goal.y,
            start.x, start.y);
    return (Node *) NULL;
}

int main(void)
{
    Node start = {.x=0, .y=0};
    Node goal  = {.x=3, .y=3};

    size_t map_w = 5, map_h = 5;
    State map_state[map_w*map_h];
    Map map = {
        .w = map_w,
        .h = map_h,
        .state = map_state
    };
    initialize_map_state(&map, EMPTY);
    print_map(map, start, goal);

    Node path[map.w * map.h];
    initialize_set(path, ARRAY_LEN(path), (Node) {.x=-1, .y=-1});

    a_star(path, ARRAY_LEN(path), start, goal, map);
    print_path("Path", path, ARRAY_LEN(path));
}
