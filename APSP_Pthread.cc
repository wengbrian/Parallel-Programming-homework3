#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#define INF 100000
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
int **dist;
int V, E, N;
pthread_cond_t cond;
pthread_mutex_t mutex;
int counter;
int *rowBound;

void read(char* in){
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
    FILE* f = fopen(in, "r");
    fscanf(f, "%d%d", &V, &E);

    // initialize adjacent matrix
    dist = (int**) malloc(sizeof(int*)*V);
    rowBound = (int*) malloc(sizeof(int)*V);
    for(int i = 0; i < V; i++){
        dist[i] = (int*) malloc(sizeof(int)*V);
        for(int j = 0; j < V; j++)
            dist[i][j] = INF;
        //printf("%d %d\n", dist[i][V-1], dist[i][V-1]==100000?1:0);
        dist[i][i] = 0;
        rowBound[i] = V*i-i*(i+1)/2;
    }

    // read edges
    for(int e = 0; e < E; e++){
        int i, j, w;
        fscanf(f, "%d%d%d", &i, &j, &w);
        dist[i][j] = dist[j][i] = w;
    }
    fclose(f);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    IOTime += time_used;
}

void write(char* out){
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
    FILE* f = fopen(out, "w");
    for(int i = 0; i < V; i++){
        for(int j = 0; j < V; j++){
            fprintf(f, "%d ", dist[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    IOTime += time_used;
}

void* worker(void* arg){
    int tid = *((int*) arg);
    int total = V*(V-1)/2;
    int re = total%N, avg = total/N;
    int start = avg*tid + ((tid > re) ? re : tid);
    int end = start + avg + ((tid < re) ? 1 : 0);
    //printf("ID[%d]: (%d, %d)\n", tid, start, end);
    // find current row from start

    for(int k = 0; k < V; k++){
        int curRow;
        for(int i = 1; i < V; i++){
            if(start < rowBound[i]){
                curRow = i;
                break;
            }
        }
        for(int pos = start; pos < end; pos++){
            if(pos >= rowBound[curRow])
                curRow++;
            int skip = curRow*(curRow+1)/2;
            int i = (pos+skip)/V, j = (pos+skip)%V;
            //printf("[%d](%d, %d, %d):%d v.s. %d+%d\n", tid, i, j, k, dist[i][j], dist[i][k], dist[k][j]);
            if(dist[i][j] > dist[i][k] + dist[k][j]){
                dist[j][i] = dist[i][j] = dist[i][k] + dist[k][j];
                //printf("%d -> %d new dist: %d\n", i, j, dist[i][j]);
            }
        }

        pthread_mutex_lock(&mutex);
        struct timespec start, end, temp;
        clock_gettime(CLOCK_MONOTONIC, &start);
        counter++;
        if(counter < N){
            pthread_cond_wait(&cond, &mutex);
        }else{
            pthread_cond_broadcast(&cond);
            counter = 0;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        temp = diff(start, end);
        double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
        cummTime += time_used;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    struct timespec start, end, temp;
    clock_gettime(CLOCK_MONOTONIC, &start);
	assert(argc == 4);
    char *in = argv[1];
    char *out = argv[2];
	N = atoi(argv[3]);
    read(in);
    pthread_t threads[N];
    int ID[N];
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex, NULL);
    for(int i = 0; i < N; i++){
        ID[i] = i;
        pthread_create(&threads[i], NULL, worker, (void*)&ID[i]);
    }
    for(int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);
    write(out);
    free(rowBound);
    for(int i = 0; i < V; i++)
        free(dist[i]);
    free(dist);
    clock_gettime(CLOCK_MONOTONIC, &end);
    temp = diff(start, end);
    double time_used = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
    totalTime += time_used;
    cummTime /= N;
    totalTime -= cummTime;
    totalTime -= IOTime;
    printf("%f %f %f\n", totalTime, cummTime, IOTime);
}

