/**
 * This file provides the implementation of the functions defined in lab.h. It includes
 * thread-safe operations for elements in a queue. 
 * 
 * @author Eric Johnson
 */

#include "lab.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Node structure for the linked list.
 */
typedef struct node
{
    void *data;
    struct node *next;
} node_t;

/**
 *  Thread-safe queue structure.
 */
struct queue
{
    node_t *front;
    node_t *rear;
    int capacity;
    int size;
    bool shutdown;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

/**
 * Initializes a queue with specified capacity.
 * 
 * @param capacity The maximum capacity.
 * @return Pointer to the initialized queue.
 */
queue_t queue_init(int capacity)
{
    if (capacity <= 0) {
        fprintf(stderr, "Error: Invalid queue size.\n");
        exit(EXIT_FAILURE);
    }

    queue_t q = (queue_t)malloc(sizeof(struct queue));
    if (!q)
    {
        perror("Failed to allocate queue");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    q->capacity = capacity;
    q->size = 0;
    q->shutdown = false;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
    return q;
}

/**
 * Destroys queue and frees all allocated memory.
 * 
 * @param q Pointer to the queue to be destroyed.
 */
void queue_destroy(queue_t q)
{
    if (!q)
        return;

    pthread_mutex_lock(&q->lock);
    q->shutdown = true;
    pthread_cond_broadcast(&q->not_empty);
    pthread_cond_broadcast(&q->not_full);
    pthread_mutex_unlock(&q->lock);

    while (q->front)
    {
        node_t *temp = q->front;
        q->front = q->front->next;
        free(temp->data);
        free(temp);
    }

    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
    free(q);
}

/**
 * Enqueues data into the queue.
 * 
 * @param q Pointer to the queue.
 * @param data Pointer to the data to be enqueued.
 */
void enqueue(queue_t q, void *data)
{
    if (!q || !data)
        return;

    pthread_mutex_lock(&q->lock);

    while (q->size == q->capacity && !q->shutdown)
    {
        pthread_cond_wait(&q->not_full, &q->lock);
    }

    if (q->shutdown)
    {
        pthread_mutex_unlock(&q->lock);
        return;
    }

    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (!new_node)
    {
        perror("Failed to allocate node");
        pthread_mutex_unlock(&q->lock);
        exit(EXIT_FAILURE);
    }
    new_node->data = data;
    new_node->next = NULL;

    if (q->rear)
    {
        q->rear->next = new_node;
    }
    else
    {
        q->front = new_node;
    }
    q->rear = new_node;
    q->size++;

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

/**
 * Dequeues data from the queue.
 * 
 * @param q Pointer to the queue.
 * @return Pointer to the dequeued data.
 */
void *dequeue(queue_t q)
{
    if (!q)
        return NULL;

    pthread_mutex_lock(&q->lock);

    while (q->size == 0 && !q->shutdown)
    {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    if (q->shutdown && q->size == 0)
    {
        pthread_mutex_unlock(&q->lock);
        return NULL;
    }

    node_t *temp = q->front;
    void *data = temp->data;
    q->front = q->front->next;
    if (!q->front)
    {
        q->rear = NULL;
    }
    free(temp);
    q->size--;

    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);

    return data;
}

/**
 * Peeks at the front of the queue without removing it.
 * 
 * @param q Pointer to the queue.
 * @return Pointer to the data at the front of the queue.
 */
void queue_shutdown(queue_t q)
{
    if (!q)
        return;

    pthread_mutex_lock(&q->lock);
    q->shutdown = true;
    pthread_cond_broadcast(&q->not_empty);
    pthread_cond_broadcast(&q->not_full);
    pthread_mutex_unlock(&q->lock);
}

/**
 *  Checks if queue is empty.
 * 
 * @param q The queue.
 * @return True if empty, else false.
 */
bool is_empty(queue_t q)
{
    if (!q)
        return true;

    pthread_mutex_lock(&q->lock);
    bool empty = (q->size == 0);
    pthread_mutex_unlock(&q->lock);

    return empty;
}

/**
 *  Checks if queue is in shutdown mode.
 * 
 * @param q The queue.
 * @return True if in shutdown, else false.
 */
bool is_shutdown(queue_t q)
{
    if (!q)
        return true;

    pthread_mutex_lock(&q->lock);
    bool shutdown = q->shutdown;
    pthread_mutex_unlock(&q->lock);

    return shutdown;
}