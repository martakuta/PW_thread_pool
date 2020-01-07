#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "threadpool.h"
#include "future.h"

//void (*function)(void *, size_t);
void text(const char* s, size_t length) {
    //printf("text: ");
    for (size_t i = 0; i < length; i++)
        printf("%c", *(s+i));
    //printf("\n");
}

void number(int a, size_t size) {

    //printf("number: ");
    printf("%d", a);
}

void power(int a, size_t size) {

    //printf("power: %d^2 = ", a);
    printf("%d", a*a);
}

void fun1(int a, size_t size) {

    usleep(1000);
    printf("1 ");
}
void fun2(int a, size_t size) {
    usleep(100);
    printf("2 ");
}
void fun3(int a, size_t size) {

    usleep(100);
    printf("3 ");
}

int dodaj1(int a, size_t size, size_t* ret_size) {

    usleep(100);
    printf("%d ", a+1);
    *ret_size = 1;
    return a+1;
}

int main(int argc, char* argv[]) {

    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, 5);

    runnable_t* task1 = (runnable_t*)malloc(sizeof(runnable_t));
    task1->function = (void*)text;
    task1->arg = "A";
    task1->argsz = strlen(task1->arg);

    runnable_t* task2 = (runnable_t*)malloc(sizeof(runnable_t));
    task2->function = (void*)number;
    task2->arg = (void*)3;
    task2->argsz = 1;

    runnable_t* task3 = (runnable_t*)malloc(sizeof(runnable_t));
    task3->function = (void*)power;
    task3->arg = (void*)3;
    task3->argsz = 1;

    runnable_t* task4 = (runnable_t*)malloc(sizeof(runnable_t));
    task4->function = (void*)fun1;
    task4->arg = (void*)5;
    task4->argsz = 1;

    runnable_t* task5 = (runnable_t*)malloc(sizeof(runnable_t));
    task5->function = (void*)fun2;
    task5->arg = (void*)8;
    task5->argsz = 1;

    runnable_t* task6 = (runnable_t*)malloc(sizeof(runnable_t));
    task6->function = (void*)fun3;
    task6->arg = (void*)9;
    task6->argsz = 1;

    callable_t* fut1 = (callable_t*)malloc(sizeof(callable_t));
    fut1->function = (void*)dodaj1;
    fut1->arg = (void*)(99);
    fut1->argsz = 1;
    future_t** future_tab = malloc(51*sizeof(future_t*));

    for (int i = 0; i < 51; i++)
        future_tab[i] = malloc(sizeof(future_t));

    sleep(3);

    async(pool, future_tab[0], *fut1);

    await(future_tab[0]);

    for (int i = 1; i <= 5; i++) {
        defer(pool, *task4);
        defer(pool, *task5);
        defer(pool, *task6);
        //fut1->arg = (void*)i;
        map(pool, future_tab[i], future_tab[i-1], (void*)dodaj1);
        //await(future_tab[i]);
    }
    printf("defered everything\n");
    sleep(3);
    thread_pool_destroy(pool);
    printf("Main thread exiting.\n");

    return 0;
}

