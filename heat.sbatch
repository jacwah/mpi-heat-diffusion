#!/bin/bash -l
#SBATCH -A edu21.dd2356
#SBATCH -J heat-perf
#SBATCH -t 1:00:00
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=32
#SBATCH -C Haswell
#SBATCH -e error_file.e
#SBATCH -o output_file.o
rm heat-time
for n in 1 4 9 16 25 36 49 64 81 100 121 144 196 225 256; do
	srun -n ${n} ./heat 55440 55440 100 >> heat-time || break
done
