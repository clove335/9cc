#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
 ret() { return 3; }
 ret5() { return 5; }
EOF

assert() {
  expected="$1"
  input="$2"

  ./9cc "main() { $input }" > tmp.s
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
  exp=$1
  stub=$2
  output=$3
  
  ./9cc "main() { $exp }" > out.s
  if [ $? -ne 0 ]; then
    echo failed to generate assembly from "$exp"
    rm out.s
    exit 1
  fi

  gcc -c out.s -o out.o
  if [ $? -ne 0 ]; then
    echo failed to generate .o file from "$exp"
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
    cat out.s
    rm out.s out.o stub.o out stdout.txt
    exit 1
  else
    echo "$exp => $output"
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
assert 12 "a=z=3*2;a+z;"
assert  3 "a = 3; a;"
assert  6 "a = 3; a = a+3;"
assert 22 "b = 5 * 6 - 8;"
assert 12 "6*4-3*4;"
assert  6 "c = 2; c*3;"

assert  5 "return 5;"
assert  8 "c=5; b=3; c+b;"
assert  2 "c = 2;
        b = 1;
        return c;"
assert 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"
assert 68 "a = 12; x = (3+5)*7; b = a+x; return b;"
assert 45 "foo = 10; bar = 35; return foo + bar;"
assert  4 "return +4;"
assert  1 "return -(6+1)+8;"
assert  5 "n=-10+15; return n;"
assert  9 "num = -3*-3; return num;"

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
assert  15 "a = 15; if (a) return a; return 5;"
assert  1 "x = 1; if (3*4 > 10) return x; return 0;"
assert  0 "x = 1; if (3*4 < 10) return x; return 0;"
assert  2 "x = 5; if (x == 5) x = 2; else x = 15; return x;"
assert  15 "x = 5; if (x == 1) x = 2; else x = 15; return x;"
assert  15 "x = 5; if (x == 1) x = 2; else if (x == 5) x = 15; return x;"
assert  5 "x = 5; if (x == 1) x = 2; else if (x == 4) x = 15; else x; return x;"

assert  7 "x = 0; while (x < 7) x = x + 1; return x;"
assert  0 "x = 10; while (x > 0) x = x - 1; return x;"
assert  10 "x = 0; i = 0; for (; i<10; i=i+1) x = x + 1; return x;"
assert  55 "x = 0; i = 0; for (; i<=10; i=i+1) x = x + i; return x;"
assert  10 "x = 0; for (;;) return 10; return x;"

assert  3 "{1; {2;} return 3;}"
assert  10 "x = 0; i = 0; while (x < 10) { x = x + 1; i = x; } return x;"
assert  5 "x = 1; if (x == 1) { x = x * 5; } else { x = 2; } return x;"
assert  2 "x = 0; if (x == 1) { x = x * 5; } else { x = 2; } return x;"
assert  1 "x = 2; if (x == 1) { x = x * 5; } else if (x == 2) { x = x/2; } else { x = 7; } return x;"
assert  7 "x = 3; if (x == 1) { x = x * 5; } else if (x == 2) { x = x/2; } else { x = 7; } return x;"

assert  3 "x = ret(); return x;"
assert  5 "x = ret5(); return x;"
assert  5 "return ret5();"

test_function_call "foo();"  "func.c" "OK"
test_function_call "test();" "tests/func_call.c" "a test for function call: OK."
test_function_call "test_underscore();" "tests/func_call.c" "a test for function with _: OK."
test_function_call "x = foo_with_arguments(2, 3);"  "func.c" "2, 3"
test_function_call "x = foo_with_6_arguments(2, 3, 4, 5, 6, 7);"  "func.c" "2, 3, 4, 5, 6, 7"
test_function_call "sum(10);"  "func.c" "a result: 55 -> a test for sum: OK."

test_program 1 "f() { 1; } main() { f(); }"
test_program 6 "f() { 6; } main() { r = f(); return r; }"
test_program 10 "f() { return 10; } main() { r = f(); return r; }"
test_program 10 "f() { x = 10;  x; } main() { r = f(); return r; }"
test_program 10 "f() { x = 5;  return x + 5; } main() { r = f(); return r; }"
test_program 7 "f() { x = 10; y = 3;  x - y; } main() { r = f(); return r; }"
test_program 30 "f() { x = 10; y = 3;  x * y; } main() { r = f(); return r; }"
test_program 3 "f() { x = 10; y = 3;  x / y; } main() { r = f(); return r; }"
test_program 55 "f() { x = 0; y = 1; z = 1; c = 2; n = 10; while (c <= n) { z = x + y; x = y; y = z; c = c + 1; } return z; } main() { r = f(); return r; }"
test_program 8 "f() { x = 2; x; } g() { y = 4; y; } main() { f() * g(); }"

test_program 4 "f(x) { x * x; } main() { f(2); }"
test_program 60 "f(x) { y = 3; z = 4; return x * y * z; } main() { a = f(5); return a; }"
test_program 120 "f(x, y, z) { return x * y * z; } main() { a = f(4, 5, 6); return a; }"
test_program 8 "fi(x) { if (x == 1) return 1; if (x == 2) return 1; a = fi(x-2) + fi(x-1); return a; } main() { a = fi(6); return a; }"

test_program 6 "main() { a = 6; b = &a; return *b; }"
test_program 18 "main() { a = 6 * 3; b = &a; return *b; }"
test_program 6 "main() { a = 6; b = 7; c = &b + 16; return *c; }"

test_error "main() { b = a * (5 + 3; return b }" "ERROR: expected ')' for pair of '(', but got ; return b }"
test_error "main() { b = a * 5 + 3); return b; }" "ERROR: expected ;, but got ); return b; }"
test_error "main() { a = 5; b = 3; return a * b }" "ERROR: expected ;, but got }"
test_error "main() { b = a * 3; return b; " "ERROR: expected }, but got "
test_error "main() { 1 = 2 + 3; }" "ERROR: expected identifier, but got "
test_error "main() { a + 3 = 5; a * b }" "ERROR: expected ;, but got }"
test_error "f(a, b) { return a * b  main() { a = f(4, 5); return a; }" "ERROR: expected ;, but got main"
test_error "f(a, b) { return a * b;  main() { a = f(4, 5); return a; }" "ERROR: expected ;, but got "
test_error "f(a, b) { return a * b } main() { a = f(4, 5); return a; }" "ERROR: expected ;, but got } main() { a = f(4, 5); return a; }"
test_error "f(a, b, c, d, e, f, g) { return a * b * c * d * e * f * g; } main() { a = f(4, 5, 6, 7, 8, 9, 10); return a; }" "ERROR: expected Up to 6 parameters, but got g"
test_error "main() { *a = 6 * 3; &b = a; return b; }" "ERROR: expected identifier, but got "

echo OK
