#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include "array.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>

#define SEM_MAX 10
#define IPV4_BUFFER_LENGTH 50
#define CONSUMER_COUNT 50
#define FILENAME_LENGTH 50

typedef struct
{
    FILE *fptr;
    pthread_mutex_t lock;

} logfile;

typedef struct
{
    int thread_id;
    char file_name[FILENAME_LENGTH];
    array *a;
} producer_thread_args_t;

typedef struct
{
    int thread_id;
    logfile *log;
    array *a;

} resolver_thread_args_t;