#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "future.h"

typedef struct pole {
    int val;
    unsigned int time;
} pole;

int macierz(pole* p, size_t size, size_t* ret_size) {

    usleep(p->time);
    *ret_size = sizeof(int);

    return p->val;
}

int main(int argc, char* argv[]) {

    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, 4);

    int k, n;
    scanf("%d%d", &k, &n);

    pole** pole_tab = malloc(n*k* sizeof(pole));
    for (int i = 0; i < n*k; i++) {
        pole_tab[i] = malloc(sizeof(pole));
        scanf("%d%d", &pole_tab[i]->val, &pole_tab[i]->time);
    }

    future_t** future_tab = malloc(n*sizeof(future_t*));
    for (int i = 0; i < n; i++) {
        future_tab[i] = malloc(sizeof(future_t));
    }

    callable_t* fut1 = (callable_t*)malloc(sizeof(callable_t));
    fut1->function = (void*)macierz;
    fut1->arg = (void*)pole_tab[0];
    fut1->argsz = sizeof(pole);

    for (int i = 0; i < k; i++) {
        int suma = 0;
        for (int j = i*n; j < (i+1)*n; j++) {
            fut1->arg = (void*)pole_tab[j];
            async(pool, future_tab[j - i*n], *fut1);
        }
        await(future_tab[n - 1]);

        for (int j=0; j<n; j++) {
            suma += (int)future_tab[j]->answer;
        }

        printf("%d\n", suma);
    }

    thread_pool_destroy(pool);
    for (int i = 0; i < n; i++)
        free(future_tab[i]);
    for (int i = 0; i < n*k; i++)
        free(pole_tab[i]);
    free(future_tab);
    free(pole_tab);
    free(pool);
    free(fut1);
}