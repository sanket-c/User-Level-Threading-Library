mythread.a: mythread.o queue.o
	ar rcs mythread.a mythread.o queue.o 
	
mythread.o: mythread.c mythread.h threadutil.h queue.c queue.h
	gcc -c mythread.c -o mythread.o

queue.o: queue.c queue.h
	gcc -c queue.c -o queue.o
