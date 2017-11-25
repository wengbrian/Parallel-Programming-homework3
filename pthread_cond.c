#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
int arr[10];
pthread_cond_t cond;
pthread_mutex_t mutex;
int ok = 0;

void* fill() {
    pthread_mutex_lock(&mutex);
    int i = 0;
    printf("Enter 4 values\n");
    for (i = 0; i < 4; i++) {
        scanf("%d", &arr[i]);
    }
    ok = 1;
    // You need to use condition variable here to signal another thread.
    // Aware all condition variable must be perfomed while a mutex is locked!
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void* read() {
    // You need to use condition variable here to wait for another thread.
    // Aware all condition variable must be perfomed while a mutex is locked!
    pthread_mutex_lock(&mutex);
    if(!ok)
        pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    printf("Values filled in array are\n");
    int i;
    for (i = 0; i < 4; i++) {
        printf("%d \n", arr[i]);
    }
    pthread_exit(NULL);
}

int main(void) {
    pthread_t worker_id[2];
    //cond = PTHREAD_COND_INITIALIZER;
    //mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex, NULL);
    /* Create thread */
    pthread_create(&worker_id[0], NULL, &fill, NULL);
    pthread_create(&worker_id[1], NULL, &read, NULL);
    printf("Threads have been created\n");
    /* Wait for all threads */
    pthread_join(worker_id[0], NULL);
    pthread_join(worker_id[1], NULL);
    return 0;
}
