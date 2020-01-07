#include "future.h"
#include <stdlib.h>
#include <stdio.h>

typedef void *(*function_t)(void *);


void wrapper(future_t* future, size_t size) {
    printf("wrapper\n");
    callable_t c = future->callable;
    future->answer = c.function(c.arg, c.argsz, &future->ans_size);
    future->ready = true;
    //user can look on the answer because he has a reference to the "future"
}

int defer_future(struct thread_pool *pool, runnable_t runnable, future_t* from) {

    //printf("defer future %zd\n", pool->free_threads);
    sem_wait(&(pool->mutex));
    pool->task = runnable;
    if (pool->free_threads > 0 && from->ready) {
        pool->free_threads--;
        pool->task = runnable;
        sem_post(&(pool->sem)); //dziedziczenie mutex
    }
    else {
        tdl_t* t = (tdl_t*)malloc(sizeof(tdl_t));
        t->task = runnable;
        t->is_future = true;
        t->previous = from;
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

int async(thread_pool_t* pool, future_t *future, callable_t callable) {

    future->callable = callable;
    future->ready = false;

    runnable_t* runnable = (runnable_t*)malloc(sizeof(runnable_t));
    runnable->arg = future;
    runnable->argsz = callable.argsz;
    runnable->function = (void*)wrapper;

    defer(pool, *runnable);

    return 0;
}

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {
    //nie czeka az poprzedni sie skonczy wykonywac, ale dodaje zadanie do puli zadan
    // do zrobienia, gdy beda one mialy swoj wynik
    //dzieki temu nie blokuje maina wywolaniami map

    callable_t* callable = (callable_t*)malloc(sizeof(callable_t));
    callable->function = function;
    callable->arg = from->answer;
    callable->argsz = from->ans_size;

    future->callable = *callable;
    future->ready = false;

    runnable_t* runnable = (runnable_t*)malloc(sizeof(runnable_t));
    runnable->arg = future;
    runnable->argsz = callable->argsz;
    runnable->function = (void*)wrapper;

    defer_future(pool, *runnable, from);

    return 0;
}

void *await(future_t *future) {

    while (future->ready == false) {
        //active waiting
    }

    return future->answer;
}
