#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define ARENA_ALLOCATOR_IMPLEMENTATION
#include "../arena_allocator.h"

#define nullptr (void *)0

typedef struct {
  const char *name;
  unsigned int age;
} Player;

int main() {
  Arena default_arena = {nullptr, nullptr};

  char *name = (char *)arena_alloc(&default_arena, strlen("Rosewood"));
  strcpy(name, "Rosewood");
  printf("%s, %p\n", name, name);

  int *arr = arena_alloc_arr(&default_arena, int, 100);
  for (int i = 0; i < 100; ++i) {
    arr[i] = i + 1;
  }

  Player *player1 = arena_alloc_struct(&default_arena, Player);
  player1->name = "John Doe";
  player1->age = 28;
  printf("\nPlayer name: %s\n", player1->name);
  printf("Player age: %u\n", player1->age);

  Player *players[3];
  for (int i = 0; i < 3; ++i) {
    players[i] = arena_alloc_struct(&default_arena, Player);
  }
  players[0]->name = "Flora";
  players[0]->age = 36;
  players[1]->name = "Jane Doe";
  players[1]->age = 39;
  players[2]->name = "Tunebridge Wells";
  players[2]->age = 38;
  for (int i = 0; i < 3; ++i) {
    printf("Player name: %s\n", players[i]->name);
    printf("Player age: %u\n", players[i]->age);
  }
  printf("\n");

  name = (char *)arena_realloc(&default_arena, name, strlen("Rosewood"),
                               strlen("Lord Buckethead"));
  strcpy(name, "Lord Buckethead");
  printf("%s, %p\n", name, name);

  arena_free(&default_arena);

  return 0;
}
