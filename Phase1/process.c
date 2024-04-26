#include "headers.h"
#include <errno.h>

/* Modify this file as needed*/
int remainingtime;

int main(int argc, char *argv[])
{
    key_t key, remKey;
    int msgid, msgid2;
    struct msgbuff3 processState;
    struct remMsgbuff remMsg; // message buffer to send the remaining time to the process

    key = ftok("keyfile", 80);
    if (key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    msgid2 = msgget(remKey, 0666 | IPC_CREAT);
    if (msgid2 == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    initClk();
    if (msgrcv(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 36, !IPC_NOWAIT) == -1)
    {
        perror("Error receiving message");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("received the remaining time from the scheduler: %d\n", remMsg.remaining_time);
        remainingtime = remMsg.remaining_time;
    }

    // TODO it needs to get the remaining time from somewhere
    while (remainingtime > 0)
    {
        if (msgrcv(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 36, !IPC_NOWAIT) == -1)
        {
            perror("Error receiving message");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("received the remaining time from the scheduler: %d\n", remMsg.remaining_time);
            remainingtime = remMsg.remaining_time;
        }
    }
    if (remainingtime == 0)
    {
        processState.mtype = 80; // Message type (can be any positive integer)
        processState.state = 1;
        if (msgsnd(msgid, &processState, sizeof(processState), 0) == -1)
        {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        printf("process terminated \n");
    }

    destroyClk(false);

    return 0;
}
