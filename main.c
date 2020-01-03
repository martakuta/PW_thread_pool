#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "threadpool.h"

int main(int argc, char* argv[]) {

    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, 5);

    for (int i = 0; i < 7; i++)
        sem_post(&(pool->sem));
    sleep(3);

    thread_pool_destroy(pool);
    printf("Main thread exiting.\n");

    return 0;
}

