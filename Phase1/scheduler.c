#include <sys/sem.h>
#include <errno.h>
#include "queue.h"

int main(int argc, char *argv[])
{
    key_t key, remKey;
    int msgid, msgid2;
    struct msgbuff3 processState;
    struct remMsgbuff remMsg; // message buffer to send the remaining time to the process

    remKey = ftok("keyfile", 90);
    if (key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Generate a unique key
    key = ftok("keyfile", 80);
    if (key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Create or get message queue
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

    int msgq1_id, msgq2_id;
    struct msgbuff message;
    struct msgbuff2 details;
    int algo, quanta;
    Queue *PCB;
    createQueue(PCB);

    // Get message queue ID
    key_t key_id = ftok("keyfile", 65);
    msgq1_id = msgget(key_id, 0666);
    if (msgq1_id == -1)
    {
        perror("Error getting message queue");
        exit(EXIT_FAILURE);
    }

    int semid2 = semget(sem_2_key, 1, IPC_CREAT | 0666);
    if (semid2 == -1)
    {
        perror("semget");
    }

    int semid3 = semget(sem_3_key, 1, IPC_CREAT | 0666);
    if (semid3 == -1)
    {
        perror("semget");
    }

    key_t key_id2 = ftok("keyfile", 70);
    msgq2_id = msgget(key_id2, 0666 | IPC_CREAT);
    if (msgq2_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    // receive message holding the algo type and the quanta of the rr
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
    if (algo == 1)
    {
        // Creating queue
        Queue *queue;
        queue = createQueue();
        int remaining_quantum = quanta;
        int running_process_id = -1;
        Process process;

        // Receive process objects from the message queue
        while (1)
        {
            down(semid2);
            if (msgrcv(msgid, &processState, sizeof(processState), 80, IPC_NOWAIT) == -1)
            {
                // perror("msgrcv");
                // exit(EXIT_FAILURE);
            }
            else
            {
                running_process_id = -1;
                // TODO:
                // Free and delete the process after termination
            }
            int sem_value;
            if ((sem_value = semctl(semid2, 0, GETVAL)) == -1)
            {
                perror("Error getting semaphore value");
                exit(EXIT_FAILURE);
            }
            if (msgrcv(msgq1_id, &message, sizeof(struct msgbuff), 7, IPC_NOWAIT) == -1)
            {
                // perror("Error receiving message");
                // exit(EXIT_FAILURE);
            }
            else
            {
                printf("Message recieved successfully from process\n");
                printf("process id: %d \n", message.process.id);
                message.process.pcb.rem_time = message.process.runtime;
                message.process.pcb.state = 0;
                message.process.pcb.waiting_time = 0;
                printf("RUNTIME???? %d\n", message.process.runtime);
                printf("the process.pcb.rem: %d\n", process.pcb.rem_time);
                enqueue(queue, message.process);
                displayQueue(queue);
            }
            // printf("Semaphore value: %d\n", sem_value);
            if (running_process_id == -1)
            {
                // No process is running, try to dequeue from the queue
                if (!isEmpty(queue))
                {
                    // Dequeue the next process
                    Process next_process = dequeue(queue);

                    // Check if the process has already been forked
                    if (!next_process.isForked)
                    {
                        // Fork a new process to execute the program
                        pid_t pid = fork();
                        if (pid < 0)
                        {
                            perror("Fork failed");
                            exit(EXIT_FAILURE);
                        }
                        else if (pid == 0)
                        {
                            // Child process

                            char *const args[] = {"./process.out", NULL};
                            execv("./process.out", args);
                            perror("Execv failed");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            // Parent process
                            remMsg.remaining_time = next_process.runtime;
                            remMsg.mtype = 36;
                            if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                            {
                                perror("msgsnd");
                                exit(EXIT_FAILURE);
                            }
                            else
                            {
                                printf("Message sent successfuuly;\n");
                            }
                            printf("Process %d started\n", next_process.id);
                            next_process.isForked = true; // Mark the process as forked
                            next_process.display = pid;
                        }
                    }

                    else
                    {
                        kill(next_process.display, SIGCONT);
                        remMsg.remaining_time = process.pcb.rem_time;
                        remMsg.mtype = 36;
                        if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                        {
                            perror("msgsnd");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            printf("Message sent from scheduler\n");
                        }
                        printf("Process %d resumed\n", next_process.id);
                    }

                    running_process_id = next_process.id;
                    remaining_quantum = quanta; // Reset quantum for new process
                    process = next_process;
                }
            }
            else
            {
                // There is a process running
                if (remaining_quantum > 0)
                {
                    remaining_quantum--;
                    process.pcb.rem_time--;
                    remMsg.remaining_time = process.pcb.rem_time;
                    remMsg.mtype = 36;
                    if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                    {
                        perror("msgsnd");
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        printf("Message sent from scheduler\n");
                    }
                }
                if (remaining_quantum == 0)
                {
                    // Quantum has ended, stop the current process and put it at the end of the queue
                    running_process_id = -1;
                    enqueue(queue, process);
                    kill(process.display, SIGSTOP);
                    printf("Salam\n");
                }
            }
            // Wait for a clock tick
            printf("is there a process running ?: %d\n", running_process_id);
            up(semid3);
            sleep(1);
        }
    }

    destroyClk(true);
    //

    // //TODO implement the scheduler :)
    // //upon termination release the clock resources.

    //
}
