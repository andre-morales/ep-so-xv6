#include "types.h"
#include "stat.h"
#include "user.h"

// teste simples da nova chamada de sistema
int main(int argc, char *argv[]) {
  int x1 = getreadcount();
  int x2 = getreadcount();
  char buf[100];
  (void) read(4, buf, 1);
  int x3 = getreadcount();
  int i;
  for (i = 0; i < 1000; i++) {
    (void) read(4, buf, 1);
  }
  int x4 = getreadcount();
  printf(1, "INITIAL VALUE %d\n", x1);
  printf(1, "XV6_TEST_OUTPUT %d %d %d\n", x2-x1, x3-x2, x4-x3);
  // saida esperada:
  // XV6_TEST_OUTPUT 0 1 1000
  exit();
}
