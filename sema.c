#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

/* If expr is false, print error message and exit. */
#define CHECK(expr, msg)                        \
    do {                                        \
        if (!(expr)) {                          \
            perror(msg);                        \
            exit(EXIT_FAILURE);                 \
        }                                       \
    } while (0)


/* Thread-safe queues */

#define MAX 10                  /* maximum number of characters in queue */

typedef struct {
    char buffer[MAX];           /* circular buffer */
    size_t in;                  /* position of producer in buffer */
    size_t out;                 /* position of consumer in buffer */
    size_t count;               /* number of unconsumed items */
    pthread_mutex_t mutex;      /* mutex protecting buffer, in, out, count */
    sem_t *empty;               /* semaphore counting empty slots in buffer */
    sem_t *full;                /* semaphore counting full slots in buffer */
} queue_s, *queue_t;

void queue_init(queue_t q)
{
    q->count = 0;
    q->in = 0;
    q->out = 0;

    CHECK(pthread_mutex_init(&q->mutex, NULL) == 0, "pthread_mutex_init");
    q->full = sem_open("full", O_CREAT, 0600, 0);
    CHECK(q->full != SEM_FAILED, "sem_open");
    CHECK(sem_unlink("full") == 0, "sem_unlink");
    q->empty = sem_open("empty", O_CREAT, 0600, MAX);
    CHECK(q->empty != SEM_FAILED, "sem_open");
    CHECK(sem_unlink("empty") == 0, "sem_unlink");
}

void queue_destroy(queue_t q)
{
    CHECK(pthread_mutex_destroy(&q->mutex) == 0, "pthread_mutex_destroy");
    CHECK(sem_close(q->full) == 0, "sem_close");
    CHECK(sem_close(q->empty) == 0, "sem_close");
}

/* Wait, if necessary, for a slot to become available in the queue,
 * and then append the character c. */
void queue_append(queue_t q, char c)
{
    CHECK(sem_wait(q->empty) == 0, "sem_wait");
    CHECK(pthread_mutex_lock(&q->mutex) == 0, "pthread_mutex_lock");

    q->buffer[q->in] = c;
    q->in++;
    q->in %= MAX;
    assert(q->count < MAX);
    q->count++;

    CHECK(pthread_mutex_unlock(&q->mutex) == 0, "pthread_mutex_unlock");
    CHECK(sem_post(q->full) == 0, "sem_post");
}

/* Wait, if necessary, for a slot to become full in the queue, and
 * then pop and return the first character. */
char queue_pop(queue_t q)
{
    CHECK(sem_wait(q->full) == 0, "sem_wait");
    CHECK(pthread_mutex_lock(&q->mutex) == 0, "pthread_mutex_lock");

    char val = q->buffer[q->out];
    q->out++;
    q->out %= MAX;
    assert(q->count > 0);
    q->count--;
    assert(q->count == (q->in + MAX - q->out) % MAX);

    CHECK(pthread_mutex_unlock(&q->mutex) == 0, "pthread_mutex_unlock");
    CHECK(sem_post(q->empty) == 0, "sem_post");
    return val;
}


/* Test harness */

queue_s queue;
char substring[5];

void *producer(void *arg)
{
    char array[] = "HelloWorld";
    size_t i = 0;
    printf("Producer Thread Created\n");
    while (array[i] != '\0') {
        queue_append(&queue, array[i]);
        i++;
    }
    return NULL;
}

void *consumer(void *arg)
{
    size_t i = 0;
    printf("Consumer Thread Created\n");
    for (i = 0; i < 5; i++) {
        substring[i] = queue_pop(&queue);
    }
    return NULL;
}

int main()
{
    pthread_t producer_thread, consumer_thread;
    queue_init(&queue);
    CHECK(pthread_create(&consumer_thread, NULL, consumer, NULL) == 0,
          "pthread_create");
    CHECK(pthread_create(&producer_thread, NULL, producer, NULL) == 0,
          "pthread_create");
    CHECK(pthread_join(consumer_thread, NULL) == 0, "pthread_join");
    CHECK(pthread_join(producer_thread, NULL) == 0, "pthread_join");
    printf("Next character in queue: %c\n", queue.buffer[queue.out]);
    printf("substring: %s\n", substring);
    queue_destroy(&queue);
    return EXIT_SUCCESS;
}
