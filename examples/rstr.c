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
  // Non-null terminated strings work fine, as the length needs to be specified
  // with `rsv_cstr()`. The length can be bigger than the actual length of the
  // string when the `rstr` or `rsv` is being constructed. The length given to
  // `rsv_cstr()` is the maximum possible length of the c string.
  char cstr[5] = {'H', 'e', 'l', 'l', 'o'};
  rstr(str, rsv_cstr(cstr, 5), &allocator);

  rstr_cp(str2, str, 0, rstr_size(&str), &allocator);
  // For string literals, `rsv_lit()` can be used. However, due to use of
  // `sizeof()` for length calculations, they cannot be used for c strings that
  // are stored in stack or heap.
  rstr_assign(&str2, rsv_lit("Hello world"), &allocator);
  rstr_append_str(&str2, rsv_lit("ello world"), &allocator);

  rstr_push_back(&str, '1', &allocator);
  rstr_pop_back(&str);
  rstr_insert(&str, 0, 1, 't', &allocator);
  rstr_erase(&str, 0, 1);
  rstr_replace(&str, 0, 5, rsv_lit("hell"), &allocator);
  printf("size: %zu, cap: %zu, str: ", rstr_size(&str), rstr_capacity(&str));
  for (const char *it = rstr_begin(&str); it < rstr_end(&str); it++) {
    putchar(*it);
  }
  printf("\n");
  rstr_clear(&str);
  printf("rstr_empty(str) = %s\n", rstr_empty(&str) ? "true" : "false");

  rstr_resize(&str2, 12, 'h', &allocator);
  printf("size: %zu, cap: %zu, str2: %s\n", rstr_size(&str2),
         rstr_capacity(&str2), rstr_data(&str2));
  printf("str2: %s\n", rstr_data(&str2));
  rstr(str3, RSV_NULL, &allocator);
  rstr_getline(stdin, &str3, &allocator);
  printf("str3: %s\n", rstr_data(&str3));
  FILE *file = fopen("input.txt", "r");
  rstr(str4, RSV_NULL, &allocator);
  rstr_getstream(file, &str4, &allocator);
  printf("str4 (from input.txt): %s", rstr_data(&str4));

  rstr_free(&str, &allocator);
  rstr_free(&str2, &allocator);
  rstr_free(&str3, &allocator);
  rstr_free(&str4, &allocator);

  const char *cstr2 = rstr_data(&str); // same as rstr_cstr(str)
  (void)cstr2;
  // May cause program to crash, you are not suppose to free this pointer.
  // free(cstr);
}
