#!/bin/bash

function assert() {
    expected=$1
    input=$2

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp

    actual="$?"

    if [ "$expected" = "$actual" ] ; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got of $actual"
        exit 1
    fi
}

assert 0 0
assert 42 42

assert 21 '5+20-4'
assert 35 '1+2+3+4-5+6+7+8+9'

echo OK
