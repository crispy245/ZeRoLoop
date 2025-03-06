#!/bin/bash

# Define the initial values for N and M
N=3
M=3

# Define the number of iterations
iterations=8  # Change this to the number of iterations you want

# Loop through the iterations
for ((i=1; i<=iterations; i++))
do
    echo "Running make with N=$N and M=$M"

    # Change directory to fes_unrolled
    cd fes_unrolled || { echo "Directory fes_unrolled not found"; exit 1; }

    # Execute make all with the current N and M values
    make all N=$N M=$M PRINT_FLAG=0

    # Execute make run in parallel and redirect output to files
    make run -j48 > normal_unrolled_${N}_${M}.txt &
    make run -j48 DECODE=false > decoderles_unrolled_${N}_${M}.txt &

    # Increment N by 1 for the next iteration
    N=$((N + 1))
    M=$((M + 1))

    # Return to the previous directory
    cd ..
done

# Wait for all background jobs to finish
wait

echo "All iterations completed."