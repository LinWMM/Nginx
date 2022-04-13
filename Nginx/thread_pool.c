#include "thread_pool.h"

static void thread_pool_exit_handler(void* data);
static void* thread_pool_cycle(void* data);
static int_t  thread_pool_init_default(thread_pool_t* tpp, char* name);

static uint_t  thread_pool_task_id;

static int debug = 0;

thread_pool_t* thread_pool_init()
{
	int err;
	pthread_t  tid;
	uint_t n;
	pthread_attr_t  attr;
	thread_pool_t* tp = NULL;

	tp = calloc(1, sizeof(thread_pool_t));  //分配内存后清零
	if (tp == NULL)
	{
		fprintf(stderr, "thread_pool_init: calloc failed!\n");
		return NULL;
	}

	thread_pool_init_default(tp, NULL);
	thread_pool_queue_init(&tp->queue);
	
	if (thread_mutex_create(&tp->mtx) != OK)
	{
		free(tp);
		return NULL;
	}

	if (thread_cond_create(&tp->cond) != OK)
	{
		thread_mutex_destroy(&tp->mtx);
		free(tp);
		return NULL;
	}

	err = pthread_attr_init(&attr);
	if (err)
	{
		fprintf(stderr, "pthread_attr_init  failed : %s\n", strerror(errno));
		free(tp);
		return NULL;
	}

	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (err)
	{
		fprintf(stderr, "pthread_attr_setdetachstate  failed : %s\n", strerror(errno));
		free(tp);
		return NULL;
	}

	for (n = 0; n < tp->threads; n++)
	{
		err = pthread_create(&tid, &attr, thread_pool_cycle, tp);
		if (err)
		{
			fprintf(stderr, "pthread_create  failed : %s\n", strerror(errno));
			free(tp);
			return NULL;
		}
	}

	(void)pthread_attr_destroy(&attr);

	return tp;
}

thread_task_t* thread_task_alloc(size_t size)
{
	thread_task_t* task;

	task = calloc(1, sizeof(thread_task_t) + size);
	if (task == NULL)
		return NULL;
	task->ctx = task + 1;     //指向void*data

	return task;
}


int_t thread_task_post(thread_pool_t* tp, thread_task_t* task)
{
	if (thread_mutex_lock(&tp->mtx) != OK)
		return ERROR;

	if (tp->waiting == tp->max_queue)
	{
		(void)thread_mutex_unlock(&tp->mtx);
		fprintf(stderr, "thread_pool \"%s\" queue overflow: %id tasks waiting", tp->name, tp->waiting);
		return ERROR;
	}

	task->id = thread_pool_task_id++;
	task->next = NULL;

	if (thread_cond_signal(&tp->cond) != OK)    //有任务就发信号，通知线程处理。
	{
		(void)thread_mutex_unlock(&tp->mtx);
		return ERROR;
	}

	*tp->queue.last = task;				//尾部加上任务
	tp->queue.last = &task->next;

	tp->waiting++;
	(void)thread_mutex_unlock(&tp->mtx);

	if (debug)
		fprintf(stdout, "task #%lu added to thread pool \"%s\"\n", task->id, tp->name);

	return OK;
}


static int_t thread_pool_init_default(thread_pool_t* tpp, char* name)
{
	if (tpp)
	{
		tpp->threads = DEFAULT_THREAD_NUM;
		tpp->max_queue = DEFAULT_QUEUE_NUM;
		tpp->name = strdup(name ? name : "default");
		if (debug)
			fprintf(stdout, "thread_pool_init, name:%s, threads:%d, max_queue:%d", tpp->name, tpp->threads, tpp->max_queue);
		return OK;
	}

	return ERROR;
}


static void* thread_pool_cycle(void* data)
{
	thread_pool_t* tp = data;

	int err;
	thread_task_t* task;

	for (;;)
	{
		if (thread_mutex_lock(&tp->mtx) != OK)
			return NULL;

		tp->waiting--;   

		while (tp->queue.first == NULL)
		{
			if (thread_cond_wait(&tp->cond, &tp->mtx) != OK)
			{
				(void)thread_mutex_unlock(&tp->mtx);
				return NULL;
			}
		}

		task = tp->queue.first;			  
		tp->queue.first = task->next;

		if (tp->queue.first == NULL)
			tp->queue.last = &tp->queue.first;
	
		if (thread_mutex_unlock(&tp->mtx) != OK)
			return NULL;
		
		if (debug)
			fprintf(stdout, "run task #%lu in thread pool  \"%s\" \n", task->id, tp->name);

		task->handler(task->ctx);			//执行任务，阻塞等待

		if(debug)
			fprintf(stdout, "complete task #%lu in thread pool  \"%s\" \n", task->id, tp->name);

		task->next = NULL;

		//free(task);
	}
}

static void thread_pool_exit_handler(void* data)     //让任务自杀
{
	uint_t* lock = data;
	*lock = 0;

	pthread_exit(0);
}
void thread_pool_destroy(thread_pool_t* tp)
{
	uint_t   n;
	thread_task_t  task;
	volatile  uint_t lock;

	memset(&task, '\0', sizeof(thread_task_t));

	task.handler = thread_pool_exit_handler;
	task.ctx = (void*)&lock;

	for (n = 0; n < tp->threads; n++)
	{
		lock = 1;

		if (thread_task_post(tp, &task) != OK)
			return;

		while (lock)
			sched_yield();   
	}

	(void)thread_cond_destroy(&tp->cond);
	(void)thread_mutex_destroy(&tp->mtx);

	free(tp);
}
