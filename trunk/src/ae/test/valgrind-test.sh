#!/bin/bash
valgrind -v --trace-children=yes --log-file=ae-valgrind.out --num-callers=12 --freelist-vol
