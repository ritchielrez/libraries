// LICENSE
// See end of the file for license information.
#ifndef RIT_STR_H_INCLUDED
#define RIT_STR_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_STR_CAP 16

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////DECLARATION///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/// @brief Custom allocator interface.
/// Functions allocating memory takes a custom allocator based off this
/// interface as a parameter.
typedef struct {
  void *(*alloc)(void *, size_t);
  void (*free)(void *, void *);
  void *(*realloc)(void *, void *, size_t, size_t);
  void *m_ctx;  // The arena, stack or etc where the memory would be allocated,
                // NULL if none
} rstr_allocator;

/// @brief Holds the metadata about the string.
/// This metadata struct is stored as header of the string.
typedef struct {
  size_t m_size;
  size_t m_capacity;
} rstr_metadata;

/// @brief Non owning reference to a string
typedef struct {
  size_t m_size;
  char *m_str;
} rsv;

/// @brief Get the pointer to the metadata header struct of a string.
/// @param t_rstr The string
#define rstr_get_metadata(t_rstr) (&((rstr_metadata *)t_rstr)[-1])

inline size_t rstr_size(char *t_rstr) {
  return rstr_get_metadata(t_rstr)->m_size;
}

inline size_t rstr_capacity(char *t_rstr) {
  return rstr_get_metadata(t_rstr)->m_capacity;
}

#define rsv_size(t_rsv) t_rsv.m_size

/// @brief Create a rsv from c string
rsv rsv_lit(char *t_cstr) {
  return (rsv){.m_size = strlen(t_cstr), .m_str = t_cstr};
}
/// @brief Create a rsv from rstr
rsv rsv_rstr(char *t_rstr) {
  return (rsv){.m_size = rstr_size(t_rstr), .m_str = t_rstr};
}
/// @brief Create a rsv from rsv
rsv rsv_rsv(rsv t_rsv) {
  return (rsv){.m_size = rsv_size(t_rsv), .m_str = t_rsv.m_str};
}

#define rsv_assign(t_rsv, t_rsv_other) \
  t_rsv.m_str = t_rstr_other.m_str;    \
  t_rsv.m_size = rsv_size(t_rstr_other)

/// @brief Access the string from rsv
const char *rsv_get(rsv t_rsv) { return t_rsv.m_str; }

/// @internal
inline bool rsv_index_bounds_check(const char *t_file, int t_line, rsv t_rsv,
                                   size_t t_index) {
  if (t_index < rsv_size((t_rsv))) return true;
  fprintf(stderr,
          "Error: string_view index is out of bounds, file: %s, line: %d\n",
          t_file, t_line);
  exit(EXIT_FAILURE);
}

#define rsv_at(t_rsv, t_index)                                 \
  (rsv_index_bounds_check(__FILE__, __LINE__, t_rsv, t_index)) \
      ? t_rsv.m_str[t_index]                                   \
      : t_rsv.m_str[t_index]

/// @brief Get the pointer to the first element of a rsv
#define rsv_begin(t_rsv) (&(t_rsv.m_str[0]))
/// @brief Get the pointer to the past-the-end element of an rsv
#define rsv_end(t_rsv) (&(t_rsv.m_str[rsv_size(t_rsv)]))

/// @brief Get the first element of an rsv
#define rsv_front(t_rsv) (t_rsv.m_str[0])
/// @brief Get the last element of an rsv
#define rsv_back(t_rsv) (t_rsv.m_str[rsv_size(t_rsv) - 1])

#define rstr_alloc(t_size, t_allocator) \
  rstr_alloc_with_location(__FILE__, __LINE__, t_size, t_allocator)

/// @internal
char *rstr_alloc_with_location(const char *t_file, int t_line, size_t t_size,
                               rstr_allocator *t_allocator);

/// @brief Set the capacity of a string.
#define rstr_reserve(t_rstr, t_new_capacity, t_allocator) \
  rstr_realloc(__FILE__, __LINE__, &t_rstr, t_new_capacity, t_allocator)

/// @internal
void rstr_realloc(const char *t_file, int t_line, char **t_rstr,
                  size_t t_new_capacity, rstr_allocator *t_allocator);

#define rstr_swap(t_rstr, t_rstr_other) \
  do {                                  \
    void *tmp = t_rstr;                 \
    t_rstr = t_rstr_other;              \
    t_rstr_other = tmp;                 \
  } while (0)

/// @brief Makes a non-binding request to make the capacity of a string equal to
/// its size. In this library this is definied as a no-op function.
inline void rstr_shrink_to_fit(char *t_rstr) { (void)t_rstr; }

/// @brief Returns a pointer to a null-terminated character array with data
/// equivalent to those stored in the string.
inline const char *rstr_cstr(char *t_rstr) { return t_rstr; }

/// @brief Returns a pointer to a null-terminated character array with data
/// equivalent to those stored in the string.
inline const char *rstr_data(char *t_rstr) { return rstr_cstr(t_rstr); }

inline void rstr_free(char *t_rstr, rstr_allocator *t_allocator) {
  t_allocator->free(t_allocator->m_ctx, t_rstr);
}

/// @internal
inline bool rstr_index_bounds_check(const char *t_file, int t_line,
                                    char *t_rstr, size_t t_index) {
  if (t_index < rstr_size(t_rstr)) return true;
  fprintf(stderr, "Error: string index is out of bounds, file: %s, line: %d\n",
          t_file, t_line);
  exit(EXIT_FAILURE);
}

/// @param Check if a string is empty.
inline bool rstr_empty(char *t_rstr) { return rstr_size(t_rstr) == 0; }

/// @brief Empty out a string.
inline void rstr_clear(char *t_rstr) {
  rstr_get_metadata(t_rstr)->m_size = 0;
  t_rstr[0] = '\0';
}

/// @brief Create a rstr.
#define rstr(t_rstr, t_rsv, t_allocator)                                       \
  char *t_rstr = rstr_alloc_with_location(__FILE__, __LINE__, rsv_size(t_rsv), \
                                          t_allocator);                        \
  strcpy(t_rstr, t_rsv.m_str)

/// @param t_rstr Where to copy
/// @param t_rstr_other What to copy
/// @param t_size The size of substring of t_rstr_other
#define rstr_cp(t_rstr, t_index, t_size, t_rstr_other, t_allocator)      \
  char *t_rstr = rstr_cp_with_location(__FILE__, __LINE__, t_rstr_other, \
                                       t_index, t_size, t_allocator)

///@internal
char *rstr_cp_with_location(const char *t_file, int t_line, char *t_rstr,
                            size_t t_index, size_t t_size,
                            rstr_allocator *t_allocator);

#define rstr_at(t_rstr, t_index)                                 \
  (rstr_index_bounds_check(__FILE__, __LINE__, t_rstr, t_index)) \
      ? t_rstr[t_index]                                          \
      : t_rstr[t_index]

#define rstr_set(t_rstr, t_index, t_char)                             \
  if (rstr_index_bounds_check(__FILE__, __LINE__, t_rstr, t_index)) { \
    t_rstr[t_index] = t_char;                                         \
  }

/// @brief Get the pointer to the first element of an array
#define rstr_begin(t_rstr) (&(t_rstr[0]))
/// @brief Get the pointer to the past-the-end element of an array
#define rstr_end(t_rstr) (&(t_rstr[rstr_size(t_rstr)]))

/// @brief Get the first element of an array
#define rstr_front(t_rstr) (t_rstr[0])
/// @brief Get the last element of an array
#define rstr_back(t_rstr) (t_rstr[rstr_size(t_rstr) - 1])

#define rstr_push_back(t_rstr, t_char, t_allocator)                 \
  t_rstr[rstr_size(t_rstr)] = t_char;                               \
  if (rstr_capacity(t_rstr) <= rstr_size(t_rstr) + 1) {             \
    rstr_reserve(t_rstr, (rstr_size(t_rstr) + 1) * 2, t_allocator); \
  }                                                                 \
  rstr_get_metadata(t_rstr)->m_size++;                              \
  t_rstr[rstr_size(t_rstr)] = '\0'

#define rstr_pop_back(t_rstr)          \
  rstr_get_metadata(t_rstr)->m_size--; \
  t_rstr[rstr_size(t_rstr)] = '\0'

#define rstr_append_char(t_rstr, t_size, t_char, t_allocator) \
  for (size_t i = 1; i <= t_size; i++) {                      \
    rstr_push_back(t_rstr, t_char, t_allocator);              \
  }

#define rstr_append_str(t_rstr, t_rsv, t_allocator)     \
  for (size_t i = 0; i < rsv_size(t_rsv); i++) {        \
    rstr_push_back(t_rstr, rsv_at(t_rsv, i), t_allocator); \
  }

/// @brief Remove elements from the end of the string
#define rstr_remove(t_rstr, t_size)      \
  for (size_t i = 1; i <= t_size; i++) { \
    rstr_pop_back(t_rstr);               \
  }

/// @brief Changes the number of characters stored
#define rstr_resize(t_rstr, t_size, t_char, t_allocator) \
  rstr_clear(t_rstr);                                    \
  rstr_append_char(t_rstr, t_size, t_char, t_allocator)

/// @brief Insert characters in the array at t_index.
#define rstr_insert(t_rstr, t_index, t_size, t_char, t_allocator)        \
  do {                                                                   \
    rstr_append_char(t_rstr, t_size, t_char, t_allocator);               \
    for (size_t i = rstr_size(t_rstr) - 1; i >= t_index + t_size; i--) { \
      rstr_set(t_rstr, i, rstr_at(t_rstr, i - t_size));                  \
      rstr_set(t_rstr, i - t_size, t_char);                              \
    }                                                                    \
  } while (0)

/// @brief Remove characters in the array at t_index.
#define rstr_erase(t_rstr, t_index, t_size)                         \
  do {                                                              \
    for (size_t i = t_index + t_size; i < rstr_size(t_rstr); i++) { \
      rstr_set(t_rstr, i - t_size, rstr_at(t_rstr, i));             \
    }                                                               \
    rstr_remove(t_rstr, t_size);                                    \
  } while (0)

/// @param t_rstr Where to assign
/// @param t_rsv What to assign
inline void rstr_assign(char *t_rstr, rsv t_rsv,
                        rstr_allocator *t_allocator) {
  rstr_clear(t_rstr);
  rstr_append_str(t_rstr, t_rsv, t_allocator);
}

/// @param t_index Starting index of the substring
/// @param t_size Number of characters in the substring
#define rstr_replace(t_rstr, t_index, t_size, t_rsv, t_allocator)         \
  rstr_replace_with_location(__FILE__, __LINE__, t_rstr, t_index, t_size, \
                             t_rsv, t_allocator)

/// @internal
void rstr_replace_with_location(const char *t_file, int t_line, char *t_rstr,
                                size_t t_index, size_t t_size, rsv t_rsv,
                                rstr_allocator *t_allocator);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////IMPLEMENTATION//////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef RIT_STR_IMPLEMENTATION

char *rstr_alloc_with_location(const char *t_file, int t_line, size_t t_size,
                               rstr_allocator *t_allocator) {
  size_t capacity = DEFAULT_STR_CAP < t_size * 2 ? t_size * 2 : DEFAULT_STR_CAP;
  rstr_metadata *arr = (rstr_metadata *)t_allocator->alloc(
      t_allocator->m_ctx, sizeof(rstr_metadata) + capacity);
  if (!arr) {
    fprintf(stderr, "Error: allocation failed, file: %s, line: %d\n", t_file,
            t_line);
    exit(EXIT_FAILURE);
  }
  arr->m_size = t_size;
  arr->m_capacity = capacity;
  arr += 1;
  return (char *)arr;
}

void rstr_realloc(const char *t_file, int t_line, char **t_rstr,
                  size_t t_new_capacity, rstr_allocator *t_allocator) {
  if (t_new_capacity > rstr_capacity(*t_rstr)) {
    rstr_metadata *arr = (rstr_metadata *)t_allocator->realloc(
        t_allocator->m_ctx, rstr_get_metadata(*t_rstr),
        sizeof(rstr_metadata) + rstr_capacity(*t_rstr),
        sizeof(rstr_metadata) + t_new_capacity);
    if (!arr) {
      fprintf(stderr, "Error: reallocation failed, file: %s, line: %d\n",
              t_file, t_line);
      exit(EXIT_FAILURE);
    }
    arr += 1;
    *t_rstr = (char *)arr;
    rstr_get_metadata(*t_rstr)->m_capacity = t_new_capacity;
  }
}

char* rstr_cp_with_location(const char *t_file, int t_line, char *t_rstr,
                           size_t t_index, size_t t_size,
                           rstr_allocator *t_allocator) {
  if (t_index > rstr_size(t_rstr)) {
    fprintf(stderr,
            "Error: starting index of substring out of bounds of the string, "
            "file: %s, line: %d\n",
            t_file, t_line);
    exit(EXIT_FAILURE);
  } else if (t_index + t_size > rstr_size(t_rstr)) {
    fprintf(stderr,
            "Error: size of substring greater than the string, file: %s, line: "
            "%d\n",
            t_file, t_line);
    exit(EXIT_FAILURE);
  }
  char *ret_rstr = rstr_alloc_with_location(t_file, t_line, t_size - t_index, t_allocator);
  for (size_t i = 0, j = t_index; i < t_size; i++, j++) {
    rstr_set(ret_rstr, i, rstr_at(t_rstr, j));
  }
  return ret_rstr;
}

void rstr_replace_with_location(const char *t_file, int t_line, char *t_rstr,
                                size_t t_index, size_t t_size, rsv t_rsv,
                                rstr_allocator *t_allocator) {
  if (t_index > rstr_size(t_rstr)) {
    fprintf(stderr,
            "Error: starting index of substring out of bounds of the string, "
            "file: %s, line: %d\n",
            t_file, t_line);
    exit(EXIT_FAILURE);
  } else if (t_size == 0) {
    fprintf(stderr,
            "Error: size of substring cannot be 0, file: %s, line: %d\n",
            t_file, t_line);
    exit(EXIT_FAILURE);
  } else if (t_index + t_size > rstr_size(t_rstr)) {
    fprintf(stderr,
            "Error: size of substring greater than the string, file: %s, line: "
            "%d\n",
            t_file, t_line);
    exit(EXIT_FAILURE);
  }
  if (t_size < rsv_size(t_rsv)) {
    size_t count = rsv_size(t_rsv) - t_size;
    rstr_insert(t_rstr, t_index, count, ' ', t_allocator);
  } else if (t_size > rsv_size(t_rsv)) {
    size_t count = t_size - rsv_size(t_rsv);
    rstr_erase(t_rstr, t_index, count);
  }
  for (size_t i = t_index, j = 0; j < rsv_size(t_rsv); i++, j++) {
    rstr_set(t_rstr, i, rsv_at(t_rsv, j));
  }
}

#endif  // RIT_STR_IMPLEMENTATION
#endif  // RIT_STR_H_INCLUDED
/*
The MIT License (MIT)

Copyright 2024 Ritchiel Reza

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
