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
} rit_dyn_arr_allocator;

/// @brief Holds the metadata about the array.
/// This metadata struct is stored as header of the array
typedef struct {
  size_t m_size;
  size_t m_capacity;
  size_t m_objsize;
} rit_dyn_arr_metadata;

/// @brief Get the pointer to the metadata header struct of an array.
/// @param t_rit_dyn_arr The array
#define rit_dyn_arr_get_metadata(t_rit_dyn_arr) \
  (&((rit_dyn_arr_metadata *)t_rit_dyn_arr)[-1])

/// @internal
inline void *rit_dyn_arr_alloc(const char *t_file, int t_line, size_t t_objsize,
                               size_t t_size,
                               rit_dyn_arr_allocator *t_allocator) {
  rit_dyn_arr_metadata *arr = (rit_dyn_arr_metadata *)t_allocator->alloc(
      t_allocator->m_ctx,
      sizeof(rit_dyn_arr_metadata) + t_objsize * t_size * 2);
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

inline void rit_dyn_arr_free(void *t_rit_dyn_arr,
                             rit_dyn_arr_allocator *t_allocator) {
  t_allocator->free(t_allocator->m_ctx, t_rit_dyn_arr);
}

inline size_t rit_dyn_arr_size(void *t_rit_dyn_arr) {
  return rit_dyn_arr_get_metadata(t_rit_dyn_arr)->m_size;
}

inline size_t rit_dyn_arr_capacity(void *t_rit_dyn_arr) {
  return rit_dyn_arr_get_metadata(t_rit_dyn_arr)->m_capacity;
}

/// @brief Set the capacity of an array.
#define rit_dyn_arr_reserve(t_rit_dyn_arr, t_new_capacity, t_allocator)   \
  rit_dyn_arr_realloc(__FILE__, __LINE__, &t_rit_dyn_arr, t_new_capacity, \
                      t_allocator)

/// @internal
inline void rit_dyn_arr_realloc(const char *t_file, int t_line,
                                void **t_rit_dyn_arr, size_t t_new_capacity,
                                rit_dyn_arr_allocator *t_allocator) {
  if (t_new_capacity > rit_dyn_arr_capacity(*t_rit_dyn_arr)) {
    size_t objsize = rit_dyn_arr_get_metadata(*t_rit_dyn_arr)->m_objsize;
    rit_dyn_arr_metadata *arr = (rit_dyn_arr_metadata *)t_allocator->realloc(
        t_allocator->m_ctx, rit_dyn_arr_get_metadata(*t_rit_dyn_arr),
        sizeof(rit_dyn_arr_metadata) +
            objsize * rit_dyn_arr_capacity(*t_rit_dyn_arr),
        sizeof(rit_dyn_arr_metadata) + objsize * t_new_capacity);
    if (!arr) {
      fprintf(stderr, "Error: reallocation failed, file: %s, line: %d\n",
              t_file, t_line);
      exit(EXIT_FAILURE);
    }
    arr += 1;
    *t_rit_dyn_arr = (void *)arr;
    rit_dyn_arr_get_metadata(*t_rit_dyn_arr)->m_capacity = t_new_capacity;
  }
}

/// @internal
inline bool rit_dyn_arr_index_bounds_check(const char *t_file, int t_line,
                                           void *t_rit_dyn_arr,
                                           size_t t_index) {
  if (t_index < rit_dyn_arr_size(t_rit_dyn_arr)) return true;
  fprintf(stderr, "Error: array index is out of bounds, file: %s, line: %d\n",
          t_file, t_line);
  exit(EXIT_FAILURE);
}

/// @param Check if an array is empty.
inline bool rit_dyn_arr_empty(void *t_rit_dyn_arr) {
  return rit_dyn_arr_size(t_rit_dyn_arr) == 0;
}

/// @brief Empty out an array.
inline void rit_dyn_arr_clear(void *t_rit_dyn_arr) {
  rit_dyn_arr_get_metadata(t_rit_dyn_arr)->m_size = 0;
}

#define rit_dyn_arr_swap(t_rit_dyn_arr, t_rit_dyn_arr_other) \
  do {                                                       \
    void *tmp = t_rit_dyn_arr;                               \
    t_rit_dyn_arr = t_rit_dyn_arr_other;                     \
    t_rit_dyn_arr_other = tmp;                               \
  } while (0)

/// @brief Makes a non-binding request to make the capacity of an array equal to
/// its size. In this library this is definied as a no-op function.
inline void shrink_to_fit(void *t_rit_dyn_arr) { (void)t_rit_dyn_arr; }

/// @brief Initialize a rit_dyn_arr.
#define rit_dyn_arr(t_type, t_rit_dyn_arr, t_size, t_allocator) \
  t_type *t_rit_dyn_arr = (t_type *)rit_dyn_arr_alloc(          \
      __FILE__, __LINE__, sizeof(t_type), t_size, t_allocator)

#define rit_dyn_arr_at(t_rit_dyn_arr, t_index)                                 \
  (rit_dyn_arr_index_bounds_check(__FILE__, __LINE__, t_rit_dyn_arr, t_index)) \
      ? t_rit_dyn_arr[t_index]                                                 \
      : t_rit_dyn_arr[t_index]

#define rit_dyn_arr_set(t_rit_dyn_arr, t_index, t_value)                \
  if (rit_dyn_arr_index_bounds_check(__FILE__, __LINE__, t_rit_dyn_arr, \
                                     t_index)) {                        \
    t_rit_dyn_arr[t_index] = t_value;                                   \
  }

/// @param t_rit_dyn_arr Where to copy
/// @param t_rit_dyn_arr_other What to copy
#define rit_dyn_arr_copy(t_rit_dyn_arr, t_rit_dyn_arr_other, t_allocator)      \
  rit_dyn_arr_reserve(t_rit_dyn_arr_other,                                     \
                      rit_dyn_arr_capacity(t_rit_dyn_arr), t_allocator);       \
  rit_dyn_arr_get_metadata(t_rit_dyn_arr_other)->m_size =                      \
      rit_dyn_arr_size(t_rit_dyn_arr);                                         \
  for (size_t i = 0; i < rit_dyn_arr_size(t_rit_dyn_arr); i++) {               \
    rit_dyn_arr_set(t_rit_dyn_arr_other, i, rit_dyn_arr_at(t_rit_dyn_arr, i)); \
  }

/// @brief Get the pointer to the first element of an array
#define rit_dyn_arr_begin(t_rit_dyn_arr) (&(t_rit_dyn_arr[0]))
/// @brief Get the pointer to the past-the-end element of an array
#define rit_dyn_arr_end(t_rit_dyn_arr) \
  (&(t_rit_dyn_arr[rit_dyn_arr_size(t_rit_dyn_arr)]))

/// @brief Get the first element of an array
#define rit_dyn_arr_front(t_rit_dyn_arr) (t_rit_dyn_arr[0])
/// @brief Get the last element of an array
#define rit_dyn_arr_back(t_rit_dyn_arr) \
  (t_rit_dyn_arr[rit_dyn_arr_size(t_rit_dyn_arr) - 1])

#define rit_dyn_arr_push_back(t_rit_dyn_arr, t_value, t_allocator) \
  if (rit_dyn_arr_capacity(t_rit_dyn_arr) <=                       \
      rit_dyn_arr_size(t_rit_dyn_arr) + 1) {                       \
    rit_dyn_arr_reserve(t_rit_dyn_arr,                             \
                        (rit_dyn_arr_size(t_rit_dyn_arr) + 1) * 2, \
                        t_allocator);                              \
  }                                                                \
  rit_dyn_arr_get_metadata(t_rit_dyn_arr)->m_size++;               \
  rit_dyn_arr_set(t_rit_dyn_arr, rit_dyn_arr_size(t_rit_dyn_arr) - 1, t_value);

#define rit_dyn_arr_pop_back(t_rit_dyn_arr) \
  rit_dyn_arr_get_metadata(t_rit_dyn_arr)->m_size--

#define rit_dyn_arr_append(t_rit_dyn_arr, t_count, t_value, t_allocator) \
  for (int i = 1; i <= t_count; i++) {                                   \
    rit_dyn_arr_push_back(t_rit_dyn_arr, t_value, t_allocator);          \
  }

#define rit_dyn_arr_resize(t_rit_dyn_arr, t_count, t_value, t_allocator) \
  rit_dyn_arr_clear(t_rit_dyn_arr);                                      \
  rit_dyn_arr_append(t_rit_dyn_arr, t_count, t_value, t_allocator)

/// @brief Insert a value in the array before t_pos.
#define rit_dyn_arr_insert(t_rit_dyn_arr, t_pos, t_value, t_allocator)       \
  rit_dyn_arr_push_back(t_rit_dyn_arr, 0, t_allocator);                      \
  for (size_t i = rit_dyn_arr_size(t_rit_dyn_arr) - 1; i >= t_pos; i--) {    \
    rit_dyn_arr_set(t_rit_dyn_arr, i, rit_dyn_arr_at(t_rit_dyn_arr, i - 1)); \
    rit_dyn_arr_set(t_rit_dyn_arr, i - 1, 0);                                \
  }                                                                          \
  rit_dyn_arr_set(t_rit_dyn_arr, t_pos - 1, t_value)

/// @brief Remove the value in the array at t_pos.
#define rit_dyn_arr_erase(t_rit_dyn_arr, t_pos)                              \
  for (size_t i = t_pos + 1; i < rit_dyn_arr_size(t_rit_dyn_arr); i++) {     \
    rit_dyn_arr_set(t_rit_dyn_arr, i - 1, rit_dyn_arr_at(t_rit_dyn_arr, i)); \
  }                                                                          \
  rit_dyn_arr_pop_back(t_rit_dyn_arr)

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
