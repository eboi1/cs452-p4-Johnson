#include "lab.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node
{
    void *data;
    struct node *next;
} node_t;

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

queue_t queue_init(int capacity)
{
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

bool is_empty(queue_t q)
{
    if (!q)
        return true;

    pthread_mutex_lock(&q->lock);
    bool empty = (q->size == 0);
    pthread_mutex_unlock(&q->lock);

    return empty;
}

bool is_shutdown(queue_t q)
{
    if (!q)
        return true;

    pthread_mutex_lock(&q->lock);
    bool shutdown = q->shutdown;
    pthread_mutex_unlock(&q->lock);

    return shutdown;
}