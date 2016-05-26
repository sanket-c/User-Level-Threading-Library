#ifndef QUEUE_H
#define QUEUE_H

typedef struct Node
{
    void *data;
    struct Node *next;
    struct Node *previous;
}Element;

typedef struct
{
    Element *head;
    int length;
}Queue;

extern void initializeQueue(Queue *queue);
extern void enqueue(Queue *queue, void *dataValue);
extern Element* dequeue(Queue *queue);
extern int getLength(Queue *queue);
extern int ifExist(Queue *queue, void *dataValue);
extern int removeElement(Queue *queue, void* dataValue);
extern int copyQueue(Queue *sourceQueue, Queue *destinationQueue);
#endif
