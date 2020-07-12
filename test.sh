assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  if [ "$?" != 0 ]; then
    echo "9cc error : $input"
    exit 1
  fi
  gcc -o tmp tmp.s
  if [ "$?" != 0 ]; then
    echo "compile error : $input"
    nl -w2 tmp.s
    exit 1
  fi
  ./tmp
  actual="$?"
  
  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
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


echo OK
