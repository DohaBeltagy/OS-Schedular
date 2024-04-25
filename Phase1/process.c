#include "headers.h"
#include <errno.h>

/* Modify this file as needed*/
int remainingtime;

void down_with_retry(int semid) {
    while (1) {
        down(semid);
        if (errno != EINTR) {
            // No interruption by a signal, exit the loop
            break;
        }
        // Signal interrupted, clear errno and retry
        errno = 0;
    }
    if (errno != 0) {
        perror("Error in down()");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char * argv[])
{
    initClk();
     
    struct msgbuff3 processState;

    int msgq3_id;
    key_t key_id3 = ftok("keyfile", 75);
    msgq3_id = msgget(key_id3, 0666);
    if (msgq3_id == -1)
    {
        perror("Error getting message queue");
        //exit(EXIT_FAILURE);
    }

    // Convert the runtime argument from string to integer
    int runtime=atoi(argv[1]);
    remainingtime=runtime;
    // Now you can use the 'runtime' variable in your program
    printf("Runtime received: %d\n", remainingtime);

    int semid4 = semget(sem_4_key, 1, IPC_CREAT | 0666);
    if (semid4 == -1)
    {
        perror("semget");
    }

    int semid3 = semget(sem_3_key, 1, IPC_CREAT | 0666);
    if (semid3 == -1)
    {
        perror("semget");
    }

    //TODO it needs to get the remaining time from somewhere
    //remainingtime = ??;
    while (remainingtime > 0)
    {
        down_with_retry(semid4);
        printf("alooo from process \n");
        remainingtime--;
        // remainingtime = ??;
        up(semid3);
        sleep(1);
    }
    if(remainingtime==0)
    {
        processState.mtype = 49;
        processState.state = 1;
        int send_val1 = msgsnd(msgq3_id, &processState, sizeof(processState) - sizeof(long), !IPC_NOWAIT);
        if (send_val1 == -1)
        {
            perror("Error sending message");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("message sent\n");
        }
        printf("process terminated \n");
    }
    
    destroyClk(false);
    
    return 0;
}
