#!/usr/bin/env bash
CFLAGS="-g -fsanitize=address -Wall -Wextra -Wno-unknown-pragmas"
file_name=$1
base_file_name=${file_name%.c}

gcc $CFLAGS $file_name -o $base_file_name
rm *.o
