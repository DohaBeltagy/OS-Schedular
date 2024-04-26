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
    Node* newNode = createNode(data);
    if (isSRTNEmpty(queue)) {
        queue->front = queue->rear = newNode;
    } 
    else 
    {
        Node* prev_ptr = nullptr;
        Node* curr_ptr = queue->front;
        while(newNode->data.pcb.rem_time > curr_ptr->data.pcb.rem_time)
        {
            prev_ptr = curr_ptr;
            curr_ptr= curr_ptr->next;
        }
        newNode = prev_ptr->next;
        newNode->next = curr_ptr;
        if (curr_ptr == nullptr) 
        {
            queue->rear = newNode;
        }
    }
}

// Function to dequeue a process
Process SRTNdequeue(SRTNQueue* queue) {
    if (SRTNisEmpty(queue)) {
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
    while (!isEmpty(queue)) {
        dequeue(queue);
    }
    free(queue);
}