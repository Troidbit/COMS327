#include <stdio.h>
#include <string.h>

typedef old_type new_typename 

typedef struct foo {
  char c;
  int i;
  float f;
  char a[80];
} foo_t;

typedef int somethingelse;


void swap_wrong(int a, int b)
{
  int tmp;

  tmp = a;
  a = b;
  b = tmp;

  printf("%d, %d\n", a, b);
}

void swap_right(int *, int *);

void populate_foo(foo_t *f)
{
  f->i = 20;
  f->f = 6.02e23;
  strcpy(f->a, "changed!");
}

int main(int argc, char *argv[])
{
  int x, y;
  struct foo f;

  f.i = 10;
  f.f = 3.14;
  strcpy(f.a, "string");

  printf("%d %f %s\n", f.i, f.f, f.a);

  populate_foo(&f);
  printf("%d %f %s\n", f.i, f.f, f.a);

  return 0;

  x = 0;
  y = 1;

  printf("%d, %d\n", x, y);
  swap_right(&x, &y);
  printf("%d, %d\n", x, y);

  return 0;
}

void swap_right(int *a, int *b)
{
  int tmp;

  tmp = *a;
  *a = *b;
  *b = tmp;

  printf("%d, %d\n", *a, *b);
}

