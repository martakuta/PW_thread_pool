#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct runnable {
  void (*function)(void *, size_t);
  void *arg;
  size_t argsz;
} runnable_t;

typedef struct to_do_list tdl_t;

typedef struct to_do_list {
    runnable_t task;
    bool is_future;
    void* previous;
    tdl_t* next;
} tdl_t;

typedef struct thread_pool {
    sem_t sem;
    sem_t mutex;
    size_t size;
    size_t free_threads;
    pthread_t* threads;
    tdl_t* to_do_list;
    tdl_t* end_of_list;
    //fl_t* future_list;
    bool alive;
    int id;
} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

void *work_in_pool();

#endif
