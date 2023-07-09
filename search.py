import math
import sys
import logging

from search_utils import Node, Map, State, \
    h, d, \
    get_current, get_neighboors, build_path, \
    node_to_index, print_2d, print_path, print_map

logging.basicConfig(level=logging.INFO)
LOG = logging.getLogger('search.py')


def print_debug_info(current: Node, open_set: list[Node], g_score: list[int], f_score: list[int]):
    LOG.debug('\n')
    LOG.debug('Current: %s', current)
    LOG.debug('Open set: %s', open_set)
    LOG.debug('g_score:')
    if LOG.getEffectiveLevel() == logging.DEBUG:
        print_2d(g_score, map.w, map.h)
    LOG.debug('f_score:')
    if LOG.getEffectiveLevel() == logging.DEBUG:
        print_2d(f_score, map.w, map.h)


def search(start: Node, goal: Node, map: Map):
    # The set of discovered nodes that may need to be (re-)expanded.
    open_set: list[Node] = [start]

    # For node n, came_from(n) is the node immediately preceding it on the cheapest path from the start
    # to n currently known.
    came_from: list[Node] = [Node(-1, -1) for _ in range(map.w * map.h)]
    
    # For node n, g_score(n) is the cost of the cheapest path from start to n currently known.
    g_score: list[int] = [math.inf for _ in range(map.w * map.h)]
    g_score[node_to_index(start, map)] = 0
    
    # For node n, f_score(n) := g_score(n) + h(n). f_score(n) represents our current best guess as to
    # how cheap a path could be from start to finish if it goes through n.
    f_score: list[int] = [math.inf for _ in range(map.w * map.h)]
    f_score[node_to_index(start, map)] = h(start, goal)

    while open_set:
        current = get_current(f_score, open_set, map)
        
        print_debug_info(current, open_set, g_score, f_score)
        
        if current == goal:
            return build_path(came_from, map, start, current)
        
        LOG.debug('Removing "current" from open_set...')
        open_set.remove(current)
        for neigh in get_neighboors(current, map):
            tentative_score = g_score[node_to_index(current, map)] + d(current, neigh)
            if tentative_score < g_score[node_to_index(neigh, map)]:
                came_from[node_to_index(neigh, map)] = current
                g_score[node_to_index(neigh, map)]   = tentative_score
                f_score[node_to_index(neigh, map)]   = tentative_score + h(neigh, goal)
                if neigh not in open_set:
                    open_set.append(neigh)

    LOG.error('Goal node (%s,%s) unrecheable from start node (%s,%s)',
              goal.x, goal.y,
              start.x, start.y)
    print_map(map, start, goal)
    sys.exit(1)


if __name__ == '__main__':
    start = Node(0, 0)
    goal  = Node(7, 4)

    # define map
    map_w, map_h = 8, 8
    map_state = [State.EMPTY for _ in range(map_w * map_h)]
    map_state[3 + 0*map_w] = State.WALL
    map_state[3 + 1*map_w] = State.WALL
    map_state[3 + 2*map_w] = State.WALL
    map_state[1 + 4*map_w] = State.WALL
    map_state[2 + 2*map_w] = State.WALL
    map_state[2 + 3*map_w] = State.WALL
    map_state[0 + 4*map_w] = State.WALL
    map = Map(map_w, map_h, map_state)

    # search path
    path = search(start, goal, map)
    print_path(path, map)
