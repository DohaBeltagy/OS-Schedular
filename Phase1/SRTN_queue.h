#ifndef SRTN_QUEUE_H 
#define SRTN_QUEUE_H

#include "stdio.h"
#include "stdlib.h"
#include "headers.h"


// Define the structure for the queue
typedef struct {
    Node* front;
    Node* rear;
} SRTNQueue;

// Function to create a new node
Node* createSRTNNode(Process data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}


// Function to initialize a queue
SRTNQueue* createSRTNQueue() {
    SRTNQueue* queue = (SRTNQueue*)malloc(sizeof(SRTNQueue));
    if (queue == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    queue->front = queue->rear = NULL;
    return queue;
}

// Function to check if the queue is empty
int isSRTNEmpty(SRTNQueue* queue) {
    return queue->front == NULL;
}

// Function to enqueue a process
void SRTNenqueue(SRTNQueue* queue, Process data) {
    Node* newNode = createSRTNNode(data);
    if (isSRTNEmpty(queue))
    {
        printf("the srtn queue is empty \n");
        queue->front = newNode;
        queue->rear = newNode;
    }
    else
    {
        printf("the queue is not empty\n");
        Node *prev_ptr = NULL;
        Node *curr_ptr = queue->front;

        while (newNode->data.pcb.rem_time > curr_ptr->data.pcb.rem_time)
        {
            prev_ptr = curr_ptr;
            curr_ptr = curr_ptr->next;
            if(curr_ptr == NULL)
            {
                break;
            }
        }
        if (curr_ptr == NULL)
        {
            prev_ptr->next = newNode; // added this line
            queue->rear = newNode;
            return;
        }
        newNode->next = curr_ptr;
        if (prev_ptr != NULL)
        {
            prev_ptr->next = newNode;
        }
        else
        {
            queue->front = newNode;
        }
    }
}

// Function to dequeue a process
Process SRTNdequeue(SRTNQueue* queue) {
    if (isSRTNEmpty(queue)) {
        fprintf(stderr, "Queue is empty\n");
        exit(EXIT_FAILURE);
    }
    Node* temp = queue->front;
    Process data = temp->data;
    queue->front = queue->front->next;
    free(temp);
    return data;
}

// Function to display the contents of the queue (for testing purposes)
void displaySRTNQueue(SRTNQueue* queue) {
    Node* current = queue->front;
    printf("Queue: ");
    while (current != NULL) {
        printf("[%d] ", current->data.id);
        current = current->next;
    }
    printf("\n");
}

// Function to free memory allocated to the queue
void freeSRTNQueue(SRTNQueue* queue) {
    while (!isSRTNEmpty(queue)) {
        SRTNdequeue(queue);
    }
    free(queue);
}

Process getSRTNHead (SRTNQueue* queue) {
    return queue->front->data;
}

#endif