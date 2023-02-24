#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "threadpool.h"

void *worker_task(void *arg)
{
    threadpool *tp = (threadpool *)arg;
    task t;

    while (1) {
        pthread_mutex_lock(&tp->lock);

        // Wait for a task to be avalable (using a cond variable and a spinlock)
        while (tp->task_count == 0 && !tp->is_shutdown) {
            pthread_cond_wait(&tp->not_empty, &tp->lock);
        }

        // Check if threadpool is shutdown
        if (tp->is_shutdown) {
            pthread_mutex_unlock(&tp->lock);
            pthread_exit(NULL);
        }
        
        // Select a task when it is found
        t = tp->tasks[tp->head];
        tp->head = (tp->head+1) % tp->size;
        tp->task_count -= 1;

         // Signal that there is space available in the queue
        pthread_cond_signal(&tp->not_full);

        // Unlock the mutex
        pthread_mutex_unlock(&tp->lock);

        // Execute the task
        (*(t.function))(t.argument);
    }
}

threadpool *threadpool_create(int max_task, int num_threads)
{
    threadpool *tp = (threadpool *)malloc(sizeof(threadpool));
    tp->head = tp->tail = tp->task_count = 0;
    tp->size = max_task; tp->thread_count = num_threads;
    
    pthread_mutex_init(&tp->lock, NULL);
    pthread_cond_init(&tp->not_empty, NULL);
    pthread_cond_init(&tp->not_full, NULL);

    tp->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    tp->tasks   = (task *)malloc((sizeof(task) * tp->size));
    for (int i = 0; i < tp->thread_count; ++i) {
        pthread_create(&tp->threads[i], NULL, worker_task, (void *)tp);
    }

    return tp;
}

void add_task(threadpool *tp, int count,  ...)
{
    va_list args;
    
    va_start(args, count);

    for (int i = 0; i < count; ++i) {
       
        task t = va_arg(args, task);
        
        pthread_mutex_lock(&tp->lock);
        // In caz ca e ful se asteapta
        tp->tasks[tp->task_count++] = t;
        // S a adaugat in coada un nou task
        pthread_cond_signal(&tp->not_empty);
        
        pthread_mutex_unlock(&tp->lock);
    }

    va_end(args);
}

void threadpool_destory(threadpool *tp)
{
    pthread_mutex_lock(&tp->lock);
    tp->is_shutdown = 1;

    pthread_cond_broadcast(&tp->not_empty);
    pthread_cond_broadcast(&tp->not_full);

    pthread_mutex_unlock(&tp->lock);

    for (int i = 0; i < tp->thread_count; ++i) {
        pthread_join(tp->threads[i], NULL);
    }

    free(tp);
}



