#ifndef FUTURE_H
#define FUTURE_H

#include "threadpool.h"


typedef struct callable {
  void *(*function)(void *, size_t, size_t *);
  void *arg;
  size_t argsz;
} callable_t;

typedef struct future {
    callable_t callable;
    void* answer;
    size_t ans_size;
    bool ready;
} future_t;

typedef struct future_list fl_t;

typedef struct future_list {
    future_t* future;
    future_t* from;
    void *(*function)(void *, size_t, size_t *);
    fl_t* next;
} fl_t;


int async(thread_pool_t *pool, future_t *future, callable_t callable);

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *));

void *await(future_t *future);

#endif
