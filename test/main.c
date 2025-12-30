#include "a.h"
#include "b.h"

#define RIT_STR_IMPLEMENTATION
#include "../rit_str.h"

int main() {
  void *buf = malloc(100);
  test_alloc_a();
  test_alloc_b();
}
