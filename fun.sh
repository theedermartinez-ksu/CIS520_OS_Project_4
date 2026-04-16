#!/bin/bash

# File names
SOURCE="pt1pthreads.c"     # FIle name 
EXEC="program"
OUTPUT="output.txt"

# Compile with pthread
gcc -o $EXEC $SOURCE -lpthread

# Check if compilation succeeded
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

echo "Compilation successful"

# Run and redirect output
./$EXEC > $OUTPUT

echo "Execution finished. Output saved to $OUTPUT"