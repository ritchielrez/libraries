#define _CRT_SECURE_NO_WARNINGS

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../rit_str.h"

#define nullptr (void *)0

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

rstr_allocator allocator = {libc_malloc, libc_free, libc_realloc, nullptr};

int main() {
  rstr(str, rsv_cstr("rstr", 4), &allocator);
  rsv sv = rsv_cstr("C string", 8);
  rsv sv1 = rsv_rstr(&str);
  rsv sv2 = rsv_rsv(rsv_cstr("rsv", 3));
  puts(rsv_get(sv));
  puts(rsv_get(sv1));
  puts(rsv_get(sv2));
  rstr_free(&str, &allocator);
  return 0;
}
