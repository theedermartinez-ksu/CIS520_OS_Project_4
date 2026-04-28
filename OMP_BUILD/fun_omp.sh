#!/bin/bash
#SBATCH --job-name=omp_test
#SBATCH --output=omp_%j.out
#SBATCH --error=omp_%j.err
#SBATCH --constraint=moles
#SBATCH --time=02:00:00

module load GCC

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

SOURCE="openmp.c"
EXEC="omp"

gcc -O2 -fopenmp -o $EXEC $SOURCE

if [ $? -ne 0 ]; then
	echo "Compilation failed"
	exit 1
fi

echo Compilation successful

./"$EXEC" "$OMP_NUM_THREADS" 500000


