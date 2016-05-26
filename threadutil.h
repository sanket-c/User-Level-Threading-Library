#ifndef THREADUTIL_H_INCLUDED
#define THREADUTIL_H_INCLUDED

#include <ucontext.h>
#include "queue.h"

int READY_STATUS = 0;
int RUNNING_STATUS = 1;
int BLOCKED_STATUS = 2;
int KILLED_STATUS = -1;

/**
        tId is thread ID
        pId is thread ID of parent
        threadContext will hold the context of the thread
        parentThread will hold the parent of the thread
        childrenQueue will hold the active children of the thread.
        (once the child thread is terminated it will be removed from this queue
        as it is no longer active)
        blockedOnThreads will hold the list of threads (its children) it is waiting for
        when it is in blocked state
        status will hold the current status of thread
*/
typedef struct thread
{
    int tId;
    int pId;
    struct thread *parentThread;
	ucontext_t threadContext;
    Queue *childrenQueue;
	Queue *blockedOnThreads;
	int status; // ready : 0, running : 1, blocked : 2, killed : -1
}Thread;

/**
    initialValue will hold the initial value for number of semaphores
    waitingThreads will hold the threads waiting on this semaphore
*/
typedef struct semaphore
{
	int initialValue;
	Queue *waitingThreads;
}Semaphore;

#endif // THREAD_H_INCLUDED
