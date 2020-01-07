#include <stdio.h>
#include <stdlib.h>
#include "future.h"

typedef struct para {
    int wyn;
    int ost;
} para;

para* silnia(para* p, size_t size, size_t* ret_size) {

    para* ret = malloc(sizeof(para));
    ret->ost = p->ost + 1;
    ret->wyn = p->wyn * ret->ost;

    *ret_size = sizeof(para);
    return ret;
}

int main(int argc, char* argv[]) {

    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, 3);

    int n;
    scanf("%d", &n);

    future_t** future_tab = malloc(n*sizeof(future_t*));
    for (int i = 0; i < n; i++) {
        future_tab[i] = malloc(sizeof(future_t));
    }

    para* p = malloc(sizeof(para));
    p->wyn = 1;
    p->ost = 1;
    callable_t* fut1 = (callable_t*)malloc(sizeof(callable_t));
    fut1->function = (void*)silnia;
    fut1->arg = (void*)p;
    fut1->argsz = sizeof(para);

    future_tab[0]->answer = (void*)p;
    future_tab[0]->ready = true;
    future_tab[0]->ans_size = sizeof(para);

    for (int i = 1; i < n; i++) {
        map(pool, future_tab[i], future_tab[i-1], (void*)silnia);
    }

    await(future_tab[n-1]);

    para* wynik = future_tab[n-1]->answer;
    printf("%d\n", wynik->wyn);

    thread_pool_destroy(pool);
    for (int i = 0; i < n; i++)
        free(future_tab[i]);
    free(future_tab);
    free(pool);
    free(fut1);
}