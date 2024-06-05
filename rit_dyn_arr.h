// LICENSE
// See end of the file for license information.

#ifndef RIT_DYN_ARR_H_INCLUDED
#define RIT_DYN_ARR_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/// @brief Custom allocator interface.
/// Functions allocating memory takes a custom allocator based off this
/// interface as a parameter.
typedef struct {
  void *(*alloc)(void *, size_t);
  void (*free)(void *, void *);
  void *(*realloc)(void *, void *, size_t, size_t);
  void *m_ctx;  // The arena, stack or etc where the memory would be allocated,
                // NULL if none
} rda_allocator;

/// @brief Holds the metadata about the array.
/// This metadata struct is stored as header of the array
typedef struct {
  size_t m_size;
  size_t m_capacity;
  size_t m_objsize;
} rda_metadata;

/// @brief Get the pointer to the metadata header struct of an array.
/// @param t_rda The array
#define rda_get_metadata(t_rda) (&((rda_metadata *)t_rda)[-1])

/// @internal
inline void *rda_alloc(const char *t_file, int t_line, size_t t_objsize,
                       size_t t_size, rda_allocator *t_allocator) {
  rda_metadata *arr = (rda_metadata *)t_allocator->alloc(
      t_allocator->m_ctx, sizeof(rda_metadata) + t_objsize * t_size * 2);
  if (!arr) {
    fprintf(stderr, "Error: allocation failed, file: %s, line: %d\n", t_file,
            t_line);
    exit(EXIT_FAILURE);
  }
  arr->m_size = t_size;
  arr->m_objsize = t_objsize;
  arr->m_capacity = t_size * 2;
  arr += 1;
  return (void *)arr;
}

inline void rda_free(void *t_rda, rda_allocator *t_allocator) {
  t_allocator->free(t_allocator->m_ctx, rda_get_metadata(t_rda));
}

inline size_t rda_size(void *t_rda) { return rda_get_metadata(t_rda)->m_size; }

inline size_t rda_capacity(void *t_rda) {
  return rda_get_metadata(t_rda)->m_capacity;
}

/// @brief Set the capacity of an array.
#define rda_reserve(t_rda, t_new_capacity, t_allocator) \
  rda_realloc(__FILE__, __LINE__, &t_rda, t_new_capacity, t_allocator)

/// @internal
inline void rda_realloc(const char *t_file, int t_line, void **t_rda,
                        size_t t_new_capacity, rda_allocator *t_allocator) {
  if (t_new_capacity > rda_capacity(*t_rda)) {
    size_t objsize = rda_get_metadata(*t_rda)->m_objsize;
    rda_metadata *arr = (rda_metadata *)t_allocator->realloc(
        t_allocator->m_ctx, rda_get_metadata(*t_rda),
        sizeof(rda_metadata) + objsize * rda_capacity(*t_rda),
        sizeof(rda_metadata) + objsize * t_new_capacity);
    if (!arr) {
      fprintf(stderr, "Error: reallocation failed, file: %s, line: %d\n",
              t_file, t_line);
      exit(EXIT_FAILURE);
    }
    arr += 1;
    *t_rda = (void *)arr;
    rda_get_metadata(*t_rda)->m_capacity = t_new_capacity;
  }
}

/// @internal
inline bool rda_index_bounds_check(const char *t_file, int t_line, void *t_rda,
                                   size_t t_index) {
  if (t_index < rda_size(t_rda)) return true;
  fprintf(stderr, "Error: array index is out of bounds, file: %s, line: %d\n",
          t_file, t_line);
  exit(EXIT_FAILURE);
}

/// @param Check if an array is empty.
inline bool rda_empty(void *t_rda) { return rda_size(t_rda) == 0; }

/// @brief Empty out an array.
inline void rda_clear(void *t_rda) { rda_get_metadata(t_rda)->m_size = 0; }

#define rda_swap(t_rda, t_rda_other) \
  do {                               \
    void *tmp = t_rda;               \
    t_rda = t_rda_other;             \
    t_rda_other = tmp;               \
  } while (0)

/// @brief Makes a non-binding request to make the capacity of an array equal to
/// its size. In this library this is definied as a no-op function.
inline void shrink_to_fit(void *t_rda) { (void)t_rda; }

/// @brief Initialize a rda.
#define rda(t_type, t_rda, t_size, t_allocator)                           \
  t_type *t_rda = (t_type *)rda_alloc(__FILE__, __LINE__, sizeof(t_type), \
                                      t_size, t_allocator)

#define rda_at(t_rda, t_index)                                 \
  (rda_index_bounds_check(__FILE__, __LINE__, t_rda, t_index)) \
      ? t_rda[t_index]                                         \
      : t_rda[t_index]

#define rda_set(t_rda, t_index)                                     \
  if (rda_index_bounds_check(__FILE__, __LINE__, t_rda, t_index)) { \
  }                                                                 \
  t_rda[t_index]

/// @param t_rda Where to copy
/// @param t_rda_other What to copy
#define rda_copy(t_rda, t_rda_other, t_allocator)             \
  rda_reserve(t_rda_other, rda_capacity(t_rda), t_allocator); \
  rda_get_metadata(t_rda_other)->m_size = rda_size(t_rda);    \
  for (size_t i = 0; i < rda_size(t_rda); i++) {              \
    rda_set(t_rda_other, i) = rda_at(t_rda, i);               \
  }

/// @brief Get the pointer to the first element of an array
#define rda_begin(t_rda) (&(t_rda[0]))
/// @brief Get the pointer to the past-the-end element of an array
#define rda_end(t_rda) (&(t_rda[rda_size(t_rda)]))

/// @brief Get the first element of an array
#define rda_front(t_rda) (t_rda[0])
/// @brief Get the last element of an array
#define rda_back(t_rda) (t_rda[rda_size(t_rda) - 1])

#define rda_push_back(t_rda, t_value, t_allocator)              \
  if (rda_capacity(t_rda) <= rda_size(t_rda) + 1) {             \
    rda_reserve(t_rda, (rda_size(t_rda) + 1) * 2, t_allocator); \
  }                                                             \
  rda_get_metadata(t_rda)->m_size++;                            \
  rda_set(t_rda, rda_size(t_rda) - 1) = t_value

#define rda_pop_back(t_rda) rda_get_metadata(t_rda)->m_size--

#define rda_append(t_rda, t_count, t_value, t_allocator) \
  for (int i = 1; i <= t_count; i++) {                   \
    rda_push_back(t_rda, t_value, t_allocator);          \
  }

#define rda_resize(t_rda, t_count, t_value, t_allocator) \
  rda_clear(t_rda);                                      \
  rda_append(t_rda, t_count, t_value, t_allocator)

/// @brief Insert a value in the array before t_pos.
#define rda_insert(t_rda, t_pos, t_value, t_allocator)    \
  rda_push_back(t_rda, 0, t_allocator);                   \
  for (size_t i = rda_size(t_rda) - 1; i >= t_pos; i--) { \
    rda_set(t_rda, i) = rda_at(t_rda, i - 1);             \
    rda_set(t_rda, i - 1) = 0;                            \
  }                                                       \
  rda_set(t_rda, t_pos - 1) = t_value

/// @brief Remove the value in the array at t_pos.
#define rda_erase(t_rda, t_pos)                          \
  for (size_t i = t_pos + 1; i < rda_size(t_rda); i++) { \
    rda_set(t_rda, i - 1) = rda_at(t_rda, i);            \
  }                                                      \
  rda_pop_back(t_rda)

#endif  // RIT_DYN_ARR_H_INCLUDED

/*
The MIT License (MIT)

Copyright 2024 Ritchiel Reza

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
