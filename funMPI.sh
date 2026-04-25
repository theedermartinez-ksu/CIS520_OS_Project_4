#!/bin/bash
#SBATCH --job-name=mpi_max_chars
#SBATCH --output=mpi_%j.out
#SBATCH --error=mpi_%j.err
#SBATCH --partition=mole
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4
#SBATCH --mem-per-cpu=1G
#SBATCH --time=02:00:00
 
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0
 
#file names
SOURCE="mpi.c"
EXEC="mpi"
RANKS=4
ARRAY_SIZE=1073741824
 
#compile
mpicc -O2 -o $EXEC $SOURCE
 
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi
 
echo "Compilation successful"
 
mpirun -np $RANKS ./$EXEC $RANKS $ARRAY_SIZE
 
echo "Execution finished. Output saved to output_mpi${RANKS}-${ARRAY_SIZE}.txt"