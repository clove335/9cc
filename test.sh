#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
 int ret() { return 3; }
 int ret5() { return 5; }
EOF

assert() {
  expected="$1"
  input="$2"

  ./9cc "int main() { $input }" > tmp.s
  if [ "$?" != 0 ]; then
    echo "9cc error : $input"
    exit 1
  fi
  gcc -static -o tmp tmp.s tmp2.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "compile error : $input"
    nl -w2 tmp.s
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

test_program() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  if [ "$?" != 0 ]; then
    echo "9cc error : $input"
    exit 1
  fi
  gcc -static -o tmp tmp.s tmp2.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "compile error : $input"
    nl -w2 tmp.s
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

test_function_call() {
  function=$1
  stub=$2
  output=$3
  
  ./9cc "int main() { $function }" > out.s
  if [ $? -ne 0 ]; then
    echo failed to generate assembly from "$function"
    rm out.s
    exit 1
  fi

  gcc -c out.s -o out.o
  if [ $? -ne 0 ]; then
    echo failed to generate .o file from "$function"
    cat out.s
    rm out.s
    exit 1
  fi

  gcc -c "$stub" -o stub.o
  if [ $? -ne 0 ]; then
    echo failed to compile "$stub"
    rm out.s out.o
    exit 1
  fi

  gcc out.o stub.o -o out
  if [ $? -ne 0 ]; then
    echo failed to link test stub and obj file generated from "$stub"
    cat out.s > error.s
    cat out.s
    rm out.s out.o stub.o
    exit 1
  fi

  ./out > stdout.txt
  ret=$?

  if [ $ret -ne 0 ]; then
    cat out.s
    rm out.s out.o stub.o out stdout.txt
    exit 1
  fi

  echo "$output" | diff - stdout.txt > /dev/null 
  if [ $? -ne 0 ]; then
    echo expect stdout to be \""$output"\", but got \""$(cat stdout.txt)"\".
    cat out.s > error.s
    rm out.s out.o stub.o out stdout.txt
    exit 1
  else
    echo "$function => $output"
  fi

  rm out.s out.o stub.o out stdout.txt
}

test_error() {
  input="$1"
  output="$2"
  ./9cc "$input" 2> tests/stderr.txt && "Compilation of \"$input\" unexpectedly succeeded."
  echo "$output" | diff - tests/stderr.txt || "Failed. Error message of \"$input\" should be \"$output\"."
}

assert  0 "0;"
assert 42 "42;"
assert 6 '2+4;'
assert 5 '9-4;'
assert 9 '3*3;'
assert 21 '5+20-4;'
assert 41 " 12 + 34 - 5;" 
assert 47 "5+6*7;"
assert  5 "15/3;"
assert 77 "(5+6)*7;"
assert  3 "15/(3+2);"
assert  3 "int a; a = 3; a;"
assert  6 "int a; a = 3; a = a+3;"
assert 22 "int b; b = 5 * 6 - 8;"
assert 12 "6*4-3*4;"
assert  6 "int c; c = 2; c*3;"
assert 12 "int a; int z; a=z=3*2;a+z;"

assert  5 "return 5;"
assert  8 "int c; c=5; int b; b=3; c+b;"
assert  2 "int c; c = 2;
        int b; b = 1;
        return c;"
assert 14 "int a; a = 3; int b; b = 5 * 6 - 8; return a + b / 2;"
assert 68 "int a; a = 12; int x; x = (3+5)*7; int b; b = a+x; return b;"
assert 45 "int foo; foo = 10; int bar; bar = 35; return foo + bar;"
assert  4 "return +4;"
assert  1 "return -(6+1)+8;"
assert  5 "int n; n=-10+15; return n;"
assert  9 "int num; num = -3*-3; return num;"

assert  0 "4==5;"
assert  1 "5==5;"
assert  0 "5 != 5;"
assert  1 "1 != 5;"
assert  1 "4 <= 5;"
assert  1 "5 <= 5;"
assert  0 "6 <= 5;"
assert  1 "4 < 5;"
assert  0 "5 < 5;"
assert  0 "6 < 5;"
assert  1 "6 > 5;"
assert  0 "5 > 5;"
assert  0 "4 > 5;"
assert  0 "6 <= 5;"
assert  1 "5 >= 5;"
assert  1 "6 >= 5;"
assert  0 "4 >= 5;"

assert  5 "if (0) return 2; return 5;"
assert  5 "if (1-1) return 2; return 5;"
assert  2 "if (1) return 2; return 5;"
assert  2 "if (2) return 2; return 5;"
assert  15 "int a; a = 15; if (a) return a; return 5;"
assert  1 "int x; x = 1; if (3*4 > 10) return x; return 0;"
assert  0 "int x; x = 1; if (3*4 < 10) return x; return 0;"
assert  2 "int x; x = 5; if (x == 5) x = 2; else x = 15; return x;"
assert  15 "int x; x = 5; if (x == 1) x = 2; else x = 15; return x;"
assert  15 "int x; x = 5; if (x == 1) x = 2; else if (x == 5) x = 15; return x;"
assert  5 "int x; x = 5; if (x == 1) x = 2; else if (x == 4) x = 15; else x; return x;"

assert  7 "int x; x = 0; while (x < 7) x = x + 1; return x;"
assert  0 "int x; x = 10; while (x > 0) x = x - 1; return x;"
assert  10 "int x; x = 0; int i; for (i = 0; i<10; i=i+1) x = x + 1; return x;"
assert  55 "int x; x = 0; int i; i = 0; for (; i<=10; i=i+1) x = x + i; return x;"
assert  10 "int x; x = 0; for (;;) return 10; return x;"

assert  3 "{1; {2;} return 3;}"
assert  10 "int x; x = 0; int i; i = 0; while (x < 10) { x = x + 1; i = x; } return x;"
assert  5 "int x; x = 1; if (x == 1) { x = x * 5; } else { x = 2; } return x;"
assert  2 "int x; x = 0; if (x == 1) { x = x * 5; } else { x = 2; } return x;"
assert  1 "int x; x = 2; if (x == 1) { x = x * 5; } else if (x == 2) { x = x/2; } else { x = 7; } return x;"
assert  7 "int x; x = 3; if (x == 1) { x = x * 5; } else if (x == 2) { x = x/2; } else { x = 7; } return x;"

assert  3 "int x; x = ret(); return x;"
assert  5 "int x; x = ret5(); return x;"
assert  5 "return ret5();"

test_function_call "foo();"  "func.c" "OK"
test_function_call "test();" "tests/func_call.c" "a test for function call: OK."
test_function_call "test_underscore();" "tests/func_call.c" "a test for function with _: OK."
test_function_call "int x; x = foo_with_arguments(2, 3);"  "func.c" "2, 3"
test_function_call "int x; x = foo_with_6_arguments(2, 3, 4, 5, 6, 7);"  "func.c" "2, 3, 4, 5, 6, 7"
test_function_call "int x; sum(10);"  "func.c" "a result: 55 -> a test for sum: OK."

test_program 1 "int f() { 1; } int main() { f(); }"
test_program 6 "int f() { 6; } int main() { int r; r = f(); return r; }"
test_program 10 "int f() { return 10; } int main() { int r; r = f(); return r; }"
test_program 10 "int f() { int x; x = 10;  x; } int main() { int r; r = f(); return r; }"
test_program 10 "int f() { int x; x = 5;  return x + 5; } int main() { int r; r = f(); return r; }"
test_program 7 "int f() { int x; x = 10; int y; y = 3;  x - y; } int main() { int r; r = f(); return r; }"
test_program 30 "int f() { int x; x = 10; int y; y = 3;  x * y; } int main() { int r; r = f(); return r; }"
test_program 3 "int f() { int x; x = 10; int y; y = 3;  x / y; } int main() { int r; r = f(); return r; }"
test_program 55 "int f() { int x; x = 0; int y; y = 1; int z; z = 1; int c; c = 2; int n; n = 10; while (c <= n) { z = x + y; x = y; y = z; c = c + 1; } return z; } int main() { int r; r = f(); return r; }"
test_program 8 "int f() { int x; x = 2; x; } int g() { int y; y = 4; y; } int main() { f() * g(); }"

test_program 4 "int f(int x) { x * x; } int main() { f(2); }"
test_program 60 "int f(int x) { int y; y = 3; int z; z = 4; return x * y * z; } int main() { int a; a = f(5); return a; }"
test_program 120 "int f(int x, int y, int z) { return x * y * z; } int main() { int a; a = f(4, 5, 6); return a; }"
test_program 8 "int fi(int x) { if (x == 1) return 1; if (x == 2) return 1; int a; a = fi(x-2) + fi(x-1); return a; } int main() { int a; a = fi(6); return a; }"

test_program 6 "int main() { int a; a = 6; int *b; b = &a; return *b; }"
test_program 18 "int main() { int a; a = 6 * 3; int *b; b = &a; return *b; }"
test_program 6 "int main() { int a; a = 6; int b; b = 7; int *c; c = &b + 8; return *c; }"

test_program 70 "int main() { int *a; int b; b = 70; a = &b; return *a; }"
 test_program 7 "int main() { int *a; int b; b = 7; a = &b; int **c; c = &a; return **c; }"
test_program 8 "int main() { int *a; int b; b = 8; a = &b; int **c; c = &a; int ***d; d = &c; return ***d; }"
test_program 10 "int main() { int i; i = 0; do { i = i + 1; } while (i < 10); return i; }"
test_program 128 "int main() { int i; i = 1; int count; count = 0; do { i = i * 2; count = count + 1; } while (count < 7); return i; }"
test_program  5 "int main() { int x; x = 0; int i; for (i = 0; i < 10; i = i + 1) { x = x + 1; if (x == 5) { break; } } return x; }"
test_program 64 "int main() { int i; i = 1; int count; count = 0; while (count < 7) { i = i * 2; if (i == 64) { break; } count = count + 1; } return i; }"
test_program 64 "int main() { int i; i = 1; int count; count = 0; do { i = i * 2; if (i == 64) { break; } count = count + 1; } while (count < 7); return i; }"
test_program 100 "int main() { int i; i = 0; do { i = i + 1; if (i < 100) continue; break; } while(1); i; }"
test_program 100 "int main() { int i; i = 1; while (1) { i = i + 1; if (i < 100) continue; break; } return i; }"
test_program 64 "int main() { int i; i = 1; int count; count = 0; while (count < 7) { count = count + 1; if (i == 64) continue; i = i * 2; } return i; }"
test_program 64 "int main() { int i; i = 1; int count; count = 0; do { if (count == 5) { count = count + 1; continue; } i = i * 2; count = count + 1; } while (count < 7); return i; }"
test_program  7 "int main() { int x; x = 0; int i; for (i = 0; i < 10; i = i + 1) { if (x == 7) { continue; } x = x + 1; } return x; }"
test_program 20 "int main() { int i; i = 0; for (;; i = i + 1) { if (i < 20) continue; break; } return i; }"

test_function_call "int *a; alloc(&a, 22, 5, 63); int *b; b = a; print_num(*b);" "func.c" "22 -> a test for pointer addition and subtraction: OK."
test_function_call "int *a; alloc(&a, 22, 5, 63); int *b; b = a; print_num(*(b + 1));" "func.c" "5 -> a test for pointer addition and subtraction: OK."
test_function_call "int *a; alloc(&a, 22, 9, 63); int *b; b = a + 1; print_num(*b);" "func.c" "9 -> a test for pointer addition and subtraction: OK."
test_function_call "int *a; alloc(&a, 22, 5, 63); int *b; b = a + 2; print_num(*b);" "func.c" "63 -> a test for pointer addition and subtraction: OK."
test_function_call "int *a; alloc(&a, 22, 5, 63); int *b; b = a; print_num(*(b + 2));" "func.c" "63 -> a test for pointer addition and subtraction: OK."
test_function_call "int *a; alloc(&a, 22, 5, 63); int *b; b = a + 2; print_num(*(b - 2));" "func.c" "22 -> a test for pointer addition and subtraction: OK."
test_function_call "int *a; alloc(&a, 22, 5, 63); int *b; b = a + 2; print_num(*(b - 1));" "func.c" "5 -> a test for pointer addition and subtraction: OK."
# test_program 5 "int main() { int *x; *x=3; int *y; *y=5; return *(x+1); }" #TODO: Not passed.

test_error "int main() { int a; int b; b = a * (5 + 3; return b }" "ERROR: expected ')' for pair of '(', but got ; return b }"
test_error "int main() { int a; int b; b = a * 5 + 3); return b; }" "ERROR: expected ;, but got ); return b; }"
test_error "int main() { int a; a = 5; int b; b = 3; return a * b }" "ERROR: expected ;, but got }"
test_error "int main() { int a; int b; b = a * 3; return b; " "ERROR: expected }, but got "
test_error "int main() { 1 = 2 + 3; }" "ERROR: expected identifier, but got 1 = 2 + 3; }"
test_error "int main() { int a; a + 3 = 5; a }" "ERROR: expected identifier, but got 3 = 5; a }"
test_error "int f(int a, int b) { return a * b  int main() { int a; a = f(4, 5); return a; }" "ERROR: expected ;, but got int"
test_error "int f(int a, int b) { return a * b;  int main() { int a; a = f(4, 5); return a; }" "ERROR: expected ';' after int declaration and identifier, but got () { int a; a = f(4, 5); return a; }"
test_error "int f(int a, int b) { return a * b } int main() { int a; a = f(4, 5); return a; }" "ERROR: expected ;, but got } int main() { int a; a = f(4, 5); return a; }"
test_error "int f(int a, int b, int c, int d, int e, int f, int g) { return a * b * c * d * e * f * g; } int main() { a = f(4, 5, 6, 7, 8, 9, 10); return a; }" "ERROR: expected Up to 6 parameters, but got g"
test_error "int main() { int a; *a = 6 * 3; int b; &b = a; return b; }" "ERROR: expected identifier, but got a"
test_error "int main() { int a; *a = 6 * 3; int b; &b = a; return b; }" "ERROR: expected identifier, but got a"
test_error  "int main() { a; a = 3; a; }" "ERROR: expected defined identifier, but got a"
test_error  "int main() { a; a = 3; a = a+3; }" "ERROR: expected defined identifier, but got a"
test_error  "int main() { int a = 3; a = a+3; }" "ERROR: expected ';' after int declaration and identifier, but got = 3; a = a+3; }"
test_error  "f() { 1; } main() { f(); }" "ERROR: expected int, but got f"
test_error  "int f() { 1; } main() { f(); }" "ERROR: expected int, but got main"
test_error  "int f(x) { x * x; } int main() { f(2); }" "ERROR: expected defined identifier or dereference operator, but got x"
test_error "int main() { int i; i = 0; do { i = i + 1; } while (i < 10) return i; }" "ERROR: expected ';' after do-while statement, but got return"
test_error "int main() { int i; i = 1; int count; count = 0; do { i = i * 2; if (i == 64) { break } count = count + 1; } while (count < 7); return i; }" "151: expected ';' after break, but got '}' "
test_error "int main() { int i; i = 1; int count; count = 0; do { i = i * 2; if (i == 64) { continue } count = count + 1; } while (count < 7); return i; }" "144: expected ';' after continue, but got '}' "

echo OK
