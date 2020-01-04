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
    pool->free_threads = num_threads;
    pool->threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    pool->alive = true;
    sem_t sem, mutex;
    sem_init(&sem, 0, 0);
    sem_init(&mutex, 0, 1);
    pool->sem = sem;
    pool->mutex = mutex;

    for (size_t i =0; i < num_threads; i++) {
        pthread_t thread;
        int result = pthread_create(&thread, NULL, work_in_pool, (void *)(pool));
        if (result != 0) {
            perror("Could not create thread.");
        }
        pool->threads[i] = thread;
    }

    printf("initialised\n");

    return 0;
}

void thread_pool_destroy(struct thread_pool *pool) {

    printf("\ndestroy\n");
    runnable_t* finish = (runnable_t*)malloc(sizeof(runnable_t)); //everything in it is NULL
    pool->alive = false;
    while (pool->free_threads > 0)
        defer(pool, *finish);

    for (size_t i = 0; i < pool->size; i++) {
        pthread_join(pool->threads[i], NULL);
        printf("Thread nr %zd exiting\n", i);
    }
    free(pool->threads);
    sem_destroy(&(pool->sem));
    sem_destroy(&(pool->mutex));

    printf("destroyed\n");
}

int defer(struct thread_pool *pool, runnable_t runnable) {

    sem_wait(&(pool->mutex));
    //printf("defer %zd\n", pool->free_threads);
    pool->task = runnable;
    if (pool->free_threads > 0) {
        pool->free_threads--;
        pool->task = runnable;
        sem_post(&(pool->sem)); //dziedziczenie mutex
    }
    else {
        tdl_t* t = (tdl_t*)malloc(sizeof(tdl_t));
        t->next = pool->to_do_list;
        t->task = runnable;
        pool->to_do_list = t;
        sem_post(&(pool->mutex));
    }

    return 0;
}

void *work_in_pool(thread_pool_t* pool) {

    printf("Hello, I'm ready to work!\n");

    while (true) {
        sem_wait(&(pool->sem));
        if (pool->task.function == NULL) {
            sem_post(&(pool->mutex));
            return 0;
        }
        //printf("work\n");
        runnable_t my_task = pool->task;
        sem_post(&(pool->mutex));
        my_task.function(my_task.args, my_task.argsz);
        //printf("finished work\n");
        sem_wait(&(pool->mutex));
        while (pool->to_do_list != NULL) {
            my_task = pool->to_do_list->task;
            pool->to_do_list = pool->to_do_list->next;
            sem_post(&(pool->mutex));
            //printf("work from tdl\n");
            my_task.function(my_task.args, my_task.argsz);
            //printf("finished work from tdl\n");
            sem_wait(&(pool->mutex));
        }
        if (pool->alive == false) {
            sem_post(&(pool->mutex));
            return 0;
        }
        pool->free_threads++;
        sem_post(&(pool->mutex));
    }
}