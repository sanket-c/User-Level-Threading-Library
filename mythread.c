#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "mythread.h"
#include "queue.h"
#include "threadutil.h"
#define MEMORY 8192 //8Kb memory

ucontext_t *mainThreadContext;
Queue *readyQueue, *blockedQueue;
Thread *runningThread;
int thread_count = 0;

/**
        Get next thread from ready queue.
        If ready queue is empty stop (resume main context).
*/
Thread* getNextThreadFromReadyQueue()
{
    Thread *next = NULL;
    if(getLength(readyQueue) > 0)
    {
        next = (Thread *)dequeue(readyQueue)->data;
    }
    else
    {
        setcontext(mainThreadContext);
    }
	return next;
}

/**
        Initialize MyThread, ready queue and blocked queue.
        Create Main Thread and Main Thread Context.
        Also creates a current thread to keep track of currently running thread.
        Initially main thread is equal to current thread.
*/
void MyThreadInit(void(*start_funct)(void *), void *args)
{
    //printf("In MyThreadInit..!!\n");
    readyQueue = malloc(sizeof(Queue));
	initializeQueue(readyQueue);

	blockedQueue = malloc(sizeof(Queue));
	initializeQueue(blockedQueue);

    mainThreadContext = (ucontext_t *)malloc(sizeof(ucontext_t));
    getcontext(mainThreadContext);

	ucontext_t *runningThreadContext = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(runningThreadContext);
    runningThreadContext->uc_stack.ss_sp = malloc(MEMORY);
	runningThreadContext->uc_stack.ss_size = MEMORY;
    runningThreadContext->uc_link = NULL;
	makecontext(runningThreadContext, (void *) start_funct, 1, args);

    thread_count++;
    runningThread = malloc(sizeof(Thread));
    runningThread->tId = thread_count;
	runningThread->childrenQueue = malloc(sizeof(Queue));
	initializeQueue(runningThread->childrenQueue);
	runningThread->blockedOnThreads = malloc(sizeof(Queue));
	initializeQueue(runningThread->blockedOnThreads);
	runningThread->pId = 0; // first thread so no parent
	runningThread->parentThread = NULL; // first thread so no parent
	runningThread->threadContext = *runningThreadContext;
	runningThread->status = RUNNING_STATUS;

	swapcontext(mainThreadContext, &(runningThread->threadContext));
	//printf("Exiting MyThreadInit..!!\n");
}

/**
        Creates new thread and adds it to ready queue.
        Also the created thread is added to childQueue of current thread
        as current thread will be the parent of newly created thread.
*/
MyThread MyThreadCreate(void(*start_funct)(void *), void *args)
{
    //printf("In MyThreadCreate..!!\n");
	ucontext_t *contextForThread = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(contextForThread);
    contextForThread->uc_stack.ss_sp = malloc(MEMORY);
	contextForThread->uc_stack.ss_size = MEMORY;
    contextForThread->uc_link = NULL;
	makecontext(contextForThread, (void *) start_funct, 1, args);

    thread_count++;
	Thread *thread = malloc(sizeof(Thread));
	thread->tId = thread_count;
	thread->childrenQueue = malloc(sizeof(Queue));
	initializeQueue(thread->childrenQueue);
	thread->blockedOnThreads = malloc(sizeof(Queue));
	initializeQueue(thread->blockedOnThreads);
	thread->pId = runningThread->tId;
	thread->parentThread = runningThread;
	thread->threadContext = *contextForThread;
	thread->status = READY_STATUS;

	enqueue(runningThread->childrenQueue, thread);
	enqueue(readyQueue, thread);
	//printf("Exiting MyThreadCreate..!!\n");
	return (void *)thread;
}

/**
        Joins the thread with invoking thread.
        i.e. invoking thread will be blocked until the given thread is terminated.
        Next thread will start running.
        If the given thread is not the immediate child of invoking thread join fails
        and invoking thread continues to run.
*/
int MyThreadJoin(MyThread thread)
{
    //printf("In MyThreadJoin..!!\n");
    if(thread != NULL)
    {
        Thread *child = (Thread *)thread;
        Thread *parent = child->parentThread;

        if(parent == runningThread) // if invoking thread is immeidate parent of given thread
        {
            if(ifExist(parent->childrenQueue, child) == 1) //if child is still active i.e. not killed
            {
                enqueue(parent->blockedOnThreads, child);
                parent->status = BLOCKED_STATUS;
                enqueue(blockedQueue, parent);
                runningThread = getNextThreadFromReadyQueue();
                runningThread->status = RUNNING_STATUS;
                swapcontext(&(parent->threadContext), &(runningThread->threadContext));
            }
        }
        else
        {
            //printf("Failure : Given thread is not an immediate child of invoking thread.\n");
            //printf("Cannot join given thread. Invoking thread will continue running.\n");
            return -1;
        }
        //printf("Exiting MyThreadJoin..!!\n");
        return 0;
    }
    else
    {
        printf("Error : Given thread is NULL.\n");
        printf("Cannot join given thread.\n");
        return -1;
    }
}

/**
        Joins all the children of invoking thread with it.
        i.e. invoking thread will be blocked until all its children are terminated.
        Next thread will start running.
        If invoking thread has no children, invoking thread will continue running.
*/
void MyThreadJoinAll(void)
{
    //printf("In MyThreadJoinAll..!!\n");
    if(getLength(runningThread->childrenQueue) > 0) // if invoking thread has children block on all of them
    {
		Thread *thread = runningThread;
        runningThread = getNextThreadFromReadyQueue();
		copyQueue(thread->childrenQueue, thread->blockedOnThreads); // copyQueue(source, destination)
		thread->status = BLOCKED_STATUS;
		enqueue(blockedQueue, thread);
		runningThread->status = RUNNING_STATUS;
		swapcontext(&(thread->threadContext), &(runningThread->threadContext));
	}
	//printf("Exiting MyThreadJoinAll..!!\n");
}

/**
        Current thread is suspended and added at the end of the ready queue.
        Next thread starts running.
        If there is no thread in ready queue, current thread will continue to run.
*/
void MyThreadYield()
{
    //printf("In MyThreadYield..!!\n");
    if(getLength(readyQueue) > 0) // if ready queue has other threads, yield to first thread
    {
        Thread *thread = runningThread;
        runningThread = getNextThreadFromReadyQueue();
        thread->status = READY_STATUS;
        enqueue(readyQueue, thread);
        runningThread->status = RUNNING_STATUS;
        swapcontext(&(thread->threadContext), &(runningThread->threadContext));
    }
    //printf("Exiting MyThreadYield..!!\n");
}

/**
        exits the thread.
        frees the memory used by the thread, updates the parent of the active children
        and notifies the waiting thread.
        Next thread will start running once the current thread is exited.
*/
void MyThreadExit(void)
{
    //printf("In MyThreadExit..!!\n");
    Thread *thread = runningThread;
    thread->status = KILLED_STATUS;

	// if exiting thread have children they will be orphaned
	if(getLength(thread->childrenQueue) > 0)
	{
        Element *node = thread->childrenQueue->head;
        do
        {
            ((Thread *)node->data)->parentThread = NULL;

            node = node->next;
        }while(node != thread->childrenQueue->head);
	}

    // notify blocked thread
    Thread *parent = thread->parentThread;

	if(parent != NULL) // if parent has not been killed
	{
		if(getLength(blockedQueue) > 0 && ifExist(blockedQueue, parent) == 1 && ifExist(parent->blockedOnThreads, thread) == 1)
        {
            if(getLength(parent->blockedOnThreads) == 1) //waiting for exiting thread only
            {
                removeElement(blockedQueue, parent);
                removeElement(parent->blockedOnThreads, thread);
                parent->status = READY_STATUS;
                enqueue(readyQueue, parent);
            }
            else if(getLength(parent->blockedOnThreads) > 1) //waiting for more threads
            {
                removeElement(parent->blockedOnThreads, thread);
            }
        }
        removeElement(parent->childrenQueue, thread); // removing child thread from parent as it is exiting
    }

    // free thread memory
	thread->parentThread = NULL;
	free(thread->blockedOnThreads);
	thread->blockedOnThreads = NULL;
	free(thread->childrenQueue);
	thread->childrenQueue = NULL;
	free((thread->threadContext).uc_stack.ss_sp);
	free(thread);

	runningThread = getNextThreadFromReadyQueue();
	runningThread->status = RUNNING_STATUS;
    //printf("Exiting MyThreadExit..!!\n");
	setcontext(&(runningThread->threadContext));
}

/**
        Initialize Semaphore with given initial value
*/
MySemaphore MySemaphoreInit(int initialValue)
{
    Semaphore *semaphore = malloc(sizeof(Semaphore));
    if(initialValue < 0)
    {
        printf("Error : Initial value should be non negative.\n");
        printf("Returning NULL.\n");
        return NULL;
    }
	semaphore->initialValue = initialValue;
	semaphore->waitingThreads = malloc(sizeof(Queue));
	initializeQueue(semaphore->waitingThreads);
	return (void *) semaphore;
}

/**
        Signals the waiting threads
*/
void MySemaphoreSignal(MySemaphore sem)
{
    if(sem != NULL)
    {
        Semaphore *semaphore = (Semaphore *) sem;

        (semaphore->initialValue)++;

        if(semaphore->initialValue <= 0 && getLength(semaphore->waitingThreads) > 0)
        {
            Thread *thread = (Thread *)dequeue(semaphore->waitingThreads)->data;
            thread->status = 0;
            enqueue(readyQueue, thread);
        }
    }
    else
    {
        printf("Error : Given semaphore is NULL.\n");
        printf("Cannot signal the waiting threads.\n");
        exit(0);
    }
}

/**
        Waits on the given semaphore
*/
void MySemaphoreWait(MySemaphore sem)
{
    if(sem != NULL)
    {
        Semaphore *semaphore = (Semaphore *) sem;

        (semaphore->initialValue)--;

        if(semaphore->initialValue < 0)
        {
            Thread *thread = runningThread;
            runningThread = getNextThreadFromReadyQueue();
            thread->status = 2;
            enqueue(semaphore->waitingThreads, thread);
            runningThread->status = 1;
            swapcontext(&(thread->threadContext), &(runningThread->threadContext));
        }
    }
    else
    {
        printf("Error : Given semaphore is NULL.\n");
        printf("Cannot wait on given semaphore.\n");
        exit(0);
    }
}

/**
        Destroys the semaphore
*/
int MySemaphoreDestroy(MySemaphore sem)
{
    if(sem != NULL)
    {
        Semaphore *semaphore = (Semaphore *) sem;

        if(getLength(semaphore->waitingThreads) == 0)
        {
            free(semaphore->waitingThreads);
            semaphore->waitingThreads = NULL;
            free(semaphore);
            return 0;
        }
        else
        {
            //printf("Failure : Threads are still waiting on given semaphore.\n");
            return -1;
        }
    }
    else
    {
        printf("Error : Given semaphore is NULL.\n");
        printf("Cannot destroy on given semaphore.\n");
        exit(0);
    }
}
