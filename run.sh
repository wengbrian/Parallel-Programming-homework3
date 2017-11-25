gcc APSP_Pthread.cc -o APSP_Pthread -pthread -lm
mpicxx APSP_MPI_sync.cc -o APSP_MPI_sync -fopenmp
mpicxx APSP_MPI_async.cc -o APSP_MPI_async -fopenmp

