LDFLAGS+=-pthread
CFLAGS+=-pthread

all:main

main:main.o thread_pool.o thread_cond.o  thread_mutex.o
	$(CC) $^ -o $@ $(LDFALGS) $(CFLAGS)


clean:
	rm -rf *.o main
