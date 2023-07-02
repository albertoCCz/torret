#include <math.h>

// function reconstruct_path(cameFrom, current)
//     total_path := {current}
//     while current in cameFrom.Keys:
//         current := cameFrom[current]
//         total_path.prepend(current)
//     return total_path

typedef struct {
    int x;
    int y;
} Vector2i;

void reconstruct_path(Vector2 *path, Vector2 came_from, Vector2 current)
{

}

int h(Vector2i goal, Vector2i node)
{
    return (goal.x >= node.x ? goal.x - node.x : node.x - goal.x) + 
        (goal.y >= node.y ? goal.y - node.y : node.y - goal.y); 
}

typedef struct {
    const size_t width;
    const size_t height;
    int *state;
} Map;

Vector2i *a_star(Vector2i start, Vector2i goal, Map map, int *h)
{
    // TODO: check vector shapes. Most probably some of them can be 1D
    // Vector2i open_set[map.width * map.height] = {0};
    // open_set[0] = start;

    // Vector2i came_from[map.width * map.height] = {0};
    
    // Vector2i g_score[map.width * map.height] = {INFINITY};
    
    // Vector2i f_score[map.width * map.height] = {INFINITY};

    


}



// // A* finds a path from start to goal.
// // h is the heuristic function. h(n) estimates the cost to reach goal from node n.
// function A_Star(start, goal, h)
//     // The set of discovered nodes that may need to be (re-)expanded.
//     // Initially, only the start node is known.
//     // This is usually implemented as a min-heap or priority queue rather than a hash-set.
//     openSet := {start}

//     // For node n, cameFrom[n] is the node immediately preceding it on the cheapest path from the start
//     // to n currently known.
//     cameFrom := an empty map

//     // For node n, gScore[n] is the cost of the cheapest path from start to n currently known.
//     gScore := map with default value of Infinity
//     gScore[start] := 0

//     // For node n, fScore[n] := gScore[n] + h(n). fScore[n] represents our current best guess as to
//     // how cheap a path could be from start to finish if it goes through n.
//     fScore := map with default value of Infinity
//     fScore[start] := h(start)

//     while openSet is not empty
//         // This operation can occur in O(Log(N)) time if openSet is a min-heap or a priority queue
//         current := the node in openSet having the lowest fScore[] value
//         if current = goal
//             return reconstruct_path(cameFrom, current)

//         openSet.Remove(current)
//         for each neighbor of current
//             // d(current,neighbor) is the weight of the edge from current to neighbor
//             // tentative_gScore is the distance from start to the neighbor through current
//             tentative_gScore := gScore[current] + d(current, neighbor)
//             if tentative_gScore < gScore[neighbor]
//                 // This path to neighbor is better than any previous one. Record it!
//                 cameFrom[neighbor] := current
//                 gScore[neighbor] := tentative_gScore
//                 fScore[neighbor] := tentative_gScore + h(neighbor)
//                 if neighbor not in openSet
//                     openSet.add(neighbor)

//     // Open set is empty but goal was never reached
//     return failure