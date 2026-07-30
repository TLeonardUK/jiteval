#!/bin/bash
git fetch
git pull

mkdir -p ../../bin/
gcc ../../test.c -lm -O3 -m64 -o ../../bin/test.elf

retVal=$?
if [ $retVal -ne 0 ]; then
    echo Compilation Failed
    exit 1
fi

../../bin/test.elf

retVal=$?
if [ $retVal -ne 0 ]; then
    echo Tests failed
    exit 1
fi

echo Completed Successfully
exit 0
