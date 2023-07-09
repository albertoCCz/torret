import math
from collections import deque
from enum import Enum


class Node:
    def __init__(self, x: int, y: int):
        self.x = x
        self.y = y

    def __eq__(self, node):
        return self.x == node.x and self.y == node.y

    def __repr__(self) -> str:
        return f"{self.x, self.y}"
    
    def __str__(self) -> str:
        return f"{self.x, self.y}"


class State(Enum):
    WALL = 'wall'
    EMPTY = 'empty'


class Map:
    def __init__(self, w: int, h: int, state: list[State]):
        self.w = w
        self.h = h
        self.state = state


def node_to_index(node: Node, map: Map) -> int:
    return node.x + map.w * node.y


def index_to_node(index: int, map: Map) -> Node:
    return Node(index % map.w, index // map.w)


def d(n1: Node, n2: Node) -> int:
    return abs(n1.x - n2.x) + abs(n1.y - n2.y)


def h(node: Node, goal: Node) -> int:
    return d(node, goal)


def build_path(came_from: list[Node], map: Map, start: Node, current: Node) -> list[Node]:
    path = deque([])
    while current != start:
        path.appendleft(current)
        current = came_from[node_to_index(current, map)]
    
    path.appendleft(start)
    return path


def node_in_map(node: Node, map: Map) -> bool:
    return 0 <= node.x < map.w and \
           0 <= node.y < map.h


def get_neighboors(node: Node, map: Map) -> list[Node]:
    neighboors = []
    for y in range(-1, 2):
        for x in range(-1, 2):
            if abs(x) + abs(y) == 1:
                possible_neigh = Node(node.x + x, node.y + y)
                if node_in_map(possible_neigh, map) and \
                   map.state[node_to_index(possible_neigh, map)] == State.EMPTY:
                    neighboors.append(possible_neigh)

    return neighboors


def is_node_in_open_set(node: Node, open_set: list[Node]) -> bool:
    for set_node in open_set:
        if set_node.x == node.x and set_node.y == node.y:
            return True
    
    return False


def get_current(f_score: list[int], open_set: list[Node], map: Map) -> Node:
    score = math.inf
    for node in open_set:
        if f_score[node_to_index(node, map)] <= score:
            score = f_score[node_to_index(node, map)]
            current = node

    return current


def remove_from_open_set(node: Node, open_set: list[Node]) -> list[Node]:
    for set_node in open_set:
        if set_node.x == node.x and set_node.y == node.y:
            open_set.remove(set_node)
            return open_set


def print_2d(l: list[int], w: int, h: int) -> None:
    assert len(l) == w * h, "length of 'l' must be 'w' times 'h'"
    for y in range(h):
        for x in range(w):
            print('%8s (%s,%s)' % (l[x + y * w], x, y), end='')
        print('')


def print_path(path: list[Node], map: Map) -> None:
    counter = 0
    for y in range(map.h):
        for x in range(map.w):
            node = Node(x, y)
            if node in path:
                print(f'{"X ":3>s}', end='')
                counter += 1
            else:
                print(f"{'B ' if State.WALL == map.state[node_to_index(node, map)] else '· ':3>s}", end='')
        print('')


def print_map(map: Map, start: Node, goal: Node) -> None:
    for y in range(map.h):
        for x in range(map.w):
            node = Node(x, y)
            if node == start:
                print(f"{'S ':3>s}", end='')
            elif node == goal:
                print(f"{'G ':3>s}", end='')
            else:
                print(f"{'B ' if State.WALL == map.state[node_to_index(node, map)] else '· ':3>s}", end='')
        print('')