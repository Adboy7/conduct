#!/bin/bash
#
#SBATCH --job-name=test
#SBATCH --output=res.txt
#
#SBATCH --ntasks=1
#SBATCH --time=10:00
#SBATCH --mem-per-cpu=100

echo "Thread"
echo "Thread==1"
export OMP_NUM_THREADS=1
time ./conducting 1  150 0.23 5000
echo "-"
echo "Thread==2"
export OMP_NUM_THREADS=2
time ./conducting 1  150 0.23 5000
echo "-"
echo "Thread==5"
export OMP_NUM_THREADS=5
time ./conducting 1  150 0.23 5000
echo "-"
echo "Thread==10"
export OMP_NUM_THREADS=10
time ./conducting 1  150 0.23 5000
echo "--------------------"

echo "d"
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
for i in $(seq 0.0 0.01 1.0);
    do
        echo "d==$i ----- " 
        time ./conducting 1 150 $i 5000
done

echo "N"
echo "N==100"
time ./conducting 1 100 0.23 5000
echo "N==150"
time ./conducting 1 150 0.23 5000
echo "N==200"
time ./conducting 1 200 0.23 5000
echo "N==250"
time ./conducting 1 250 0.23 5000
echo "N==300"
time ./conducting 1 300 0.23 5000
echo "N==350"
time ./conducting 1 350 0.23 5000
echo "N==400"
time ./conducting 1 400 0.23 5000

echo "M"
echo "M==100"
time ./conducting 1 150 0.23 100
echo "M==500"
time ./conducting 1 150 0.23 500
echo "M==1000"
time ./conducting 1 150 0.23 1000
echo "M==2500"
time ./conducting 1 150 0.23 2500
echo "M==5000"
time ./conducting 1 150 0.23 5000
echo "M==10000"
time ./conducting 1 150 0.23 10000

