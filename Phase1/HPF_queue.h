#include "stdio.h"
#include "stdlib.h"
#include "headers.h"
#include "Phase1/headers.h"

// Define the structure for a node in the queue
typedef struct Node {
    Process data;
    struct Node* next;
} Node;

// Define the structure for the queue
typedef struct {
    Node* front;
    Node* rear;
} HPFQueue;

// Function to create a new node
Node* createNode(Process data) {
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
HPFQueue* createQueue() {
    HPFQueue* queue = (HPFQueue*)malloc(sizeof(HPFQueue));
    if (queue == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    queue->front = queue->rear = NULL;
    return queue;
}

// Function to check if the queue is empty
int isEmpty(HPFQueue* queue) {
    return queue->front == NULL;
}

// Function to enqueue a process
void enqueue(HPFQueue* queue, Process data) {
    Node* newNode = createNode(data);
    if (isEmpty(queue)) 
    {
        queue->front = queue->rear = newNode;
    } 
    else 
    {   Node* prev_ptr = nullptr;
        Node* curr_ptr = queue->front;
        while(newNode->data.priority > curr_ptr->data.priority)
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
Process dequeue(HPFQueue* queue) {
    if (isEmpty(queue)) {
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
void displayQueue(HPFQueue* queue) {
    Node* current = queue->front;
    printf("Queue: ");
    while (current != NULL) {
        printf("[%d] ", current->data.id);
        current = current->next;
    }
    printf("\n");
}

// Function to free memory allocated to the queue
void freeQueue(HPFQueue* queue) {
    while (!isEmpty(queue)) {
        dequeue(queue);
    }
    free(queue);
}
