
#include "queue.h"

int main(int argc, char *argv[])
{
    initClk();

    int msgq1_id, msgq2_id;
    struct msgbuff message;
    struct msgbuff2 details;
    int algo, quanta;

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



    // Creating queue
    Queue *queue;
    queue = createQueue();

    // Receive process objects from the message queue
    while (1)
    {
        if (msgrcv(msgq1_id, &message, sizeof(struct msgbuff) - sizeof(long), 7, !IPC_NOWAIT) == -1)
        {
            perror("Error receiving message");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("Message recieved successfully from process\n");
            printf("process id: %d \n", message.process.id);
            enqueue(queue, message.process);
            printf("queue successfull");
        }
        // Implement your scheduling logic here using the received process object
    }

    destroyClk(true);
    //

    // //TODO implement the scheduler :)
    // //upon termination release the clock resources.

    //
}
