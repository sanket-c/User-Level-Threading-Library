#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void initializeQueue(Queue *queue)
{
    if(queue != NULL)
    {
        queue->head = NULL;
        queue->length = 0;
    }
    else
    {
        printf("Error : Queue is NULL.\n");
        printf("Cannot initialize Queue.\n");
    }
}

void enqueue(Queue *queue, void *dataValue)
{
    if(queue != NULL)
    {
        if (queue->head == NULL)
        {
            Element *node = malloc(sizeof(Element));
            node->data = dataValue;
            node->next = node;
            node->previous = node;
            queue->head = node;
            queue->length++;
            return;
        }

        Element *node = malloc(sizeof(Element));
        node->data = dataValue;
        node->next = queue->head;
        node->previous = queue->head->previous;
        queue->head->previous->next = node;
        queue->head->previous = node;
        queue->length++;
    }
    else
    {
        printf("Error : Queue is NULL.\n");
        printf("Cannot enqueue data in Queue.\n");
    }
}

Element* dequeue(Queue *queue)
{
    if(queue != NULL)
    {
        Element *temp;
        Element *node = queue->head;
        if(node == NULL)
        {
            printf("Error : The Queue is Empty.\n");
            printf("Exiting.\n");
            return node;
        }
        else if (node->next == queue->head)
        {
            queue->head = NULL;
            queue->length --;
        }
        else
        {
            queue->head = node->next;
            temp = node->previous;
            temp->next = node->next;
            node->next->previous = temp;
            queue->length--;
        }
        return node;
    }
    else
    {
        printf("Error : Queue is NULL.\n");
        return NULL;
    }
}

int getLength(Queue *queue)
{
    if(queue != NULL)
    {
        return queue->length;
    }
    else
    {
        printf("Error : Queue is NULL.\n");
        return 0;
    }
}

int ifExist(Queue *queue, void* dataValue)
{
    if(queue != NULL)
    {
        Element *node = queue->head;
        if(node != NULL)
        {
            do
            {
                if(node->data == dataValue)
                {
                    return 1;
                }
                node = node->next;
            }while(node != queue->head);
        }
    }
    else
    {
        printf("Error : Queue is NULL.\n");
    }
    return 0;
}

int removeElement(Queue *queue, void* dataValue)
{
    if(queue != NULL)
    {
        Element *node = queue->head;
        if(node != NULL)
        {
            do
            {
                if(getLength(queue) == 1 && node->data == dataValue)
                {
                    queue->head = NULL;
                    free(node);
                    queue->length--;
                    return 1;
                }
                else if(node->data == dataValue)
                {
                    if(node == queue->head)
                    {
                        queue->head = node->next;
                    }
                    node->previous->next = node->next;
                    node->next->previous = node->previous;
                    free(node);
                    queue->length--;
                    return 1;
                }
                node = node->next;
            }while(node != queue->head);
        }
    }
    else
    {
        printf("Error : Queue is NULL.\n");
    }
    return 0;
}

int copyQueue(Queue *sourceQueue, Queue *destinationQueue)
{
    if(sourceQueue != NULL && destinationQueue != NULL)
    {
        int length = getLength(sourceQueue);
        if(length > 0)
        {
            while(length != 0)
            {
                Element *node = dequeue(sourceQueue);
                enqueue(destinationQueue, node->data);
                enqueue(sourceQueue, node->data);
                length--;
            }
            return 0;
        }
        else
        {
            printf("Source queue is empty.\n");
            return -1;
        }
    }
    else
    {
        printf("Error : Queue is NULL.\n");
        return -1;
    }
}
