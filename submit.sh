#! /bin/bash
# SBATCH -p q_student
#SBATCH -p q_student_low_long 
#SBATCH -N 32                  # start on 2 nodes
#SBATCH --ntasks-per-node=32   # start 4 processes per node
#SBATCH --cpu-freq=High       # set CPU frequency to maximum
#SBATCH --time=20:00           # job will run for 1 minute (maximum)


cd HPC
git checkout main
make clean
make
mpirun ./build/bfs 18 16
git checkout pooling
make clean
make
mpirun ./build/bfs 18 16












