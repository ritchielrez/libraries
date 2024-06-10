#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../rit_dyn_arr.h"

#define ARENA_ALLOCATOR_IMPLEMENTATION
#include "../arena_allocator.h"

#define nullptr (void *)0

void *arena_allocator_alloc(void *t_arena, size_t t_size_in_bytes) {
  return arena_alloc((Arena *)t_arena, t_size_in_bytes);
}
void arena_allocator_free(void *t_arena, void *t_ptr) {
  (void)t_ptr;
  arena_free(t_arena);
}
void *arena_allocator_realloc(void *t_arena, void *t_old_ptr,
                              size_t t_old_size_in_bytes,
                              size_t t_new_size_in_bytes) {
  return arena_realloc((Arena *)t_arena, t_old_ptr, t_old_size_in_bytes,
                       t_new_size_in_bytes);
}

void *libc_malloc(void *t_ctx, size_t t_size_in_bytes) {
  (void)t_ctx;
  return malloc(t_size_in_bytes);
}

void libc_free(void *t_ctx, void *t_ptr) {
  (void)t_ctx;
  free(t_ptr);
}

void *libc_realloc(void *t_ctx, void *t_old_ptr, size_t t_old_size_in_bytes,
                   size_t t_new_size_in_bytes) {
  (void)t_ctx;
  (void)t_old_size_in_bytes;
  return realloc(t_old_ptr, t_new_size_in_bytes);
}

Arena arena = {nullptr, nullptr};
rda_allocator ctx_allocator = {libc_malloc, libc_free, libc_realloc, nullptr};

int main() {
  rda(int, arr, 10, &ctx_allocator);
  printf("size: %zu, capacity: %zu\n", rda_size(arr), rda_capacity(arr));
  for (int i = 0; i < 10; ++i) {
    rda_at(arr, i) = i + 1;
    printf("%d\n", rda_at(arr, i));
  }
  rda_push_back(arr, 11, &ctx_allocator);
  rda_pop_back(arr);
  rda_insert(arr, 2, 1, 69, &ctx_allocator);
  rda_erase(arr, 1, 1);
  rda_resize(arr, 3, 0, &ctx_allocator);
  for (int *it = rda_begin(arr); it < rda_end(arr); it++) {
    printf("%d\n", *it);
  }
  printf("1st element: %d, last element: %d\n", rda_front(arr), rda_back(arr));
  rda_clear(arr);
  printf("arr.empty() = %d\n", rda_empty(arr));

  printf("size: %zu, capacity: %zu\n", rda_size(arr), rda_capacity(arr));

  rda(int, arr2, 10, &ctx_allocator);
  for (int *it = rda_begin(arr2); it < rda_end(arr2); it++) {
    *it = 0;
  }

  for (int *it = rda_begin(arr); it < rda_end(arr); it++) {
    printf("%d ", *it);
  }
  rda_resize(arr, 30, 1, &ctx_allocator);
  rda_append(arr, &ctx_allocator, 2, 2)
      printf("\n[Before swap]\n1st array: \n");
  for (int *it = rda_begin(arr); it < rda_end(arr); it++) {
    printf("%d ", *it);
  }
  printf("\n2nd array: \n");
  for (int *it = rda_begin(arr2); it < rda_end(arr2); it++) {
    printf("%d ", *it);
  }
  rda_swap(arr, arr2);
  printf("\n[After swap]\n1st array: \n");
  rda_for_each(it, arr) { printf("%d ", *it); }
  printf("\n2nd array: \n");
  rda_for_each(it, arr2) { printf("%d ", *it); }

  rda_cp(int, arr3, arr, 0, rda_size(arr), &ctx_allocator);
  printf("\n3rd array(copy of the 1st one): \n");
  rda_for_each(it, arr3) { printf("%d ", *it); }
  printf("\n\n");

  // int tmp_arr[] = {1, 2, 3, 4, 5, 6};
  // rda_assign_arr(arr, tmp_arr, &ctx_allocator);
  rda_assign(arr, &ctx_allocator, 1, 2, 3, 4, 5, 6);
  rda_for_each(it, arr) { printf("%d ", *it); }

  rda_free(arr, &ctx_allocator);
  rda_free(arr2, &ctx_allocator);
  rda_free(arr3, &ctx_allocator);
}