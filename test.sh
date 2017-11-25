srun -n3 --overcommit ./APSP_MPI_sync testcase/1.in out/1.out 3
diff out/1.out testcase/1.out
srun -n10 --overcommit ./APSP_MPI_sync testcase/2.in out/2.out 10
diff out/2.out testcase/2.out
srun -n50 --overcommit ./APSP_MPI_sync testcase/3.in out/3.out 50
diff out/3.out testcase/3.out
srun -n100 --overcommit ./APSP_MPI_sync testcase/4.in out/4.out 100
diff out/4.out testcase/4.out
srun -n150 --overcommit ./APSP_MPI_sync testcase/5.in out/5.out 150
diff out/5.out testcase/5.out
