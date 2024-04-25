
#include "queue.h"

int main(int argc, char *argv[])
{
    initClk();

    int msgq1_id, msgq2_id;
    struct msgbuff message;
    struct msgbuff2 details;
    int algo, quanta;
    int currentTime =getClk();
    // Get message queue ID
    key_t key_id = ftok("keyfile", 65);
    msgq1_id = msgget(key_id, 0666);
    if (msgq1_id == -1)
    {
        perror("Error getting message queue");
        exit(EXIT_FAILURE);
    }

    key_t key_id2 = ftok("keyfile", 70);
    msgq2_id = msgget(key_id2, 0666 | IPC_CREAT);
    if (msgq2_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    //receive message holding the algo type and the quanta of the rr
    if (msgrcv(msgq2_id, &details, sizeof(struct msgbuff2) - sizeof(long), 10, !IPC_NOWAIT) == -1)
    {
        perror("Error receiving message");
        exit(EXIT_FAILURE);
    }
    else
    {
        algo = details.algoType;
        quanta = details.quanta; 
        printf("Message recieved successfully from algo and quanta \n");
        printf("algo type: %d \n", details.algoType);
        printf("quatnta : %d \n", details.quanta);
    }

/////////////////////////////////////////////////////////////////Round Robin///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Creating queue
    Queue *ready_queue;
    ready_queue = createQueue();

    Process running_process; // To keep track of the currently running process
    bool is_process_running = false; // Flag to indicate if a process is currently running
    int quanta_counter = 0; // Counter to track the remaining quanta for the running process

    // Receive process objects from the message queue and perform scheduling
    while (1)
    {
        // Check if there's a running process and if its quanta has ended
        if (is_process_running && quanta_counter == 0) {
            // Send a stop signal to the running process
            kill(running_process.id, SIGSTOP);
            printf("Process %d stopped\n", running_process.id);

            // Enqueue the stopped process at the end of the ready queue
            enqueue(ready_queue, running_process);
            is_process_running = false; // No process is running anymore
        }

        // If there's no running process, check if there's a process in the ready queue
        if (!is_process_running && !isEmpty(ready_queue)) {
            // Dequeue the next process to run
            running_process = dequeue(ready_queue);
            is_process_running = true; // Set the flag indicating a process is running
            quanta_counter = quanta; // Reset the quanta counter

            // Fork a new process to execute the program if it's the first time running
            if (!running_process.forked) {
                pid_t pid = fork();
                if (pid < 0) {
                    perror("Fork failed");
                    exit(EXIT_FAILURE);
                } else if (pid == 0) {
                    // Child process
                    char runtime_str[10];
                    sprintf(runtime_str, "%d", running_process.runtime);
                    char *const args[] = {"./process.out", runtime_str, NULL};
                    execv("./process.out", args);
                    perror("Execv failed");
                    exit(EXIT_FAILURE);
                } else {
                    running_process.forked = true; // Mark the process as forked
                    printf("Process %d started\n", running_process.id);
                }
            } else {
                // Resume the process by sending a SIGCONT signal
                kill(running_process.id, SIGCONT);
                printf("Process %d resumed\n", running_process.id);
            }
        }

        // Receive process objects from the message queue
        if (msgrcv(msgq1_id, &message, sizeof(struct msgbuff) - sizeof(long), 7, !IPC_NOWAIT) == -1)
        {
            perror("Error receiving message");
            exit(EXIT_FAILURE);
        }
        else
        {
            // New message received, enqueue it in the ready queue
            printf("Message received successfully from process\n");
            printf("Process id: %d \n", message.process.id);
            enqueue(ready_queue, message.process);
            displayQueue(ready_queue);
        }

        // Decrement the quanta counter if a process is running
        if (is_process_running && quanta_counter > 0) {
            quanta_counter--;
        }
    }

    destroyClk(true); // Terminate the clock process upon exit


}
