#pragma once

#include "../rit_str.h"

typedef struct rstr ritstr;

void *libc_malloc(void *, size_t);
void libc_free(void *, void *);
void *libc_realloc(void *, void *, size_t, size_t);
void test_alloc_a();
