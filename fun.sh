#!/bin/bash

# File names
SOURCE="pt1pthreads.c"     # FIle name 
EXEC="program"

#default valiue
THREADS=${1:-4}
MEMORY=${2:-100000}


# Compile with pthread
gcc -o $EXEC $SOURCE -lpthread

# Check if compilation succeeded
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

echo "Compilation successful"

# Run and redirect output
./$EXEC $THREADS $MEMORY

echo "Execution finished. Output saved"