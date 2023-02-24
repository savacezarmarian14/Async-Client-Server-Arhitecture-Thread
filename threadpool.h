#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

typedef struct {
    void (*function)(void *);
    void *argument;
} task;

typedef struct {
    task *tasks;
    int head;
    int tail;
    int task_count;
    int size;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    pthread_t *threads;
    int thread_count;
    int is_shutdown;
} threadpool;

void *worker_task(void *);
threadpool *threadpool_create(int, int);
void add_task(threadpool *, int, ...);
void threadpool_destory(threadpool *);


#endif