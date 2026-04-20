#!/bin/bash
#SBATCH --job-name=hw4
#SBATCH --output=out_%j.txt
#SBATCH --error=err_%j.txt
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --mem-per-cpu=128M
#SBATCH --time=00:30:00
#SBATCH --chdir=/homes/edermar/KSU_Courses/CIS_520-Operating_Systems/hw4

/home/edermar/KSU_Courses/CIS_520-Operating_Systems/hw4/fun.sh $1 $2
