#include <pthread.h>
#include <semaphore.h>

#pragma once

#define STACK_SIZE 8 // max elements in stack
#define MAX_WORD_SIZE 100

typedef struct
{
    char array[STACK_SIZE][MAX_WORD_SIZE]; // storage array for integers
    int top;                               // array index indicating where the top is
    pthread_mutex_t lock;
    sem_t full;
    sem_t empty;
} array;

int array_init(array *s);                // initialise the array
int array_put(array *s, char *element);  // place element on the top of the array
int array_get(array *s, char **element); // remove element from the top of the array
void array_free(array *s);               // free the stack's resources