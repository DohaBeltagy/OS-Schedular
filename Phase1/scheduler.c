#include "headers.h"

int main(int argc, char *argv[])
{
    initClk();

    int msgq_id;
    struct msgbuff message;

    // Get message queue ID
    key_t key_id = ftok("keyfile", 65);
    msgq_id = msgget(key_id, 0666);
    if (msgq_id == -1)
    {
        perror("Error getting message queue");
        exit(EXIT_FAILURE);
    }

    // TODO: Implement scheduler logic

    // Receive process objects from the message queue
    while (1)
    {
        if (msgrcv(msgq_id, &message, sizeof(struct msgbuff) - sizeof(long), 7, !IPC_NOWAIT) == -1)
        {
            perror("Error receiving message");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("Message recieved successfully!!!!!!!\n");
            printf("process id: %d \n", message.process.id);
        }
        // Process the received message
        Process process = message.process;
        // Implement your scheduling logic here using the received process object
    }

    destroyClk(true);
    //

    // //TODO implement the scheduler :)
    // //upon termination release the clock resources.

    //
}
