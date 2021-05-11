#include<stdio.h>
foo(void) { printf("OK\n"); return 0; }
foo_with_arguments(first, second) { 
    printf("%d, %d\n", first, second);
    return 0;
}
foo_with_6_arguments(first, second, third, fourth, fifth, sixth) { 
    printf("%d, %d, %d, %d, %d, %d\n", first, second, third, fourth, fifth, sixth);
    return 0;
}
sum(n) {
  int res = 0;
  for (int i = 1; i <= n; i++) {
    res = res + i;
  }
  printf("a result: %d -> a test for sum: OK.\n", res);
  return 0;
}
