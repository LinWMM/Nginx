#include "thread.h"

int thread_mutex_create(pthread_mutex_t* mtx)
{
	int err;
	pthread_mutexattr_t  attr;

	err = pthread_mutexattr_init(&attr);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutexattr_init error: %s\n", strerror(errno));
		return ERROR;
	}

	err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutexattr_settype error: %s\n", strerror(errno));
		return ERROR;
	}

	err = pthread_mutex_init(mtx, &attr);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutex_init error: %s\n", strerror(errno));
		return ERROR;
	}

	err = pthread_mutexattr_destroy(&attr);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutexattr_destroy error: %s\n", strerror(errno));
	}
	
	return OK;
}

int thread_mutex_lock(pthread_mutex_t* mtx)
{
	int err;

	err = pthread_mutex_lock(mtx);
	if (err == 0)
		return OK;
	fprintf(stderr, "thread_mutex_lock error: %s\n", strerror(errno));
	return ERROR;
}
int thread_mutex_unlock(pthread_mutex_t* mtx)
{
	int err;

	err = pthread_mutex_unlock(mtx);
	if (err == 0)    
		return OK;
	fprintf(stderr, "thread_mutex_unlock error: %s\n", strerror(errno));
	return ERROR;
}
int thread_mutex_destroy(pthread_mutex_t* mtx)
{
	int err;

	err = pthread_mutex_destroy(mtx);
	if (err == 0)
		return OK;
	fprintf(stderr, "thread_mutex_destroy error: %s\n", strerror(errno));
	return ERROR;
}
