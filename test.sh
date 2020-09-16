#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
 ret() { return 3; }
 ret5() { return 5; }
EOF

assert() {
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
  if [ "$?" != 0 ]; then
    echo "compile error : $input"
    nl -w2 tmp.s
    exit 1
  fi
  
  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

test_function_call() {
  exp=$1
  stub=$2
  output=$3
  
  #echo "$exp" | ./9cc > out.s
  ./9cc "$exp" > out.s
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
  #echo output
  #echo "$output"
  #echo stdout
  #cat stdout.txt
  #echo dev/null
  #cat /dev/null
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

assert  0 "0;"
assert 42 "42;"
assert 21 '5+20-4;'
assert 41 " 12 + 34 - 5;" 
assert 47 "5+6*7;"
assert  5 "15/3;"
assert 77 "(5+6)*7;"
assert  3 "15/(3+2);"
assert 12 "a=z=3*2;a+z;"
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

echo OK
