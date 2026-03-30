#include <stdio.h>
#include <stdlib.h>
#include "array.h"
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

int array_init(array *s)
{
    s->top = 0;
    sem_init(&(s->empty), 0, STACK_SIZE);
    sem_init(&(s->full), 0, 0);
    return pthread_mutex_init(&s->lock, NULL);
}

int array_put(array *s, char *element)
{
    sem_wait(&(s->empty));
    pthread_mutex_lock(&s->lock);
    strlcpy(s->array[s->top], element, sizeof(char) * (MAX_WORD_SIZE));
    s->top += 1;
    pthread_mutex_unlock(&s->lock);
    sem_post(&(s->full));
    return 0;
}

int array_get(array *s, char **element)
{
    sem_wait(&(s->full));
    pthread_mutex_lock(&s->lock);
    s->top -= 1;
    strlcpy(*element, s->array[s->top], MAX_WORD_SIZE);
    pthread_mutex_unlock(&s->lock);
    sem_post(&(s->empty));
    return 0;
}
void array_free(array *s)
{
    s->top = 0;
    pthread_mutex_destroy(&s->lock);
    sem_destroy(&(s->empty));
    sem_destroy(&(s->full));
}