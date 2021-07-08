#include <stdio.h>
#include <stdlib.h>
int foo() {
  printf("OK\n");
  return 0;
}
int foo_with_arguments(int first, int second) {
  printf("%d, %d\n", first, second);
  return 0;
}
int foo_with_6_arguments(int first, int second, int third, int fourth,
                         int fifth, int sixth) {
  printf("%d, %d, %d, %d, %d, %d\n", first, second, third, fourth, fifth,
         sixth);
  return 0;
}
int sum(int n) {
  int res = 0;
  for (int i = 1; i <= n; i++) {
    res = res + i;
  }
  printf("a result: %d -> a test for sum: OK.\n", res);
  return 0;
}
int *alloc(int **buf, int a, int b, int c) {
  *buf = (int *)malloc(sizeof(int) * 3);
  (*buf)[0] = a;
  (*buf)[1] = b;
  (*buf)[2] = c;
  return 0;
}
int print_num(int n) {
  printf("%d -> a test for pointer addition and subtraction: OK.\n", n);
  return 0;
}
