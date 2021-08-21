#!/bin/bash
set -e

if [[ -z $1 ]]; then
  echo "Must supply argument"
  exit 1
fi

gcc -c $1.s
objdump -d $1.o > $1.d
