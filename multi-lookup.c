#include "multi-lookup.h"
#include "array.h"

static int resolve_ipv4_address(const char *hostname, char *ipv4, size_t ipv4_len)
{
    struct addrinfo hints;
    struct addrinfo *results = NULL;
    struct addrinfo *current = NULL;
    int status = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, NULL, &hints, &results);
    if (status != 0)
    {
        return -1;
    }

    for (current = results; current != NULL; current = current->ai_next)
    {
        struct sockaddr_in *address = (struct sockaddr_in *)current->ai_addr;
        if (inet_ntop(AF_INET, &(address->sin_addr), ipv4, ipv4_len) != NULL)
        {
            freeaddrinfo(results);
            return 0;
        }
    }

    freeaddrinfo(results);
    return -1;
}

void logfile_init(char filename[100], logfile *log_file)
{
    FILE *fptr = fopen(filename, "w");
    if (fptr == NULL)
    {
        fprintf(stderr, "Could not initialize log file\n");
        return;
    }

    log_file->fptr = fptr;

    pthread_mutex_init(&log_file->lock, NULL);
}

void logfile_free(logfile *log_file)
{
    fclose(log_file->fptr);
    pthread_mutex_destroy(&log_file->lock);
}

void logfile_append(logfile *log_file, char ipv4[IPV4_BUFFER_LENGTH], char original_name[50])
{
    pthread_mutex_lock(&log_file->lock);

    if (log_file->fptr == NULL)
    {
        fprintf(stderr, "Could not write to log file\n");
        pthread_mutex_unlock(&log_file->lock);
        return;
    }

    fprintf(log_file->fptr, "%s, %s\n", original_name, ipv4);

    pthread_mutex_unlock(&log_file->lock);
}

void *resolve_names(void *ptr)
{
    resolver_thread_args_t *args = (resolver_thread_args_t *)ptr;
    char * buffer = malloc(sizeof(char) * MAX_WORD_SIZE);
    char * ipv4 = malloc(sizeof(char) * IPV4_BUFFER_LENGTH);

    while (1)
    {;
        array_get(args->a, &buffer);

        if (strcmp(buffer, "STOP_SIGNAL") == 0)
        {
            break;
        }

        if (resolve_ipv4_address(buffer, ipv4, 50) != 0)
        {
            logfile_append(args->log, "NOT_RESOLVED", buffer);
        }
        else
        {
            logfile_append(args->log, ipv4, buffer);
        }
    }

    free(buffer);
    free(args);
    free(ipv4);
    return NULL;
}

void *producer(void *ptr)
{
    producer_thread_args_t *args = (producer_thread_args_t *)ptr;

    char buffer[MAX_WORD_SIZE];

    FILE *fptr = fopen(args->file_name, "r");
    if (fptr == NULL)
    {
        printf("Failed to open file on thread %d\n", args->thread_id);
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), fptr) != NULL)
    {
        buffer[strcspn(buffer, "\n\r")] = '\0';
        array_put(args->a, buffer);
    }

    free(args);
    fclose(fptr);
    return NULL;
}

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        printf("Nah\n");
        return -1;
    }

    int producer_thread_count = argc - 2;

    logfile* log_file = malloc(sizeof(logfile));
    logfile_init(argv[1], log_file);

    pthread_t threads[producer_thread_count];
    pthread_t consumer_threads[CONSUMER_COUNT];

    array a;

    array_init(&a);

    // producer threads
    for (int i = 0; i < producer_thread_count; i++)
    {
        producer_thread_args_t *args = malloc(sizeof(producer_thread_args_t));
        args->a = &a;
        strlcpy(args->file_name, argv[i + 2], sizeof(args->file_name));
        args->thread_id = i;
        if (pthread_create(&threads[i], NULL, producer, args) != 0)
        {
            printf("Failed to create thread %d\n", i);
        }
    }

    printf("Created producer threads\n");

    // consumer threads
    for (int i = 0; i < CONSUMER_COUNT; i++)
    {
        resolver_thread_args_t *args = malloc(sizeof(resolver_thread_args_t));
        args->a = &a;
        args->log = log_file;
        args->thread_id = i;
        if (pthread_create(&consumer_threads[i], NULL, resolve_names, args) != 0)
        {
            printf("Failed to create consumer thread %d\n", i);
        }
    }

    printf("Created consumer threads\n");

    for (int i = 0; i < producer_thread_count; i++)
    {
        pthread_join(threads[i], NULL);
        printf("Thread %d has finished producing\n", i);
    }

    char * stop_signal = "STOP_SIGNAL";

    for (int i = 0; i < CONSUMER_COUNT; i++)
    {
        array_put(&a, stop_signal);
    }

    printf("HELLO\n");

    for (int i = 0; i < CONSUMER_COUNT; i++)
    {
        pthread_join(consumer_threads[i], NULL);
        printf("Thread %d has finished.\n", i);
    }

    array_free(&a);

    logfile_free(log_file);
    free(log_file);

    return 0;
}