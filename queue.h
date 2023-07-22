int  random_int(int min, int max);

void print_array(int *array, size_t size);
void fill_array_randomly(int *array, size_t size, int min, int max);
void swap_array_values(int *array, size_t i, size_t j);

void enforce_heap_condition(int *heap, size_t index);
void heapify(int *array, int *heap, size_t size);