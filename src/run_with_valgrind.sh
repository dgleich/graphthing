#! /bin/bash

OUTPUT="valgrind.log"

echo "Running with valgrind, output to \"$OUTPUT\"..."
valgrind -v --leak-check=yes --num-callers=8 \
	./gt 2> $OUTPUT
