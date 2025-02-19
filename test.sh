#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 6 '10 -3 + 4   -5;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 20 '50 + 5 * -6;'
assert 10 '-(-10);'
assert 20 'a=20;'
assert 20 'a=10;a+10;'
assert 30 'abc=20;def=10;abc+def;'
assert 5 'a=1;b1=1;b_=1;b1a_=1;ok=1;a+b1+b_+b1a_+ok;'
assert 12 'return 12;'
assert 10 'if(1) 10;'
assert 31 'if(0) 5; else 31;'

echo OK
