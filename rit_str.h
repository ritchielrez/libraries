// LICENSE
// See end of the file for license information.

#ifndef RSTR_INTERNAL_DEF
#define RSTR_INTERNAL_DEF static
#endif // RSTR_INTERNAL_DEF

#ifndef RIT_STR_H_INCLUDED
#define RIT_STR_H_INCLUDED

#define RSV_NULL (rsv){.m_size = 0, .m_str = ""}

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Disable MSVC warning 4702: unreachable code
#pragma warning(disable : 4702)

#define DEFAULT_STR_CAP 16

#ifdef RIT_DYN_ARR_H_INCLUDED
typedef rda_allocator rstr_allocator;
#else
/// @brief Custom allocator interface.
/// Functions allocating memory takes a custom allocator based off this
/// interface as a parameter.
typedef struct {
  void *(*alloc)(void *, size_t);
  void (*free)(void *, void *);
  void *(*realloc)(void *, void *, size_t, size_t);
  void *m_ctx; // The arena, stack or etc where the memory would be allocated,
               // NULL if none
} rstr_allocator;
#endif // RIT_DYN_ARR_H_INCLUDED

/// @brief Owning reference to a string
struct rstr {
  size_t m_size;     // Do not modify this, this is private
  size_t m_capacity; // Do not modify this, this is private
  char *m_data;      // Do not modify this, this is private
};

/// @brief Non owning reference to a string
typedef struct {
  size_t m_size;     // Do not modify this, this is private
  const char *m_str; // Do not modify this, this is private
} rsv;

static inline size_t rstr_size(struct rstr *t_rstr) { return t_rstr->m_size; }

static inline size_t rstr_capacity(struct rstr *t_rstr) {
  return t_rstr->m_capacity;
}

static inline size_t rsv_size(rsv t_rsv) { return t_rsv.m_size; }

/// @brief Create a rsv from a string literal.
/// Do not use this for strings that are stored somewhere in the stack of heap.
/// This important to understand because for length counting `sizeof()` is used
/// which only works for string literals. `strlen()` is not used because it
/// relies on `\0` which is unsafe.
#define rsv_lit(t_lit) ((rsv){.m_size = sizeof(t_lit) - 1, .m_str = (t_lit)})

/// @brief Create a rsv from c string
/// The length is going to be `t_len` if the null character was not found in the
/// first `t_len`bytes of `t_cstr`
static inline rsv rsv_cstr(char *t_cstr, size_t t_len) {
  size_t len = 0;
  for (; len < t_len && t_cstr[len] != '\0'; len++) {
  }
  return (rsv){.m_size = len, .m_str = t_cstr};
}
/// @brief Create a rsv from rstr
static inline rsv rsv_rstr(struct rstr *t_rstr) {
  return (rsv){.m_size = rstr_size(t_rstr), .m_str = t_rstr->m_data};
}
/// @brief Create a rsv from rsv
static inline rsv rsv_rsv(rsv t_rsv) {
  return (rsv){.m_size = rsv_size(t_rsv), .m_str = t_rsv.m_str};
}

#define rsv_copy(t_rsv, t_rsv_other)                                           \
  (t_rsv).m_str = (t_rstr_other)->m_str;                                       \
  (t_rsv).m_size = rsv_size(t_rstr_other)

/// @brief Access a possibly non null-terminated string from rsv
static inline const char *rsv_data(rsv t_rsv) { return t_rsv.m_str; }

/// @internal
RSTR_INTERNAL_DEF inline bool _rsv_index_bounds_check(const char *t_file,
                                                      int t_line, rsv t_rsv,
                                                      size_t t_index) {
  if (t_index < rsv_size(t_rsv))
    return true;
  fprintf(stderr,
          "Error: string_view index is out of bounds, file: %s, line: %d\n",
          t_file, t_line);
  exit(EXIT_FAILURE);
}

#define rsv_at(t_rsv, t_index)                                                 \
  (_rsv_index_bounds_check(__FILE__, __LINE__, (t_rsv), (t_index)))            \
      ? (t_rsv).m_str[t_index]                                                 \
      : (t_rsv).m_str[t_index]

/// @brief Get the pointer to the first element of a rsv
static inline const char *rsv_begin(rsv t_rsv) { return &(t_rsv.m_str[0]); }
/// @brief Get the pointer to the past-the-end element of an rsv
static inline const char *rsv_end(rsv t_rsv) {
  return &(t_rsv.m_str[rsv_size(t_rsv)]);
}

/// @brief Get the first element of an rsv
static inline char rsv_front(rsv t_rsv) { return t_rsv.m_str[0]; }
/// @brief Get the last element of an rsv
static inline char rsv_back(rsv t_rsv) {
  return t_rsv.m_str[rsv_size(t_rsv) - 1];
}

/// @internal
RSTR_INTERNAL_DEF inline void
_rstr_init_with_location(const char *t_file, int t_line, struct rstr *t_rstr,
                         size_t t_size, rstr_allocator *t_allocator) {
  size_t capacity = DEFAULT_STR_CAP < t_size * 2 ? t_size * 2 : DEFAULT_STR_CAP;
  t_rstr->m_data = (char *)t_allocator->alloc(t_allocator->m_ctx, capacity);
  if (!t_rstr->m_data) {
    fprintf(stderr, "Error: allocation failed, file: %s, line: %d\n", t_file,
            t_line);
    exit(EXIT_FAILURE);
  }
  t_rstr->m_size = t_size;
  t_rstr->m_capacity = capacity;
}

#define rstr_init(t_rstr, t_size, t_allocator)                                 \
  _rstr_init_with_location(__FILE__, __LINE__, (t_rstr), (t_size),             \
                           (t_allocator))

/// @internal
RSTR_INTERNAL_DEF inline void _rstr_realloc(const char *t_file, int t_line,
                                            struct rstr *t_rstr,
                                            size_t t_new_capacity,
                                            rstr_allocator *t_allocator) {
  if (t_new_capacity > rstr_capacity(t_rstr)) {
    t_rstr->m_data =
        (char *)t_allocator->realloc(t_allocator->m_ctx, t_rstr->m_data,
                                     rstr_capacity(t_rstr), t_new_capacity);
    if (!t_rstr->m_data) {
      fprintf(stderr, "Error: reallocation failed, file: %s, line: %d\n",
              t_file, t_line);
      exit(EXIT_FAILURE);
    }
    t_rstr->m_capacity = t_new_capacity;
  }
}

/// @brief Set the capacity of a string.
#define rstr_reserve(t_rstr, t_new_capacity, t_allocator)                      \
  _rstr_realloc(__FILE__, __LINE__, (t_rstr), (t_new_capacity), (t_allocator))

#define rstr_swap(t_rstr, t_rstr_other)                                        \
  do {                                                                         \
    struct rstr tmp = *(t_rstr);                                               \
    *(t_rstr) = *(t_rstr_other);                                               \
    *(t_rstr_other) = tmp;                                                     \
  } while (0)

/// @brief Returns a pointer to a null-terminated character array with data
/// equivalent to those stored in the string.
static inline const char *rstr_cstr(struct rstr *t_rstr) {
  if (t_rstr->m_data[rstr_size(t_rstr)] != '\0') {
    t_rstr->m_data[rstr_size(t_rstr)] = '\0';
  }
  return t_rstr->m_data;
}
/// @brief Returns a pointer to a possibly non null-terminated character array
/// with data equivalent to those stored in the string.
static inline const char *rstr_data(struct rstr *t_rstr) {
  return t_rstr->m_data;
}

static inline void rstr_free(struct rstr *t_rstr, rstr_allocator *t_allocator) {
  t_allocator->free(t_allocator->m_ctx, t_rstr->m_data);
}

/// @param Check if a string is empty.
#define rstr_empty(t_rstr) rstr_size(t_rstr) == 0

/// @brief Empty out a string.
static inline void rstr_clear(struct rstr *t_rstr) {
  t_rstr->m_size = 0;
  t_rstr->m_data[0] = '\0';
}

/// @brief Create a rstr.
#define rstr(t_rstr, t_rsv, t_allocator)                                       \
  struct rstr t_rstr = {};                                                     \
  rstr_init(&t_rstr, rsv_size(t_rsv), (t_allocator));                          \
  memcpy(t_rstr.m_data, (t_rsv).m_str, rsv_size(t_rsv));                       \
  t_rstr.m_data[rsv_size(t_rsv)] = '\0'

#define rstr_ret_ptr_at_index(t_rstr, t_index)                                 \
  (((t_index) >= rstr_size(t_rstr))                                            \
       ? (fprintf(stderr,                                                      \
                  "Error: array index out of bounds, file: %s, line: %d\n",    \
                  __FILE__, __LINE__),                                         \
          exit(EXIT_FAILURE), &((t_rstr)->m_data[t_index]))                    \
       : &((t_rstr)->m_data[t_index]))

#define rstr_at(t_rstr, t_index) (*(rstr_ret_ptr_at_index(t_rstr, t_index)))

/// @internal
RSTR_INTERNAL_DEF inline void
_rstr_cp_with_location(const char *t_file, int t_line, struct rstr *t_rstr,
                       struct rstr *t_rstr_other, size_t t_index, size_t t_size,
                       rstr_allocator *t_allocator) {
  if (t_index > rstr_size(t_rstr_other)) {
    fprintf(stderr,
            "Error: starting index of substring out of bounds of the string, "
            "file: %s, line: %d\n",
            t_file, t_line);
    exit(EXIT_FAILURE);
  } else if (t_index + t_size > rstr_size(t_rstr_other)) {
    fprintf(stderr,
            "Error: size of substring greater than the string, file: %s, line: "
            "%d\n",
            t_file, t_line);
    exit(EXIT_FAILURE);
  }
  _rstr_init_with_location(t_file, t_line, t_rstr, t_size - t_index,
                           t_allocator);
  for (size_t i = 0, j = t_index; i < t_size; i++, j++) {
    rstr_at(t_rstr, i) = rstr_at(t_rstr_other, j);
  }
}

/// @param t_rstr Where to copy
/// @param t_rstr_other What to copy
/// @param t_size The size of substring of t_rstr_other
#define rstr_cp(t_rstr, t_rstr_other, t_index, t_size, t_allocator)            \
  struct rstr t_rstr = {};                                                     \
  _rstr_cp_with_location(__FILE__, __LINE__, &t_rstr, &t_rstr_other,           \
                         (t_index), (t_size), (t_allocator))
/// @brief Get the pointer to the first element of an array
static inline const char *rstr_begin(struct rstr *t_rstr) {
  return (const char *)&(t_rstr->m_data[0]);
}
/// @brief Get the pointer to the past-the-end element of an array
static inline const char *rstr_end(struct rstr *t_rstr) {
  return (const char *)&(t_rstr->m_data[rstr_size(t_rstr)]);
}

/// @brief Get the first element of an array
static inline char rstr_front(struct rstr *t_rstr) { return t_rstr->m_data[0]; }
/// @brief Get the last element of an array
static inline char rstr_back(struct rstr *t_rstr) {
  return t_rstr->m_data[rstr_size(t_rstr) - 1];
}

static inline void rstr_push_back(struct rstr *t_rstr, char t_char,
                                  rstr_allocator *t_allocator) {
  if (rstr_capacity(t_rstr) <= rstr_size(t_rstr) + 1) {
    rstr_reserve(t_rstr, (rstr_size(t_rstr) + 1) * 2, t_allocator);
  }
  t_rstr->m_data[rstr_size(t_rstr)] = t_char;
  t_rstr->m_size++;
  t_rstr->m_data[rstr_size(t_rstr)] = '\0';
}

static inline void rstr_pop_back(struct rstr *t_rstr) {
  t_rstr->m_size--;
  t_rstr->m_data[rstr_size(t_rstr)] = '\0';
}

static inline void rstr_append_char(struct rstr *t_rstr, size_t t_size,
                                    char t_char, rstr_allocator *t_allocator) {
  for (size_t i = 1; i <= t_size; i++) {
    rstr_push_back(t_rstr, t_char, t_allocator);
  }
}

static inline void rstr_append_str(struct rstr *t_rstr, rsv t_rsv,
                                   rstr_allocator *t_allocator) {
  for (size_t i = 0; i < rsv_size(t_rsv); i++) {
    rstr_push_back(t_rstr, rsv_at(t_rsv, i), t_allocator);
  }
}

static inline void rstr_remove(struct rstr *t_rstr, size_t t_size) {
  for (size_t i = 1; i <= t_size; i++) {
    rstr_pop_back(t_rstr);
  }
}

static inline void rstr_resize(struct rstr *t_rstr, size_t t_size, char t_char,
                               rstr_allocator *t_allocator) {
  rstr_clear(t_rstr);
  rstr_append_char(t_rstr, t_size, t_char, t_allocator);
}

/// @brief Insert characters in the array at t_index.
static inline void rstr_insert(struct rstr *t_rstr, size_t t_index,
                               size_t t_size, char t_char,
                               rstr_allocator *t_allocator) {
  rstr_append_char(t_rstr, t_size, t_char, t_allocator);
  for (size_t i = rstr_size(t_rstr) - 1; i >= t_index + t_size; i--) {
    rstr_at(t_rstr, i) = rstr_at(t_rstr, i - t_size);
    rstr_at(t_rstr, i - t_size) = t_char;
  }
}

/// @brief Remove characters in the array at t_index.
static inline void rstr_erase(struct rstr *t_rstr, size_t t_index,
                              size_t t_size) {
  for (size_t i = t_index + t_size; i < rstr_size(t_rstr); i++) {
    rstr_at(t_rstr, i - t_size) = rstr_at(t_rstr, i);
  }
  rstr_remove(t_rstr, t_size);
}

/// @param t_rstr Where to assign
/// @param t_rsv What to assign
static inline void rstr_assign(struct rstr *t_rstr, rsv t_rsv,
                               rstr_allocator *t_allocator) {
  rstr_clear(t_rstr);
  rstr_append_str(t_rstr, t_rsv, t_allocator);
}

/// @internal
RSTR_INTERNAL_DEF inline void
_rstr_replace_with_location(const char *t_file, int t_line, struct rstr *t_rstr,
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
    rstr_at(t_rstr, i) = rsv_at(t_rsv, j);
  }
}

/// @param t_index Starting index of the substring
/// @param t_size Number of characters in the substring
#define rstr_replace(t_rstr, t_index, t_size, t_rsv, t_allocator)              \
  _rstr_replace_with_location(__FILE__, __LINE__, t_rstr, t_index, t_size,     \
                              t_rsv, t_allocator)

/// @brief Extracts characters from a input stream until \n is reached and
/// stores them in a rstr
static inline void rstr_getline(FILE *t_istream, struct rstr *t_rstr,
                                rstr_allocator *t_allocator) {
  for (int ch = fgetc(t_istream); ch != '\n'; ch = fgetc(t_istream)) {
    rstr_push_back(t_rstr, (char)ch, (t_allocator));
  }
}

/// @brief Extracts characters from a input stream until EOF is reached and
/// stores them in a rstr
static inline void rstr_getstream(FILE *t_istream, struct rstr *t_rstr,
                                  rstr_allocator *t_allocator) {
  for (int ch = fgetc(t_istream); ch != EOF; ch = fgetc(t_istream)) {
    rstr_push_back(t_rstr, (char)ch, (t_allocator));
  }
}

#endif // RIT_STR_H_INCLUDED

// Enable MSVC warning 4702: unreachable code
#pragma warning(disable : 4702)

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
