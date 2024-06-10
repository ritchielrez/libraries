#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0

#define nullptr 0

#define DEFAULT_CHUNK_MAX_COUNT 8192

typedef struct Buffer Buffer;
typedef struct Arena Arena;

typedef struct {
  const char *name;
  unsigned int age;
} Player;

// Macros related to arena allocator
#define arena_alloc_arr(arena, type, count)                                    \
  ((type *)arena_alloc(arena, sizeof(type) * count))
#define arena_alloc_struct(arena, type)                                        \
  ((type *)arena_alloc(arena, sizeof(type)))

// Arena allocator

/// @brief Holds data as uintptr(usually 8 byte) chunks.
///
/// Buffers are just like nodes in a linked list
struct Buffer {
  Buffer *m_next;
  size_t m_chunk_max_count;
  size_t m_chunk_current_count;
  /// holds the actual memory chunks, where user data is stored
  uintptr_t m_data[];
};

/// @brief Creates a new buffer, where chunks of bytes are allocated
/// @param t_chunk_count Maximum number of chunks the buffer can hold
/// @return Buffer*
Buffer *buffer_new(size_t t_chunk_count) {
  size_t size_in_bytes = sizeof(Buffer) + sizeof(uintptr_t) * t_chunk_count;
  Buffer *new_buffer = malloc(size_in_bytes);

  new_buffer->m_next = nullptr;
  new_buffer->m_chunk_max_count = t_chunk_count;
  new_buffer->m_chunk_current_count = 0;

  return new_buffer;
}

/// @brief Frees up a buffer
/// @param t_buffer The buffer to be freed
/// @return void
void buffer_free(Buffer *t_buffer) { free(t_buffer); }

/// @brief Arena is just a growing list of buffers.
///
/// An arena usually looks like this:
/// m_begin -> next buffer -> active buffer
struct Arena {
  /// contains the starting buffer
  Buffer *m_begin;
  /// contains the active buffer
  Buffer *m_active;
};

/// @brief Allocate some data inside an arena.
///
/// The allocated data are stored in a buffer.
/// If the data is too big, a new buffer will be created.
///
/// @param t_arena The arena where data gets allocated
/// @param t_size_in_bytes The requested number of bytes to be allocated
/// @return void*
void *arena_alloc(Arena *t_arena, size_t t_size_in_bytes) {
  if (t_arena == nullptr) {
    fprintf(stderr, "Error, no valid arena was provided\n");
    exit(EXIT_FAILURE);
  }

  // To understand the following code, you need to have proper knowledge about
  // memory alignment. Align the requsted size to 8 bytes
  t_size_in_bytes = t_size_in_bytes + (sizeof(uintptr_t) - 1);
  size_t chunk_count = t_size_in_bytes / sizeof(uintptr_t);

  if (t_arena->m_active == nullptr) {
    // If there is no active buffer in an t_arena, there also should not be a
    // starting buffer
    assert(t_arena->m_begin == nullptr);
    size_t chunk_max_count = DEFAULT_CHUNK_MAX_COUNT;
    if (chunk_max_count < chunk_count)
      chunk_max_count = chunk_count;
    t_arena->m_active = buffer_new(chunk_max_count);
    t_arena->m_begin = t_arena->m_active;
  }

  if (t_arena->m_active->m_chunk_current_count + chunk_count >
      t_arena->m_active->m_chunk_max_count) {
    size_t chunk_max_count = DEFAULT_CHUNK_MAX_COUNT;
    if (chunk_max_count < chunk_count)
      chunk_max_count = chunk_count;
    t_arena->m_active->m_next = buffer_new(chunk_max_count);
    t_arena->m_active = t_arena->m_active->m_next;
  }

  void *result =
      &(t_arena->m_active->m_data[t_arena->m_active->m_chunk_current_count]);
  t_arena->m_active->m_chunk_current_count += chunk_count;
  return result;
}

/// @brief Resize some old data insdie an arena
///
/// The allocated data are stored in a buffer.
/// If the data is too big, a new buffer will be created.
///
/// @param t_arena The arena where data gets allocated
/// @param t_old_ptr The old ptr where the data is held
/// @param t_old_size_in_bytes The size of the old pointer
/// @param t_new_size_in_bytes The size of the new pointer
/// @return void*
void *arena_realloc(Arena *t_arena, void *t_old_ptr, size_t t_old_size_in_bytes,
                    size_t t_new_size_in_bytes) {
  if (t_arena == nullptr) {
    fprintf(stderr, "Error, no valid arena was provided\n");
    exit(EXIT_FAILURE);
  }

  t_old_size_in_bytes = t_old_size_in_bytes + (sizeof(uintptr_t) - 1);
  size_t old_chunk_count = t_old_size_in_bytes / sizeof(uintptr_t);
  t_new_size_in_bytes = t_new_size_in_bytes + (sizeof(uintptr_t) - 1);
  size_t new_chunk_count = t_new_size_in_bytes / sizeof(uintptr_t);
  if (old_chunk_count >= new_chunk_count) {
    return t_old_ptr;
  }

  void *result = arena_alloc(t_arena, t_new_size_in_bytes);
  char *old_ptr_bytes = t_old_ptr;
  char *new_ptr_bytes = result;

  for (size_t i = 0; i < t_old_size_in_bytes; ++i) {
    new_ptr_bytes[i] = old_ptr_bytes[i];
  }

  return result;
}

/// @brief Resets the allocated chunk count of an arena
/// @param t_arena The arena that will be resetted
/// @return void
void arena_reset(Arena *t_arena) {
  if (t_arena == nullptr) {
    fprintf(stderr, "Error, no valid arena was provided\n");
    exit(EXIT_FAILURE);
  }

  Buffer *current_buffer = t_arena->m_begin;
  while (current_buffer != nullptr) {
    current_buffer->m_chunk_current_count = 0;
    current_buffer = current_buffer->m_next;
  }
}

/// @brief Frees up an arena
/// @param t_arena The arena that will be freed
/// @return void
void arena_free(Arena *t_arena) {
  if (t_arena == nullptr) {
    fprintf(stderr, "Error, no valid arena was provided\n");
    exit(EXIT_FAILURE);
  }

  Buffer *current_buffer = t_arena->m_begin;
  while (current_buffer->m_next != nullptr) {
    Buffer *next_buffer = current_buffer->m_next;
    current_buffer->m_chunk_max_count = 0;
    current_buffer->m_chunk_current_count = 0;
    free(current_buffer);
    current_buffer = next_buffer;
  }

  // Assigning null value to freed pointers is a good practice
  // because this ensures that by accessing any freed pointers
  // does not cause undefined behaviours, even though accessing
  // null values do cause them too, it is more easily debuggable.
  t_arena->m_begin = nullptr;
  t_arena->m_active = nullptr;
}

typedef struct {
  void *(*alloc)(void *, size_t);
  void (*free)(void *, void *);
} allocator;

void *allocator_arena_alloc(void *t_arena, size_t t_size_in_bytes) {
  return arena_alloc((Arena *)t_arena, t_size_in_bytes);
}

void allocator_arena_free(void *t_ptr, void *t_arena) {
  (void)t_ptr;
  arena_free(t_arena);
}

void *allocator_libc_malloc(void *t_ptr, size_t t_size_in_bytes) {
  (void)t_ptr;
  return malloc(t_size_in_bytes);
}

void allocator_libc_free(void *t_ptr, void *t_buffer) {
  (void)t_ptr;
  free(t_buffer);
}

static allocator libc_allocator = {allocator_libc_malloc, allocator_libc_free};
static allocator arena_allocator = {allocator_arena_alloc,
                                    allocator_arena_free};

int main() {
  Arena default_arena = {nullptr, nullptr};

  char *str = (char *)libc_allocator.alloc(&default_arena, 256);
  strcpy(str, "Hello World!");
  printf("%s\n", str);
  libc_allocator.free(nullptr, str);

  // Even though we are accessing memory completely out of the bounds
  // of the region, where the string has been allocated, the statement
  // below yet does not cause any error. this is not still recommmended
  // to do though, because this is actually an undefined behaviour :)
  // str[70000] = nullptr;

  char *name =
      (char *)arena_allocator.alloc(&default_arena, strlen("Rakhiel Reza"));
  strcpy(name, "Rakhiel Reza");
  printf("%s, %p\n", name, name);

  int *arr = arena_alloc_arr(&default_arena, int, 100);
  for (int i = 0; i < 100; ++i) {
    arr[i] = i + 1;
  }

  Player *player1 = arena_alloc_struct(&default_arena, Player);
  player1->name = "Mustafizur Rahman";
  player1->age = 28;
  printf("\nPlayer name: %s\n", player1->name);
  printf("Player age: %u\n", player1->age);

  Player *players[3];
  for (int i = 0; i < 3; ++i) {
    players[i] = arena_alloc_struct(&default_arena, Player);
  }
  players[0]->name = "Mushfiqur Rahim";
  players[0]->age = 36;
  players[1]->name = "Mahmudullah Riyad";
  players[1]->age = 39;
  players[2]->name = "Dinesh Karthik";
  players[2]->age = 38;
  for (int i = 0; i < 3; ++i) {
    printf("Player name: %s\n", players[i]->name);
    printf("Player age: %u\n", players[i]->age);
  }
  printf("\n");

  name = (char *)arena_realloc(&default_arena, name, strlen("Rakhiel Reza"),
                               strlen("Ritchiel Reza"));
  strcpy(name, "Ritchiel Reza");
  printf("%s, %p\n", name, name);

  arena_allocator.free(nullptr, &default_arena);

  return 0;
}
