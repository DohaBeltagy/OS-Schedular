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
    key_t key;
    int msgid;
    struct msgbuff3 processState;

    key = ftok("keyfile", 80);
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    initClk();
     
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
        if (remainingtime > 0)
        {
            up(semid3);
        }
        sleep(1);
    }
    if(remainingtime==0)
    {
        processState.mtype = 80; // Message type (can be any positive integer)
        processState.state = 1;
        if (msgsnd(msgid, &processState, sizeof(processState), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
        printf("process terminated \n");
        up(semid3);
    }
    
    destroyClk(false);
    
    return 0;
}
