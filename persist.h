#ifndef _PERSIST_H
#define _PERSIST_H

int save_grid(Grid grid, const char* file_name);
int load_grid(Grid* grid, const char* file_name);

#endif // _PERSIST_H
