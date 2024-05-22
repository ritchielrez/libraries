#define _CRT_SECURE_NO_WARNINGS

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RIT_STR_IMPLEMENTATION
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
  rstr(str, rsv_lit("hello world"), &allocator);

  rstr_cp(str2, 0, rstr_size(str), str, &allocator);
  rstr_assign(str2, rsv_lit("Hello world"), &allocator);
  rstr_append_str(str2, rsv_lit("ello world"), &allocator);

  rstr_push_back(str, '1', &allocator);
  rstr_pop_back(str);
  rstr_insert(str, 0, 1, 't', &allocator);
  rstr_erase(str, 0, 1);
  rstr_replace(str, 0, 5, "hell", &allocator);
  printf("size: %zu, cap: %zu, str: ", rstr_size(str),
         rstr_capacity(str));
  for (char *it = rstr_begin(str); it < rstr_end(str); it++) {
    putchar(*it);
  }
  printf("\n");
  rstr_clear(str);
  printf("rstr_empty(str) = %s\n", rstr_empty(str) ? "true" : "false");

  rstr_resize(str2, 12, 'h', &allocator);
  printf("size: %zu, cap: %zu, str: %s\n", rstr_size(str2),
         rstr_capacity(str2), str2);

  const char *cstr = rstr_data(str);  // same as rstr_cstr(str)
  (void)cstr;
  // May cause program to crash, you are not suppose to free this pointer.
  // free(cstr);
}
