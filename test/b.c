#include "b.h"
#include <stdio.h>

#include "../rit_str.h"

static void *libc_malloc(void *t_ctx, size_t t_size_in_bytes) {
  (void)t_ctx;
  return malloc(t_size_in_bytes);
}

static void libc_free(void *t_ctx, void *t_ptr) {
  (void)t_ctx;
  free(t_ptr);
}

static void *libc_realloc(void *t_ctx, void *t_old_ptr,
                          size_t t_old_size_in_bytes,
                          size_t t_new_size_in_bytes) {
  (void)t_ctx;
  (void)t_old_size_in_bytes;
  return realloc(t_old_ptr, t_new_size_in_bytes);
}

static rstr_allocator allocator = {libc_malloc, libc_free, libc_realloc,
                                   (void *)0};

void test_alloc_b() {
  rstr(str, rsv_lit("From b.c"), &allocator);
  puts(rstr_cstr(str));
  rstr_free(str, &allocator);
}
