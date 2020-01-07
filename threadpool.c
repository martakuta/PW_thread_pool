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
        t->task = runnable;
        t->is_future = false;
        if (pool->to_do_list == NULL) {
            pool->to_do_list = t;
            //printf("pierwszy element\n");
        } else {
            pool->end_of_list->next = t;
        }
        pool->end_of_list = t;
        //printf("dodaj na koniec\n");
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
        sem_post(&(pool->mutex));
        my_task.function(my_task.arg, my_task.argsz);
        printf("did the first\n");
        sem_wait(&(pool->mutex));

        while (pool->to_do_list != NULL) {
            printf("while\n");
            bool is_future = pool->to_do_list->is_future;

            future_t* prev_future;
            if (is_future)
                prev_future = (future_t *) pool->to_do_list->previous;

            my_task = pool->to_do_list->task;
            pool->to_do_list = pool->to_do_list->next;
            
            sem_post(&(pool->mutex));

            if (is_future && prev_future->ready == false) {
                await(prev_future);
            }

            my_task.function(my_task.arg, my_task.argsz);
            sem_wait(&(pool->mutex));
        }

        /*while (pool->to_do_list != NULL) {



            while (pool->to_do_list != NULL) {
                if (pool->to_do_list->is_future && prev_future->ready == false) {
                    //if the element has to wait for its future answer, I'm carrying it to the end of list
                    printf("*************na koniec\n");
                    pool->end_of_list->next = pool->to_do_list;
                    pool->end_of_list = pool->to_do_list;
                    pool->to_do_list = pool->to_do_list->next;
                    pool->end_of_list->next = NULL;
                }
                else {
                    //else I'm invoking the function
                    printf("robię to!\n");
                    my_task = pool->to_do_list->task;
                    pool->to_do_list = pool->to_do_list->next;
                    sem_post(&(pool->mutex));
                    my_task.function(my_task.arg, my_task.argsz);
                    sem_wait(&(pool->mutex));
                }
            }

        }*/
        if (pool->alive == false) {
            sem_post(&(pool->mutex));
            return 0;
        }
        pool->free_threads++;
        sem_post(&(pool->mutex));
    }
}