#include "thread_pool.h"

void task_handler1(void* data)
{
	static int index = 0;
	printf("Hello, this is first test.index=%d\n", index++);
}

void task_handler2(void* data)
{
	static int index = 0;
	printf("Hello, this is second test.index=%d\n", index++);
}

int main()
{
	thread_pool_t* tp = NULL;

	thread_task_t* test1= thread_task_alloc(0);
	thread_task_t* test2 = thread_task_alloc(0);

	test1->handler = task_handler1;
	test2->handler = task_handler2;
	
	tp = thread_pool_init();

	thread_task_post(tp, test1);
	thread_task_post(tp, test2);
	
	fflush(NULL);
	sleep(10);
	thread_pool_destroy(tp);
}
