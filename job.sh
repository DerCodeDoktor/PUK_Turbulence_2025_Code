#!/usr/bin/env bash
#SBATCH --job-name=PUK
#SBATCH --partition=modi_short
#SBATCH --nodes=1
#SBATCH --ntasks=38
#SBATCH --threads-per-core=1
##SBATCH --exclusive

mpirun -n $SLURM_NTASKS -- apptainer exec \
   ~/modi_images/ucphhpc/hpc-notebook:latest \
   ./Calc_Energy $1
