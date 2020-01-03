#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <semaphore.h>
#include <pthread.h>

typedef struct runnable {
  void (*function)(void *, size_t);
  void *arg;
  size_t argsz;
} runnable_t;

typedef struct thread_pool {
    sem_t sem;
    size_t size;
    pthread_t* threads;
} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

void *work_in_pool();

#endif
