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

rda_allocator allocator = {libc_malloc, libc_free, libc_realloc, nullptr};

typedef struct {
  double x, y;
} Vec2;

int main() {
  rda(Vec2, aoVec2, 3, &allocator);
  for (int i = 1; i <= rda_size(aoVec2); i++) {
    rda_at(aoVec2, i).x = i;
    rda_at(aoVec2, i).y = i * 2;
    printf("x: %lf, y: %lf\n", rda_at(aoVec2, i).x, rda_at(aoVec2, i).y);
  }
}
