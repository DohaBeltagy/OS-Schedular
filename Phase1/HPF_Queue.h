#ifndef HPF_QUEUE_H 
#define HPF_QUEUE_H

#include "stdio.h"
#include "stdlib.h"
#include "headers.h"

// // Define the structure for a node in the queue
// typedef struct  {
//     Process data;
//     struct Node* next;
// } Node;

// Define the structure for the queue
typedef struct {
    Node* front;
    Node* rear;
} HPFQueue;

// Function to create a new node
Node* createHPFNode(Process data) {
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
HPFQueue* createHPFQueue() {
    HPFQueue* queue = (HPFQueue*)malloc(sizeof(HPFQueue));
    if (queue == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    queue->front = queue->rear = NULL;
    return queue;
}

// Function to check if the queue is empty
int isHPFEmpty(HPFQueue* queue) {
    return queue->front == NULL;
}

// Function to enqueue a process
void HPFenqueue(HPFQueue* queue, Process data) {
    Node* newNode = createHPFNode(data);
    if (isHPFEmpty(queue)) 
    {
        queue->front = queue->rear = newNode;
    } 
    else 
    {   Node* prev_ptr = NULL;
        Node* curr_ptr = queue->front;
        while(newNode->data.priority > curr_ptr->data.priority)
        {
            prev_ptr = curr_ptr;
            curr_ptr= curr_ptr->next;
        }
        newNode = prev_ptr->next;
        newNode->next = curr_ptr;
        if (curr_ptr == NULL) 
        {
            queue->rear = newNode;
        }
    }
}

// Function to dequeue a process
Process HPFdequeue(HPFQueue* queue) {
    if (isHPFEmpty(queue)) {
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
void displayHPFQueue(HPFQueue* queue) {
    Node* current = queue->front;
    printf("Queue: ");
    while (current != NULL) {
        printf("[%d] ", current->data.id);
        current = current->next;
    }
    printf("\n");
}

// Function to free memory allocated to the queue
void freeHPFQueue(HPFQueue* queue) {
    while (!isHPFEmpty(queue)) {
        HPFdequeue(queue);
    }
    free(queue);
}

Process getHPFHead (HPFQueue* queue) {
    return queue->front->data;
}

#endif