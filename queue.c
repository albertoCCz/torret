#include <stdlib.h>
#include <assert.h>

#define SEED 2342343
#define heap_parent_index(index) ((index - 1) / 2) >= 0 ? (index - 1) / 2 : index

void print_array(int *array, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        printf(" %d ", array[i]);
    }
    printf("\n");
}

int random_int(int min, int max)
{
    assert(min <= max && "Error generating random int, min value is greater than max value");
    return ((float) rand() / RAND_MAX) * (max - min) + min;
}

void fill_array_randomly(int *array, size_t size, int min, int max)
{
    for (size_t i = 0; i < size; ++i) {
        array[i] = random_int(min, max);
    }
}

void swap_array_values(int *array, size_t i, size_t j)
{
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

void enforce_heap_condition(int *heap, size_t index)
{
    size_t parent_index = heap_parent_index(index);
    int temp;
    while ((heap[parent_index] < heap[index]) && (0 < index)) {
        swap_array_values(heap, parent_index, index);

        index = parent_index;
        parent_index = heap_parent_index(parent_index);
    }
}

void heapify(int *array, int *heap, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        heap[i] = array[i];
        enforce_heap_condition(heap, i);
    }
}
