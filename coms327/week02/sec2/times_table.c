#include <stdio.h>

#define DIM 12

void fill_times_table(int table[DIM][DIM])
{
  int i, j;

  for (j = 0; j < DIM; j++) {
    for (i = 0; i < DIM; i++) {
      table[j][i] = (i + 1) * (j + 1);
    }
  }
}

unsigned fib(unsigned i)
{
  if (i < 2) {
    return i;
  }

  return fib(i - 1) + fib(i - 2);
}

int main(int argc, char *argv[])
{
  int table[DIM][DIM];
  int i, j;

  fill_times_table(table);

  for (j = 0; j < DIM; j++) {
    for (i = 0; i < DIM; i++) {
      printf("%3d ", table[j][i]);
    }
    printf("\n");
  }

  for (i = 0; i < 50; i++) {
    printf("%d: %d\n", i, fib(i));
  }

  return 0;
}
