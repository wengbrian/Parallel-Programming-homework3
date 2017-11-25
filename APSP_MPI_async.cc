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
int debug = 1;

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

int **dist; // adjacent matrix
int *dist2; // distance for different source
int *num; // num ack not accepted
int V, E, N;
int rank, size;
int *pred;

void read(char* in){
    FILE* f = fopen(in, "r");
    fscanf(f, "%d%d", &V, &E);

    // read edges
    for(int e = 0; e < E; e++){
        int i, j, w;
        fscanf(f, "%d%d%d", &i, &j, &w);
        dist[i][j] = dist[j][i] = w;
    }
    fclose(f);
}

void write(char* out){
    FILE* f = fopen(out, "w");
    for(int i = 0; i < V; i++){
        for(int j = 0; j < V; j++){
            fprintf(f, "%d ", dist[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void APSP(){
    MPI_Status status;
    status.MPI_SOURCE = rank; // at start, pred[rank] = rank
    pred[rank] = rank;
    int root = rank;
    int new_dist = 0; // at start, distacne to self is 0
    int d[N];
    int has_send[N];
    MPI_Request requests[N];
    for(int i = 0; i < N; i++){
        d[i] = INF;
        requests[i] = MPI_REQUEST_NULL;
        has_send[i] = 0;
    }
    int ACK = -1;
    int total_num = 0;
    
    do{
        if((new_dist != ACK) && (new_dist < dist2[root])){
            // send ack to old pred
            if((pred[root] != rank) && (has_send[root]==0)){
                MPI_Isend(&ACK, 1, MPI_INT, pred[root], root, MPI_COMM_WORLD, &requests[rank]);
            }
            if(dist2[root] != INF){
                if(debug) printf("process[%d]: new distance %d is shorter than %d for root %d, send ACK to P%d and change pred to P%d\n", rank, new_dist, dist2[root], root, pred[root], status.MPI_SOURCE);
            }else{
                if(debug) printf("process[%d]: new distance %d is shorter than init %d for root %d, change pred to P%d\n", rank, new_dist, dist2[root], root, status.MPI_SOURCE);
            }
            dist2[root] = new_dist; /* start searching around vertex */
            pred[root] = status.MPI_SOURCE;
            has_send[root] = 0;
            for(int i = 0; i < V; i++){ /* get next edge */
                if ((i != pred[root]) && (i != rank) && (dist[rank][i] != INF)) {
                    d[i] = dist2[root] + dist[rank][i];
                    MPI_Status tmp_status;
                    if(requests[i] != MPI_REQUEST_NULL) MPI_Wait(&requests[i], &tmp_status);
                    MPI_Isend(&d[i], 1, MPI_INT, i, root, MPI_COMM_WORLD, &requests[i]);
                    num[root]++;
                    total_num++;
                    if(debug) printf("process[%d]: send %d to P%d for root %d, num: %d/%d\n", rank, d[i], i, root, num[root], total_num);
                }
            }
            if(num[root] == 0){ // no message send, return ACK
                MPI_Isend(&ACK, 1, MPI_INT, status.MPI_SOURCE, root, MPI_COMM_WORLD, &requests[rank]);
            }
        }else if(new_dist == ACK){
            // if receive ACK and num[root]==0, send ACK to pred[root]
            //printf("process[%d]: ACK received from P%d for root %d\n", rank, status.MPI_SOURCE, root);
        }else{
            // if new distance < shortest distance to root, send ACK back to source
            if(debug) printf("process[%d]: new distance %d is not shorter than %d"
                " for root %d, send ACK to P%d\n", rank, new_dist, dist2[root], root, status.MPI_SOURCE);
            MPI_Isend(&ACK, 1, MPI_INT, status.MPI_SOURCE, root, MPI_COMM_WORLD, &requests[rank]);
        }
        myMPI_Recv(&new_dist, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        root = status.MPI_TAG;
        if(new_dist == ACK){ // receive ACK
            num[root]--;
            total_num--;
            if(debug) printf("precess[%d]: receive ACK for root %d from P%d,"
                                " num: %d/%d\n", rank, root, status.MPI_SOURCE, num[root], total_num);
            if((num[root]==0) && (root!=rank)){ // if num become zero, send to pred, do not send to self
                has_send[root] = 1;
                if(debug) printf("process[%d]: All ACK received,"
                    " send ACK to parent P%d for root %d\n", rank, pred[root], root);
                MPI_Isend(&ACK, 1, MPI_INT, pred[root], root, MPI_COMM_WORLD, &requests[rank]);
            }
        }else{
            if(debug) printf("precess[%d]: receive new distance %d"
                " for root %d from P%d\n", rank, new_dist, root, status.MPI_SOURCE);
        }
    }while(total_num > 0);
    if(debug) printf("exit loop\n");
    if(debug) printf("%d %d %d %d\n", rank, dist2[0], dist2[1], dist2[2]);
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
    int *adj = (int*) malloc(sizeof(int)*V*V);
    dist = (int**) malloc(sizeof(int*)*V);
    num = (int*) malloc(sizeof(int)*V);
    dist2 = (int*) malloc(sizeof(int)*V);
    pred = (int*) malloc(sizeof(int)*V);
    for(int i = 0; i < V; i++){
        dist[i] = &adj[i*V];
        for(int j = 0; j < V; j++)
            dist[i][j] = INF;
        dist[i][i] = 0;
        num[i] = 0;
        dist2[i] = INF;
        pred[i] = -1;
    }

    // read file
    if(rank==0)
        read(in);
    // broadcast 
    myMPI_Bcast(adj, V*V, MPI_INT, 0, MPI_COMM_WORLD);
    APSP();
    MPI_Gather(dist2, V, MPI_INT, adj, V, MPI_INT, 0, MPI_COMM_WORLD);
    if(debug && rank == 0){
        for(int i = 0; i < V; i++){
            for(int j = 0; j < V; j++)
                printf("%d ", adj[i*V+j]);
            printf("\n");
        }
    }
    if(rank==0)
        write(out);
	MPI_Finalize();
    free(adj);
    free(dist);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    totalTime += time_used;
    totalTime -= cummTime;
    totalTime -= IOTime;
    printf("%f %f %f\n", totalTime, cummTime, IOTime);
    //printf("rank%d: %f %f %f\n", rank, totalTime, cummTime, IOTime);

}

