/*
 * This file is done for you.
 * Probably you will not need to change anything.
 * This file represents an emulated clock for simulation purpose only.
 * It is not a real part of operating system!
 */

#include <sys/sem.h>
#include "headers.h"

int shmid;

/* Clear the resources before exit */
void cleanup(int signum)
{
    shmctl(shmid, IPC_RMID, NULL);
    printf("Clock terminating!\n");
    exit(0);
}

/* This file represents the system clock for ease of calculations */
int main(int argc, char * argv[])
{

    int semid2 = semget(sem_2_key, 1, IPC_CREAT | 0666);
    if (semid2 == -1)
    {
        perror("semget");
    }

    union Semun semun;

    semun.val = 0; /* initial value of the semaphore, Binary semaphore */
    if (semctl(semid2, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }

    int semid3 = semget(sem_3_key, 1, IPC_CREAT | 0666);
    if (semid3 == -1)
    {
        perror("semget");
    }

    semun.val = 1; /* initial value of the semaphore, Binary semaphore */
    if (semctl(semid3, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }

    int semid4 = semget(sem_4_key, 1, IPC_CREAT | 0666);
    if (semid4 == -1)
    {
        perror("semget");
    }

    semun.val = 0; /* initial value of the semaphore, Binary semaphore */
    if (semctl(semid4, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }


    printf("Clock starting\n");
    signal(SIGINT, cleanup);
    int clk = 0;
    //Create shared memory for one integer variable 4 bytes
    shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);
    if ((long)shmid == -1)
    {
        perror("Error in creating shm!");
        exit(-1);
    }
    int * shmaddr = (int *) shmat(shmid, (void *)0, 0);
    if ((long)shmaddr == -1)
    {
        perror("Error in attaching the shm in clock!");
        exit(-1);
    }
    int semid1 = semget(server_sem_key, 1, IPC_CREAT | 0666);
    if (semid1 == -1)
    {
        perror("semget");
    }
    *shmaddr = clk; /* initialize shared memory */
    up(semid1);
    while (1)
    {
        down(semid3);
        sleep(1);
        (*shmaddr)++;
        printf("this is the clock: %d \n", *shmaddr);
        up(semid2);
        int sem_value;
        if ((sem_value = semctl(semid2, 0, GETVAL)) == -1) {
            perror("Error getting semaphore value");
            exit(EXIT_FAILURE);
        }
        //printf("Semaphore2 value: %d\n", sem_value);
    }
}
