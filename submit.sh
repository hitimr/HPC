#! /bin/bash
# SBATCH -p q_student
#SBATCH -p q_student_low_long 
#SBATCH -N 32                  # start on 2 nodes
#SBATCH --ntasks-per-node=32   # start 4 processes per node
#SBATCH --cpu-freq=High       # set CPU frequency to maximum
#SBATCH --time=20:00           # job will run for 1 minute (maximum)



#mpirun ./build/bfs 6 16
#mpirun ./build/bfs 7 16
#mpirun ./build/bfs 8 16
#mpirun ./build/bfs 9 16
#mpirun ./build/bfs 10 16
#mpirun ./build/bfs 11 16
#mpirun ./build/bfs 12 16
#mpirun ./build/bfs 13 16
mpirun ./build/bfs 14 16
#mpirun ./bfs 15 16
#mpirun ./bfs 16 16
#mpirun ./bfs 17 16
#mpirun ./bfs 18 16
#mpirun ./bfs 19 16
#mpirun ./bfs 20 16
#mpirun ./bfs 21 16
#mpirun ./bfs 22 16
#mpirun ./bfs 19 16
#mpirun ./bfs 19 16
#mpirun ./bfs 19 16
#mpirun ./bfs 19 16












