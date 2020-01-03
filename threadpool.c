#include "threadpool.h"
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

int thread_pool_init(thread_pool_t *pool, size_t num_threads) {

    printf("init\n");

    pool->size = num_threads;
    pool->threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    sem_t sem;
    sem_init(&sem, 0, 0);
    pool->sem = sem;

    for (size_t i =0; i < num_threads; i++) {
        pthread_t thread;
        int result = pthread_create(&thread, NULL, work_in_pool, (void *)(pool));
        if (result != 0) {
            perror("Could not create thread.");
        }
        pool->threads[i] = thread;
    }
    return 0;
}

void thread_pool_destroy(struct thread_pool *pool) {

    printf("destroy\n");
    sem_destroy(&(pool->sem));
    for (size_t i = 0; i < pool->size; i++) {
        pthread_kill(pool->threads[i], 2);
    }
    printf("destroyed\n");

    for (size_t i = 0; i < pool->size; i++) {
        pthread_join(pool->threads[i], NULL);
        printf("Thread nr %zd exiting\n", i);
    }
    free(pool->threads);
}

int defer(struct thread_pool *pool, runnable_t runnable) {
    return 0;
}

void *work_in_pool(thread_pool_t* pool) {

    printf("Hello, I'm ready to work!\n");

    while (true) {
        sem_wait(&(pool->sem));
        printf("work\n");
        //sem_post(&(pool->sem));
    }

    return 0;
}