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

echo OK
