#pragma once
#include "thread.h"

#define DEFAULT_THREAD_NUM			4
#define DEFAULT_QUEUE_NUM	65535

typedef unsigned long					atomic_uint_t;
typedef struct thread_task_s		thread_task_t;
typedef struct thread_pool_s		thread_pool_t;

struct  thread_task_s
{
	thread_task_t* next;
	uint_t     id;
	void* ctx;    //上下文
	void  (*handler)(void* data);   //函数指针
};

typedef struct {
	thread_task_t* first;
	thread_task_t** last;
}thread_pool_queue_t;

#define thread_pool_queue_init(q)  \
	(q)->first = NULL;   \
	(q)->last = &(q)->first

struct thread_pool_s
{
	pthread_mutex_t  mtx;
	thread_pool_queue_t queue;
	int_t  waiting;
	pthread_cond_t  cond;

	char* name;
	uint_t   threads;			//一个线程池的线程数量
	int_t  max_queue;			//最多容纳的任务数量
};

thread_task_t* thread_task_alloc(size_t size);
int_t thread_task_post(thread_pool_t* tp, thread_task_t* task);
thread_pool_t* thread_pool_init();
void thread_pool_destroy(thread_pool_t* tp);
