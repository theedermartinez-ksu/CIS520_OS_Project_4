#!/bin/bash

JOBID=$1
CPUS=$2
MEM=$3
NODES=$4

OUTPUT="output_${NODES}n_${CPUS}c_${MEM}_${JOBID}.txt"

echo "===== PERFORMANCE RESULTS =====" >> "$OUTPUT"

sacct -j "$JOBID" --format=JobID,Elapsed,TotalCPU,MaxRSS,State >> "$OUTPUT"

echo "" >> "$OUTPUT"
echo "===== SEFF =====" >> "$OUTPUT"

seff "$JOBID" >> "$OUTPUT"
