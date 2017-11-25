#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include<time.h>
#define INF 100000
int debug = 0;

double cummTime = 0;
double IOTime = 0;
double totalTime = 0;
struct timespec diff(struct timespec start, struct timespec end) {
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}
// MPI function that record time
int myMPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request){
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
    MPI_Isend(buf, count, datatype, dest, tag, comm, request);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    cummTime += time_used;
}

int myMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
    MPI_Send(buf, count, datatype, dest, tag, comm);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    cummTime += time_used;
}

int myMPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request){
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
    MPI_Irecv(buf, count, datatype, source, tag, comm, request);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    cummTime += time_used;
}

int myMPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status){
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
    MPI_Recv(buf, count, datatype, source, tag, comm, status);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    cummTime += time_used;
}

int myMPI_Wait(MPI_Request *request, MPI_Status *status){
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
    MPI_Wait(request, status);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    cummTime += time_used;
}

int myMPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[]){
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
    MPI_Waitall(count, array_of_requests, array_of_statuses);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    cummTime += time_used;
}
int myMPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
    MPI_Bcast(buffer, count, datatype, root, comm);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    cummTime += time_used;
}

int myMPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm){
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
    MPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    cummTime += time_used;
}

int **adj; // adjacent matrix
int *dist; // distance to different source
int V, E, N;
int rank, size;

void read(char* in){
    FILE* f = fopen(in, "r");
    fscanf(f, "%d%d", &V, &E);

    // read edges
    for(int e = 0; e < E; e++){
        int i, j, w;
        fscanf(f, "%d%d%d", &i, &j, &w);
        adj[i][j] = adj[j][i] = w;
    }
    fclose(f);
}

void write(char* out){
    FILE* f = fopen(out, "w");
    for(int i = 0; i < V; i++){
        for(int j = 0; j < V; j++){
            fprintf(f, "%d ", adj[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void APSP(){
    MPI_Request requests[N]; 
    MPI_Status status;
    for(int i = 0; i < N; i++){
        dist[i] = adj[rank][i];
    }
    for(int epoch = 0; epoch < N; epoch++){
        // send to neighbor
        for(int i = 0; i < N; i++){
            if((i != rank) && (adj[rank][i] != INF)){
                MPI_Isend(dist, N, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[i]);
            }
        }
        int flag = 0;
        int gflag;
        // receive from neighbor
        for(int i = 0; i < N; i++){
            if((i != rank) && (adj[rank][i] != INF)){
                int tmp[N];
                MPI_Recv(tmp, N, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
                for(int j = 0; j < N; j++){
                    if(dist[i]+tmp[j] < dist[j]){ // if (self to i) + (i to j) < (self to j)
                        dist[j] = dist[i] + tmp[j]; // (self to j) = (self to i) + (i to j)
                        flag = 1;
                    }
                }
            }
        }
        // collect flag from all
        MPI_Allreduce(&flag, &gflag, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        if(gflag == 0){
            if(debug) printf("epoch[%d]: break\n", epoch);
            break;
        }

    }
}

int main(int argc, char** argv) {
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // argument parsing
	assert(argc == 4);
    char *in = argv[1];
    char *out = argv[2];
	V = N = atoi(argv[3]);

    // init MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

    // initialize adjacent matrix
    adj = (int**) malloc(sizeof(int*)*V);
    int *tmp = (int*) malloc(sizeof(int)*V*V);
    dist = (int*) malloc(sizeof(int)*V);
    for(int i = 0; i < V; i++){
        adj[i] = &tmp[i*V];
        for(int j = 0; j < V; j++)
            adj[i][j] = INF;
        adj[i][i] = 0;
    }

    // read file
    if(rank==0)
        read(in);

    // broadcast 
    myMPI_Bcast(tmp, V*V, MPI_INT, 0, MPI_COMM_WORLD);
    if(false){
        printf("process[%d] adjacent matrix:\n", rank);
        for(int i = 0; i < V; i++){
            for(int j = 0; j < V; j++)
                printf("%d ", adj[i][j]);
            printf("\n");
        }
    }

    // APSP
    APSP();

    // gather distance
    MPI_Gather(dist, V, MPI_INT, tmp, V, MPI_INT, 0, MPI_COMM_WORLD);
    if(debug && rank == 0){
        printf("adjacent matrix:\n");
        for(int i = 0; i < V; i++){
            for(int j = 0; j < V; j++)
                printf("%d ", adj[i][j]);
            printf("\n");
        }
    }
    // write file
    //
    if(rank==0)
        write(out);

    // MPI_Finalize and free memory
	MPI_Finalize();
    free(adj);
    free(tmp);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    totalTime += time_used;
    totalTime -= cummTime;
    totalTime -= IOTime;
    printf("%f %f %f\n", totalTime, cummTime, IOTime);
    //printf("rank%d: %f %f %f\n", rank, totalTime, cummTime, IOTime);

}

