#include<stdio.h>
foo(void) { printf("OK\n"); return 0; }
foo_with_arguments(first, second) { 
    printf("%d, %d\n", first, second);
    return 0;
}
int sum(int n) {
  int sum = 0;
  for (int i = 1; i <= n; i++) {
    sum = sum + i;
  }
  printf("a result: %d -> a test for sum: OK.\n", sum);
  return 0;
}
