#include <stdio.h>

int main(int argc, char *argv[])
{
  int i;

  if (argc > 1) {
    printf("Hello");
    for (i = 1; i < argc; i++) {
      printf(" %s", argv[i]);
    }
    printf("!\n");
  } else {
    printf("Hello World!\n");
  }

  return 0;
}
