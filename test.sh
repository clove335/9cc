try() {
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



try  0 "0;"
try 42 "42;"
try 21 '5+20-4;'
try 41 " 12 + 34 - 5;" 
try 47 "5+6*7;"
try  5 "15/3;"
try 77 "(5+6)*7;"
try  3 "15/(3+2);"
try 22 "b = 5 * 6 - 8;"
try 12 "a=z=3*2;a+z;"
try  6 "a = 3; a = a+3;"
try 12 "6*4-3*4;"
try  6 "c = 2; c*3;"
try  5 "return 5;"
try  8 "c=5; b=3; c+b;"
try  2 "c = 2;
        b = 1;
        return c;"
try 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"
try 68 "a = 12; x = (3+5)*7; b = a+x; return b;"
try 45 "foo = 10; bar = 35; return foo + bar;"
try  4 "return +4;"
try  1 "return -(6+1)+8;"
try  5 "n=-10+15; return n;"
try  9 "num = -3*-3; return num;"

echo OK
