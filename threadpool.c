#include "threadpool.h"
#include "future.h"
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

int thread_pool_init(thread_pool_t *pool, size_t num_threads) {

    printf("init\n");

    pool->size = num_threads;
    pool->free_threads = num_threads;
    pool->threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    pool->alive = true;
    pool->to_do_list = NULL;
    pool->end_of_list = NULL;
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
    runnable_t* finish = (runnable_t*)malloc(sizeof(runnable_t));
    finish->arg = NULL;
    finish->function = NULL;
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
    free(finish);

    printf("destroyed\n");
}

int defer(thread_pool_t *pool, runnable_t runnable) {

    sem_wait(&(pool->mutex));
    tdl_t* t = (tdl_t*)malloc(sizeof(tdl_t));
    t->task = runnable;
    t->is_future = false;
    t->next = NULL;
    t->previous = NULL;
    if (pool->to_do_list == NULL) {
        pool->to_do_list = t;
    } else {
        pool->end_of_list->next = t;
    }
    pool->end_of_list = t;

    pool->task = runnable;
    if (pool->free_threads > 0) {
        pool->free_threads--;
        sem_post(&(pool->sem)); //dziedziczenie mutex
    }
    else {
        sem_post(&(pool->mutex));
    }

    return 0;
}

void *work_in_pool(thread_pool_t* pool) {

    printf("Hello, I'm ready to work!\n");

    while (true) {
        sem_wait(&(pool->sem));  // TODO obsluga bledow semafora
        if (pool->task.function == NULL) { //it is NULL when the pool should be destroyed
            sem_post(&(pool->mutex));
            return 0;
        }
        runnable_t my_task = pool->task;
        /*sem_post(&(pool->mutex));
        my_task.function(my_task.arg, my_task.argsz);
        sem_wait(&(pool->mutex));
*/
        while (pool->to_do_list != NULL) {
            bool is_future = pool->to_do_list->is_future;

            future_t* prev_future;
            if (is_future) {
                prev_future = (future_t *) pool->to_do_list->previous;
            }

            my_task = pool->to_do_list->task;
            tdl_t* help = pool->to_do_list;
            pool->to_do_list = pool->to_do_list->next;
            free(help);

            sem_post(&(pool->mutex));

            if (is_future && prev_future->ready == false) {
                await(prev_future);
            }

            if (is_future) {
                callable_t c = ((future_t*)my_task.arg)->callable;
                callable_t* new = (callable_t*)malloc(sizeof(callable_t));
                new->function = c.function;
                new->arg = prev_future->answer;
                new->argsz = c.argsz;
                ((future_t*)my_task.arg)->callable = *new;

            }

            my_task.function(my_task.arg, my_task.argsz);
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