all : test

test: test.c user_thread.c
	gcc -g -Wall -o $@ $^

clean :
	rm test
