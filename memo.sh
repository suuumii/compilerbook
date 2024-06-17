#!/bin/bash

docker run --rm -it -v $HOME/workspace/compilerbook:/9cc -w /9cc --platform=linux/amd64 compilerbook /bin/bash

# Windows Git Bash
# docker run --rm -it -v /$PWD://9cc -w //9cc compilerbook //bin/bash
# cc -o tmp tmp.s -Wa,--noexecstac

