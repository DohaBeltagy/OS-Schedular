#include <sys/sem.h>
#include <errno.h>
#include "queue.h"
#include "HPF_Queue.h"
#include "SRTN_queue.h"
#include <signal.h>
#include <math.h>
#include "memory.c"

FILE *perf_file;

void handle_signal(int signal)
{
    if (perf_file != NULL)
    {
        fclose(perf_file);
    }
    exit(signal);
}

int main(int argc, char *argv[])
{
    printf("in scheduler \n");
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
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
    int algo, quanta, processNum;
    Queue *finished;
    finished = createQueue();
    float idleTime = 0;
    float totalTime = 0;
    int rdy_processCount = 0;
    // a blocked queue to hold the processes that don't have a place in memory
    Queue *blocked_processes;
    blocked_processes = createQueue();
    int blk_processCount = 0;

    initialize(1024);
    printf("after initialize \n");

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
        processNum = details.processesNum;
        printf("Message recieved successfully from algo and quanta \n");
        printf("algo type: %d \n", details.algoType);
        printf("quatnta : %d \n", details.quanta);
        printf("Number of processes equals : %d \n", details.processesNum);
    }
    FILE *file = fopen("log.txt", "w");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    // Print how log works
    fprintf(file, "#At time x Process y state arr w total z remain y wait k\n");
    fflush(file);
    int finishedProcesses = 0;

    FILE * mem_file = fopen("memory.log","w");
    if (mem_file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    // Print how log works
    fprintf(mem_file, "#At time x allocated y bytes for process z from i to j\n");
    fflush(mem_file);

    // Round Robin Algorithm
    if (algo == 1)
    {
        // Creating queue
        Queue *queue;
        queue = createQueue();
        int remaining_quantum = quanta;
        int running_process_id = -1;
        Process process;
        Process blocked;

        // Receive process objects from the message queue
        while (finishedProcesses < processNum)
        {
            down(semid2);
            int sem_value;
            if ((sem_value = semctl(semid2, 0, GETVAL)) == -1)
            {
                perror("Error getting semaphore value");
                // exit(EXIT_FAILURE);
            }
            if (msgrcv(msgq1_id, &message, sizeof(struct msgbuff), 7, IPC_NOWAIT) == -1)
            {
                // perror("Error receiving message");
                // exit(EXIT_FAILURE);
            }
            else
            {
                printf("Message recieved successfully from process generator\n");
                printf("process id: %d \n", message.process.id);
                message.process.pcb.state = 0;
                message.process.pcb.waiting_time = 0;
                // try to allocate memory for the process
                // if it returned -1, this will mean that there's no space for it in the memory
                // so we enqueue in the blocked queue and continue to get the next process
                printf("BEFORE THE BUDDY ALLOC\n");
                int process_adress = allocate(message.process.mem_size);
                printf("this is the process address %d \n", process_adress);
                printf("AFTER THE BUDDY ALLOC\n");
                if (process_adress == -1)
                {
                    printf("NO ENOUGH SPACE IN MEMORY, TRY AGAIN LATER\n");
                    enqueue(blocked_processes, message.process);
                    blk_processCount++;
                    printf("This is blocked queue: \n");
                    displayQueue(blocked_processes);
                }
                else
                {
                    message.process.address = process_adress;
                    printf("MEMORY ALLOCATED SUCCEFULLY AND THIS IS ITS ADDRESS: %d\n", message.process.address);
                    enqueue(queue, message.process); // enqueue in ready queue
                    rdy_processCount++;
                    displayQueue(queue);
                    int n = ceil(log(message.process.mem_size) / log(2));
                    int final_size = pow(2,n);
                    fprintf(mem_file,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),message.process.mem_size,message.process.id,message.process.address,message.process.address+final_size-1);
                    fflush(mem_file);
                }
            }

            if (running_process_id == -1)
            {
                // No process is running, try to dequeue from the queue
                if (!isEmpty(queue))
                {
                    // Dequeue the next process
                    Process next_process = dequeue(queue);
                    rdy_processCount--;

                    // Check if the process has already been forked
                    if (!next_process.isForked)
                    {
                        // Fork a new process to execute the program
                        pid_t pid = fork();
                        if (pid < 0)
                        {
                            perror("Fork failed");
                        }
                        else if (pid == 0)
                        {
                            // Child process
                            char *const args[] = {"./process.out", NULL};
                            execv("./process.out", args);
                            perror("Execv failed");
                        }
                        else
                        {
                            // Parent process
                            remMsg.remaining_time = next_process.runtime;
                            remMsg.mtype = 36;
                            if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                            {
                                perror("msgsnd");
                            }
                            else
                            {
                                printf("Message sent successfuly from scheduler to process\n");
                            }
                            printf("Process %d started\n", next_process.id);
                            next_process.isForked = true; // Mark the process as forked
                            next_process.display = pid;   // Set the process's pid to its actual pid
                            // print state at process start in file

                            int total = next_process.runtime;
                            fprintf(file, "At time %d Process %d started arr %d total %d remain %d wait %d\n", getClk(), next_process.id, next_process.arrival_time, total, next_process.pcb.rem_time, next_process.pcb.waiting_time);
                            fflush(file);
                        }
                    }
                    // the process is an old process (already forked)
                    else
                    {
                        printf("Process %d resumed\n", next_process.id);
                        // print process resumed state in file
                        int total = next_process.runtime;
                        fprintf(file, "At time %d Process %d resumed arr %d total %d remain %d wait %d\n", getClk(), next_process.id, next_process.arrival_time, total, next_process.pcb.rem_time, next_process.pcb.waiting_time);
                        fflush(file);
                        kill(next_process.display, SIGCONT);
                        printf("NEXT PROCESS REM TIME IS: %d\n", next_process.pcb.rem_time);
                        remMsg.remaining_time = next_process.pcb.rem_time;
                        remMsg.mtype = 36;
                        if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                        {
                            perror("msgsnd");
                        }
                        else
                        {
                            printf("Message sent from scheduler\n");
                        }
                    }
                    running_process_id = next_process.id;
                    remaining_quantum = quanta; // Reset quantum for new process
                    process = next_process;
                }
                // queue is empty and the program is still running
                else
                {
                    idleTime++; // correct don't remove
                }
            }
            // There is a process running
            else
            {
                if (remaining_quantum > 0)
                {
                    remaining_quantum--;
                    process.pcb.rem_time = process.pcb.rem_time - 1;
                    remMsg.remaining_time = process.pcb.rem_time;
                    remMsg.mtype = 36;
                    if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                    {
                        perror("msgsnd");
                    }
                    else
                    {
                        printf("Message sent from scheduler\n");
                    }
                }
                if (remaining_quantum == 0 && remMsg.remaining_time != 0)
                {
                    // Quantum has ended, stop the current process and put it at the end of the queue
                    running_process_id = -1;
                    enqueue(queue, process);
                    kill(process.display, SIGSTOP);
                    rdy_processCount++;
                    printf("Process %d stopped\n", process.id);
                    // print process stopped state in file
                    int total = process.runtime;
                    fprintf(file, "At time %d Process %d stopped arr %d total %d remain %d wait %d\n", getClk(), process.id, process.arrival_time, total, process.pcb.rem_time, process.pcb.waiting_time);
                    up(semid2);
                    continue;
                }
                sleep(2);
            }
            // message queue to recieve notification from process upon termination
            if (msgrcv(msgid, &processState, sizeof(processState), 80, IPC_NOWAIT) == -1)
            {
                // perror("msgrcv");
            }
            else
            {

                running_process_id = -1;
                finishedProcesses++;
                process.pcb.finish_time = getClk();
                printf("Finish Time = %d", process.pcb.finish_time);
                // print state at termination in file
                int total = process.runtime;
                int TA = process.pcb.finish_time - process.arrival_time;
                float WTA;
                if (process.runtime == 0)
                {
                    WTA = 0;
                }
                else
                {
                    WTA = TA / process.runtime;
                }
                fprintf(file, "At time %d Process %d finished arr %d total %d remain %d wait %d TA %d WTA %0.2f\n", getClk(), process.id, process.arrival_time, total, process.pcb.rem_time, process.pcb.waiting_time, TA, WTA);
                fflush(file);
                printf("This is finish queue: \n");
                enqueue(finished, process);
                displayQueue(finished);

                // remove the allocated memory for the process
                deallocate(process.address);
                int n = ceil(log(process.mem_size) / log(2));
                int final_size = pow(2,n);
                fprintf(mem_file,"At time %d freed %d bytes from process %d from %d to %d\n", getClk(), process.mem_size, process.id,process.address, process.address+final_size-1);
                fflush(mem_file);
                // check the blocked queue for the first process to be allocated
                while (!isEmpty(blocked_processes))
                {
                    // we check if the there is a place for the first process in the blocked queue
                    // if yes, we dequeue the process and add it to the ready queue

                    if (checkAllocation(blocked_processes->front->data.mem_size) == 1)
                    {
                        blocked = dequeue(blocked_processes);
                        printf("BEFORE THE BUDDY ALLOC\n");
                        int process_adress = allocate(blocked.mem_size);
                        blocked.address = process_adress;
                        printf("this is the process address %d \n", process_adress);
                        printf("AFTER THE BUDDY ALLOC\n");
                        enqueue(queue, blocked); // to be removed
                        rdy_processCount++;
                        displayQueue(queue);
                        blk_processCount--;
                        int n = ceil(log(blocked.mem_size) / log(2));
                        int final_size = pow(2,n);
                        fprintf(mem_file,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),blocked.mem_size,blocked.id,blocked.address, blocked.address+final_size-1);
                        fflush(mem_file);
                    }
                    else
                    {
                        break;
                    }
                }
                up(semid2);
                continue;
            }          
            totalTime++;
            displayQueue(queue);
            printf("before up\n");
            if (rdy_processCount > 0)
            {
                Process processCalc;
                for (int i = 0; i < rdy_processCount; i++)
                {
                    Process processCalc = dequeue(queue);
                    processCalc.pcb.waiting_time = processCalc.pcb.waiting_time + 1;
                    enqueue(queue, processCalc);
                }
            }
            if (blk_processCount > 0)
            {
                for (int i = 0; i < blk_processCount; i++)
                {
                    Process processCalc = dequeue(blocked_processes);
                    processCalc.pcb.waiting_time = processCalc.pcb.waiting_time + 1;
                    enqueue(blocked_processes, processCalc);
                }   
            }    
            up(semid3);
            // sleep(1);
        }
    }
    else if (algo == 2) // shortest remaining time next
    {
        // Creating queue
        SRTNQueue *queue;
        queue = createSRTNQueue();
        int running_process_id = -1;
        Process running_process;
        Process next_process;
        running_process.isForked = false;
        Process blocked;

        // Receive process objects from the message queue
        while (finishedProcesses < processNum)
        {
            down(semid2);
            int sem_value;
            if ((sem_value = semctl(semid2, 0, GETVAL)) == -1)
            {
                perror("Error getting semaphore value");
                // exit(EXIT_FAILURE);
            }
            // if there was a running process
            if (running_process_id != -1)
            {
                // check if the ready queue is not empty , increment the waiting time of each
                // decrement the remaining time and check termination
                running_process.pcb.rem_time--;
                remMsg.remaining_time = running_process.pcb.rem_time;
                remMsg.mtype = 36;
                if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                {
                    perror("msgsnd");
                    // exit(EXIT_FAILURE);
                }
                else
                {
                    printf("Message sent successfuuly;\n");
                }
                sleep(2);
                if (msgrcv(msgid, &processState, sizeof(processState), 80, IPC_NOWAIT) == -1)
                {
                    // perror("msgrcv");
                    // exit(EXIT_FAILURE);
                }
                else
                {
                    finishedProcesses++;
                    running_process.pcb.finish_time = getClk();
                    printf("Finish Time = %d", running_process.pcb.finish_time);
                    // print state at termination in file
                    int total = running_process.runtime;
                    int TA = running_process.pcb.finish_time - running_process.arrival_time;
                    float WTA;
                    if (running_process.runtime == 0)
                    {
                        WTA = 0;
                    }
                    else
                    {
                        WTA = TA / running_process.runtime;
                    }
                    fprintf(file, "At time %d Process %d finished arr %d total %d remain %d wait %d TA %d WTA %f\n", getClk(), running_process.id, running_process.arrival_time, total, running_process.pcb.rem_time, running_process.pcb.waiting_time, TA, WTA);
                    fflush(file);
                    printf("This is finish queue: \n");
                    enqueue(finished, running_process);
                    displayQueue(finished);
                    running_process_id = -1;
                    // remove the allocated memory for the process
                    deallocate(running_process.address);
                    int n = ceil(log(running_process.mem_size) / log(2));
                    int final_size = pow(2,n);
                    fprintf(mem_file,"At time %d freed %d bytes from process %d from %d to %d\n", getClk(), running_process.mem_size, running_process.id,running_process.address,running_process.address+final_size-1);
                    fflush(mem_file);
                    // check the blocked queue for the first process to be allocated
                    while (!isEmpty(blocked_processes))
                    {
                        // we check if the there is a place for the first process in the blocked queue
                        // if yes, we dequeue the process and add it to the ready queue
                    
                        if (checkAllocation(blocked_processes->front->data.mem_size) == 1)
                        {
                            blocked = dequeue(blocked_processes);
                            blk_processCount--;
                            printf("blocked process dequeued: %d\n", blocked.id);
                            printf("BEFORE THE BUDDY ALLOC\n");
                            int process_adress = allocate(blocked.mem_size);
                            blocked.address = process_adress;
                            printf("this is the process address %d \n", process_adress);
                            printf("AFTER THE BUDDY ALLOC\n");
                            SRTNenqueue(queue, blocked); // to be removed
                            rdy_processCount++;
                            displaySRTNQueue(queue);
                            int n = ceil(log(blocked.mem_size) / log(2));
                            int final_size = pow(2,n);
                            fprintf(mem_file,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),blocked.mem_size,blocked.id,blocked.address, blocked.address+final_size-1);
                            fflush(mem_file);
                        }
                        else
                        {
                            break;
                        }
                    }
                    up(semid2);
                    continue;

                    // check if the ready queue is not empty , increment the waiting time of each
                    pid_t pid = fork();
                    if (pid < 0)
                    {
                        perror("Fork failed");
                        // exit(EXIT_FAILURE);
                    }
                    else if (pid == 0)
                    {
                        // Child process
                        running_process = message.process;
                        running_process_id = message.process.id;
                        running_process.isForked = true;
                        char *const args[] = {"./process.out", NULL};
                        execv("./process.out", args);
                        perror("Execv failed");
                        // exit(EXIT_FAILURE);
                    }
                    else
                    {
                        // Parent process
                        remMsg.remaining_time = running_process.runtime;
                        remMsg.mtype = 36;
                        if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                        {
                            perror("msgsnd");
                            // exit(EXIT_FAILURE);
                        }
                        else
                        {
                            printf("Message sent successfuuly;\n");
                        }
                        printf("Process %d started\n", running_process.id);
                        sleep(2);
                    }
                }
            }
            // if no process was received at this second
            if (msgrcv(msgq1_id, &message, sizeof(struct msgbuff), 7, IPC_NOWAIT) == -1)
            {
                // if there was no running process and the ready queue was empty, increment the idle time
                if (running_process_id == -1 && isSRTNEmpty(queue))
                {
                    idleTime++;
                }
                // if there was no running process and the ready queue wasn't empty
                if (running_process_id == -1 && !isSRTNEmpty(queue))
                {
                    printf("no running process and the ready queue is not empty -> dequeue the head process\n");
                    // dequeue the process next in line and make it the running process
                    next_process = getSRTNHead(queue);
                    running_process = next_process;
                    running_process_id = next_process.id;
                    SRTNdequeue(queue);
                    rdy_processCount--;
                    if (running_process.isForked == false)
                    {
                        running_process.isForked = true;
                        printf("Process %d started\n", running_process.id);
                        int total = running_process.runtime;
                        fprintf(file, "At time %d Process %d started arr %d total %d remain %d wait %d\n", getClk(), running_process.id, running_process.arrival_time, total, running_process.pcb.rem_time, running_process.pcb.waiting_time);
                        fflush(file);
                        pid_t pid = fork();
                        if (pid < 0)
                        {
                            perror("Fork failed");
                            // exit(EXIT_FAILURE);
                        }
                        else if (pid == 0)
                        {
                            // Child process
                            running_process = message.process;
                            running_process_id = message.process.id;
                            running_process.isForked = true;
                            char *const args[] = {"./process.out", NULL};
                            execv("./process.out", args);
                            perror("Execv failed");
                            // exit(EXIT_FAILURE);
                        }
                        else
                        {
                            // Parent process
                            // remMsg.remaining_time = running_process.runtime;
                            // remMsg.mtype = 36;
                            // if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                            // {
                            //     perror("msgsnd");
                            //     // exit(EXIT_FAILURE);
                            // }
                            // else
                            // {
                            //     printf("Message sent successfuuly;\n");
                            // }
                            // printf("Process %d started\n", running_process.id);
                            // sleep(2);
                        }
                    }
                    else
                    {
                        int total = running_process.runtime;
                        fprintf(file, "At time %d Process %d resumed arr %d total %d remain %d wait %d\n", getClk(), running_process.id, running_process.arrival_time, total, running_process.pcb.rem_time, running_process.pcb.waiting_time);
                        fflush(file);
                    }

                    // check if the ready queue is not empty , increment the waiting time of each
                }
            }
            else // there was a process received at this second
            {
                printf("Message recieved successfully from process\n");
                printf("process id: %d \n", message.process.id);
                message.process.pcb.state = 0;
                message.process.pcb.waiting_time = 0;
                message.process.pcb.rem_time = message.process.runtime;
                message.process.isForked = false;
                // try to allocate memory for the process
                // if it returned -1, this will mean that there's no space for it in the memory
                // so we enqueue in the blocked queue and continue to get the next process
                printf("BEFORE THE BUDDY ALLOC\n");
                int process_adress = allocate(message.process.mem_size);
                printf("this is the process address %d \n", process_adress);
                printf("AFTER THE BUDDY ALLOC\n");
                if (process_adress == -1)
                {
                    printf("NO ENOUGH SPACE IN MEMORY, TRY AGAIN LATER\n");
                    enqueue(blocked_processes, message.process);
                    blk_processCount++;
                    printf("This is blocked queue: \n");
                    displayQueue(blocked_processes);
                    up(semid2);
                    continue;                  
                }
                else
                {
                    message.process.address = process_adress;
                    printf("MEMORY ALLOCATED SUCCEFULLY AND THIS IS ITS ADDRESS: %d\n", message.process.address);
                    int n = ceil(log(message.process.mem_size) / log(2));
                    int final_size = pow(2,n);
                    fprintf(mem_file,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),message.process.mem_size,message.process.id,message.process.address,message.process.address+final_size-1);
                    fflush(mem_file);
                    // if there was no running process and no processes in the ready queue
                    if (running_process_id == -1 && isSRTNEmpty(queue))
                    {
                        printf("no running process and no process in the ready queue-> incoming process becomes running\n");
                        // Fork a new process to execute the program
                        running_process = message.process;
                        running_process_id = message.process.id;
                        running_process.isForked = true;
                        printf("Process %d started\n", running_process.id);
                        int total = running_process.runtime;
                        fprintf(file, "At time %d Process %d started arr %d total %d remain %d wait %d\n", getClk(), running_process.id, running_process.arrival_time, total, running_process.pcb.rem_time, next_process.pcb.waiting_time);
                        fflush(file);
                        pid_t pid = fork();
                        if (pid < 0)
                        {
                            perror("Fork failed");
                            // exit(EXIT_FAILURE);
                        }
                        else if (pid == 0)
                        {
                            // Child process
                            char *const args[] = {"./process.out", NULL};
                            execv("./process.out", args);
                            perror("Execv failed");
                            // exit(EXIT_FAILURE);
                        }
                        else
                        {
                            // Parent process
                            remMsg.remaining_time = running_process.runtime;
                            remMsg.mtype = 36;
                            if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                            {
                                perror("msgsnd");
                                // exit(EXIT_FAILURE);
                            }
                            else
                            {
                                printf("Message sent successfuuly;\n");
                            }
                            printf("Process %d started\n", running_process.id);
                            sleep(2);
                        }
                        displaySRTNQueue(queue);
                    }
                    // if there was a running process and the incoming process has less remaining time than that of the running process
                    else if (running_process_id != -1 && running_process.pcb.rem_time > message.process.pcb.rem_time)
                    {
                        printf("there is a running process but with bigger remaining time than the incoming one\n");
                        kill(running_process.display, SIGSTOP);
                        SRTNenqueue(queue, running_process);
                        rdy_processCount++;
                        int total = running_process.runtime;
                        sleep(2);
                        fprintf(file, "At time %d Process %d stopped arr %d total %d remain %d wait %d\n", getClk(), running_process.id, running_process.arrival_time, total, running_process.pcb.rem_time, running_process.pcb.waiting_time);
                        running_process = message.process;
                        running_process_id = message.process.id;
                        running_process.isForked = true;
                        printf("Process %d started\n", running_process.id);
                        total = running_process.runtime;
                        fprintf(file, "At time %d Process %d started arr %d total %d remain %d wait %d\n", getClk(), running_process.id, running_process.arrival_time, total, running_process.pcb.rem_time, running_process.pcb.waiting_time);
                        fflush(file);
                        // check if the ready queue is not empty , increment the waiting time of each
                        next_process = getSRTNHead(queue);
                        // Fork a new process to execute the program
                        next_process = getSRTNHead(queue);
                        pid_t pid = fork();
                        if (pid < 0)
                        {
                            perror("Fork failed");
                            // exit(EXIT_FAILURE);
                        }
                        else if (pid == 0)
                        {
                            // Child process
                            char *const args[] = {"./process.out", NULL};
                            execv("./process.out", args);
                            perror("Execv failed");
                            // exit(EXIT_FAILURE);
                        }
                        else
                        {
                            // Parent process
                            remMsg.remaining_time = running_process.runtime;
                            remMsg.mtype = 36;
                            if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                            {
                                perror("msgsnd");
                                // exit(EXIT_FAILURE);
                            }
                            else
                            {
                                printf("Message sent successfuuly;\n");
                            }
                            printf("Process %d started\n", running_process.id);
                            sleep(2);
                        }
                        displaySRTNQueue(queue);
                    }
                    // if there is no running process and the remaining time of the incoming process is higher than that of the next process in the queue
                    else if (running_process_id == -1 && getSRTNHead(queue).pcb.rem_time < message.process.pcb.rem_time)
                    {
                        printf("there is no running process and the remaining time of the incoming is greater than the head -> dequeue the head and enqueue incoming\n");
                        running_process = getSRTNHead(queue);
                        running_process_id = running_process.id;
                        SRTNdequeue(queue);
                        SRTNenqueue(queue, message.process);
                        if (running_process.isForked == false)
                        {
                            running_process.isForked = true;
                            printf("Process %d started\n", running_process.id);
                            int total = running_process.runtime;
                            fprintf(file, "At time %d Process %d started arr %d total %d remain %d wait %d\n", getClk(), running_process.id, running_process.arrival_time, total, running_process.pcb.rem_time, running_process.pcb.waiting_time);
                            fflush(file);
                            // Fork a new process to execute the program
                            pid_t pid = fork();
                            if (pid < 0)
                            {
                                perror("Fork failed");
                                // exit(EXIT_FAILURE);
                            }
                            else if (pid == 0)
                            {
                                // Child process
                                char *const args[] = {"./process.out", NULL};
                                execv("./process.out", args);
                                perror("Execv failed");
                                // exit(EXIT_FAILURE);
                            }
                            else
                            {
                                // Parent process
                                remMsg.remaining_time = running_process.runtime;
                                remMsg.mtype = 36;
                                if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                                {
                                    perror("msgsnd");
                                    // exit(EXIT_FAILURE);
                                }
                                else
                                {
                                    printf("Message sent successfuuly;\n");
                                }
                                printf("Process %d started\n", running_process.id);
                                sleep(2);
                            }
                        }
                        else
                        {
                            kill(running_process.display, SIGCONT);
                            int total = running_process.runtime;
                            fprintf(file, "At time %d Process %d resumed arr %d total %d remain %d wait %d\n", getClk(), running_process.id, running_process.arrival_time, total, running_process.pcb.rem_time, running_process.pcb.waiting_time);
                            fflush(file);
                        }
                        // check if the ready queue is not empty , increment the waiting time of each
                        displaySRTNQueue(queue);
                    }
                    else if (running_process_id == -1 && getSRTNHead(queue).pcb.rem_time > message.process.pcb.rem_time)
                    {
                        printf("no running process and incoming rem time < head rem time\n");
                        // Fork a new process to execute the program
                        running_process = message.process;
                        running_process_id = message.process.id;
                        running_process.isForked = true;
                        printf("Process %d started\n", running_process.id);
                        int total = running_process.runtime;
                        fprintf(file, "At time %d Process %d started arr %d total %d remain %d wait %d\n", getClk(), running_process.id, running_process.arrival_time, total, running_process.pcb.rem_time, next_process.pcb.waiting_time);
                        fflush(file);
                        pid_t pid = fork();
                        if (pid < 0)
                        {
                            perror("Fork failed");
                            // exit(EXIT_FAILURE);
                        }
                        else if (pid == 0)
                        {
                            // Child process
                            char *const args[] = {"./process.out", NULL};
                            execv("./process.out", args);
                            perror("Execv failed");
                            // exit(EXIT_FAILURE);
                        }
                        else
                        {
                            // Parent process
                            remMsg.remaining_time = running_process.runtime;
                            remMsg.mtype = 36;
                            if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                            {
                                perror("msgsnd");
                                // exit(EXIT_FAILURE);
                            }
                            else
                            {
                                printf("Message sent successfuuly;\n");
                            }
                            printf("Process %d started\n", running_process.id);
                            sleep(2);
                        }
                        displaySRTNQueue(queue);
                    }
                    else
                    {
                        printf("there is a running process, 3adi geddan no special cases here\n");
                        // check if the ready queue is not empty , increment the waiting time of each
                        // // decrement the remaining time and check termination
                        // running_process.pcb.rem_time--;
                        // remMsg.remaining_time = running_process.pcb.rem_time;
                        // remMsg.mtype = 36;
                        // if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                        // {
                        //     perror("msgsnd");
                        //     // exit(EXIT_FAILURE);
                        // }
                        // else
                        // {
                        //     printf("Message sent successfuuly;\n");
                        // }
                        rdy_processCount++;
                        SRTNenqueue(queue, message.process);
                        displaySRTNQueue(queue);
                        // sleep(2);
                    }
                }
            }
            if (rdy_processCount > 0)
            {
                Process processCalc;
                Queue* waitingQueue;
                waitingQueue = createQueue();
                for (int i = 0; i < rdy_processCount; i++)
                {
                    Process processCalc = SRTNdequeue(queue);
                    enqueue(waitingQueue, processCalc);
                }
                for (int i = 0; i < rdy_processCount; i++)
                {
                    Process processCalc = dequeue(waitingQueue);
                    processCalc.pcb.waiting_time = processCalc.pcb.waiting_time + 1;
                    enqueue(waitingQueue, processCalc);
                }
                for (int i = 0; i < rdy_processCount; i++)
                {
                    Process processCalc = dequeue(waitingQueue);
                    SRTNenqueue(queue, processCalc);
                }
            }
            if (blk_processCount > 0)
            {
                for (int i = 0; i < blk_processCount; i++)
                {
                    Process processCalc = dequeue(blocked_processes);
                    processCalc.pcb.waiting_time = processCalc.pcb.waiting_time + 1;
                    enqueue(blocked_processes, processCalc);
                }   
            }    
            totalTime++;
            up(semid3);
            sleep(1);
        }
    }
    else if (algo == 3) // non-preemptive highest priority first
    {
        // Creating queue
        HPFQueue *queue;
        queue = createHPFQueue();
        int running_process_id = -1;
        Process running_process;
        Process next_process;
        Process blocked;

        // Receive process objects from the message queue
        while (finishedProcesses < processNum)
        {
            down(semid2);
            int sem_value;
            if ((sem_value = semctl(semid2, 0, GETVAL)) == -1)
            {
                perror("Error getting semaphore value");
                // exit(EXIT_FAILURE);
            }
            // Receive processes from the message queue
            if (msgrcv(msgq1_id, &message, sizeof(struct msgbuff), 7, IPC_NOWAIT) == -1)
            {
            }
            else // there was a process received at this second
            {
                printf("Message recieved successfully from process\n");
                printf("process id: %d \n", message.process.id);
                message.process.pcb.state = 0;
                message.process.pcb.waiting_time = 0;
                message.process.pcb.rem_time = message.process.runtime;
                printf("BEFORE THE BUDDY ALLOC\n");
                int process_adress = allocate(message.process.mem_size);
                printf("this is the process address %d \n", process_adress);
                printf("AFTER THE BUDDY ALLOC\n");
                if (process_adress == -1)
                {
                    printf("NO ENOUGH SPACE IN MEMORY, TRY AGAIN LATER\n");
                    enqueue(blocked_processes, message.process);
                    blk_processCount++;
                    printf("This is blocked queue: \n");
                    displayQueue(blocked_processes);
                }
                else
                {
                    message.process.address = process_adress;
                    printf("MEMORY ALLOCATED SUCCEFULLY AND THIS IS ITS ADDRESS: %d\n", message.process.address);
                    HPFenqueue(queue, message.process);
                    displayHPFQueue(queue);
                    rdy_processCount++;
                    int n = ceil(log(message.process.mem_size) / log(2));
                    int final_size = pow(2,n);
                    fprintf(mem_file,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),message.process.mem_size,message.process.id,message.process.address, message.process.address+final_size-1);
                    fflush(mem_file);
                }
            }
            // if there is no process running
            if (running_process_id == -1)
            {
                if (!isHPFEmpty(queue))
                {
                    next_process = HPFdequeue(queue);
                    running_process = next_process;
                    running_process_id = running_process.id;
                    rdy_processCount--;
                    pid_t pid = fork();
                    if (pid < 0)
                    {
                        perror("Fork failed");
                    }
                    else if (pid == 0)
                    {
                        // Child process
                        char *const args[] = {"./process.out", NULL};
                        execv("./process.out", args);
                        perror("Execv failed");
                    }
                    else
                    {
                        // Parent process
                        remMsg.remaining_time = next_process.runtime;
                        remMsg.mtype = 36;
                        if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                        {
                            perror("msgsnd");
                        }
                        else
                        {
                            printf("Message sent successfuly from scheduler to process\n");
                        }
                        printf("Process %d started\n", next_process.id);
                        // print state at process start in file

                        int total = next_process.runtime;
                        fprintf(file, "At time %d Process %d started arr %d total %d remain %d wait %d\n", getClk(), next_process.id, next_process.arrival_time, total, next_process.pcb.rem_time, next_process.pcb.waiting_time);
                        fflush(file);
                    }
                }
                else
                {
                    idleTime++;
                }
            }
            // there is a process running
            else
            {
                running_process.pcb.rem_time = running_process.pcb.rem_time - 1;
                remMsg.remaining_time = running_process.pcb.rem_time;
                remMsg.mtype = 36;
                if (msgsnd(msgid2, &remMsg, sizeof(remMsg) - sizeof(long), 0) == -1)
                {
                    perror("msgsnd");
                }
                else
                {
                    printf("Message sent from scheduler\n");
                }
                sleep(2);
            }

            // recieve terminated from process
            if (msgrcv(msgid, &processState, sizeof(processState), 80, IPC_NOWAIT) == -1)
            {
                // perror("msgrcv");
                // exit(EXIT_FAILURE);
            }
            else
            {
                finishedProcesses++;
                running_process.pcb.finish_time = getClk();
                printf("Finish Time = %d", running_process.pcb.finish_time);
                // print state at termination in file
                int total = running_process.runtime;
                int TA = running_process.pcb.finish_time - running_process.arrival_time;
                float WTA;
                if (running_process.runtime == 0)
                {
                    WTA = 0;
                }
                else
                {
                    WTA = TA / running_process.runtime;
                }
                fprintf(file, "At time %d Process %d finished arr %d total %d remain %d wait %d TA %d WTA %f\n", getClk(), running_process.id, running_process.arrival_time, total, running_process.pcb.rem_time, running_process.pcb.waiting_time, TA, WTA);
                fflush(file);
                printf("This is finish queue: \n");
                enqueue(finished, running_process);
                displayQueue(finished);
                running_process_id = -1;
                 // remove the allocated memory for the process
                deallocate(running_process.address);
                printf("the runnin gprocess anddress: %d\n", running_process.address);
                int n = ceil(log(running_process.mem_size) / log(2));
                int final_size = pow(2,n);
                fprintf(mem_file,"At time %d freed %d bytes from process %d from %d to %d\n", getClk(), running_process.mem_size, running_process.id,running_process.address,running_process.address+final_size-1);
                fflush(mem_file);
                // check the blocked queue for the first process to be allocated
                while (!isEmpty(blocked_processes))
                {
                    // we check if the there is a place for the first process in the blocked queue
                    // if yes, we dequeue the process and add it to the ready queue

                    if (checkAllocation(blocked_processes->front->data.mem_size) == 1)
                    {
                        blocked = dequeue(blocked_processes);
                        blk_processCount--;
                        printf("blocked process dequeued: %d\n", blocked.id);
                        printf("BEFORE THE BUDDY ALLOC\n");
                        int process_adress = allocate(blocked.mem_size);
                        blocked.address = process_adress;
                        HPFenqueue(queue,blocked);
                        rdy_processCount++;
                        displayHPFQueue(queue);
                        printf("blocked process address: %d\n",blocked.address);
                        printf("this is the process address %d \n", process_adress);
                        printf("AFTER THE BUDDY ALLOC\n");
                        int n = ceil(log(blocked.mem_size) / log(2));
                        int final_size = pow(2,n);
                        fprintf(mem_file,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),blocked.mem_size,blocked.id,process_adress,process_adress+final_size-1);
                        fflush(mem_file);
                    }
                    else
                    {
                        break;
                    }
                }
                up(semid2);
                continue;
            }
            if (rdy_processCount > 0)
            {
                Process processCalc;
                Queue* waitingQueue;
                waitingQueue = createQueue();
                for (int i = 0; i < rdy_processCount; i++)
                {
                    Process processCalc = HPFdequeue(queue);
                    enqueue(waitingQueue, processCalc);
                }
                for (int i = 0; i < rdy_processCount; i++)
                {
                    Process processCalc = dequeue(waitingQueue);
                    processCalc.pcb.waiting_time = processCalc.pcb.waiting_time + 1;
                    enqueue(waitingQueue, processCalc);
                }
                for (int i = 0; i < rdy_processCount; i++)
                {
                    Process processCalc = dequeue(waitingQueue);
                    HPFenqueue(queue, processCalc);
                }
            }
            if (blk_processCount > 0)
            {
                for (int i = 0; i < blk_processCount; i++)
                {
                    Process processCalc = dequeue(blocked_processes);
                    processCalc.pcb.waiting_time = processCalc.pcb.waiting_time + 1;
                    enqueue(blocked_processes, processCalc);
                }   
            }            
            totalTime++;
            up(semid3);
        }
    }
    fclose(file);
    fclose(mem_file);
    // creates perf file
    perf_file = fopen("perf.txt", "w");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    // Calc cpu utilization
    float cpuUti = ((totalTime - idleTime) / totalTime) * 100;
    // print cpu utilization to file
    fprintf(perf_file, "CPU utilization = %0.4f %%\n", cpuUti);
    fflush(perf_file);
    float totalWTA = 0;
    float totalWT = 0;
    // Calc total WTA and WT
    for (int i = 0; i < details.processesNum; i++)
    {
        Process processCalc = dequeue(finished);
        if (processCalc.runtime == 0)
        {
            continue;
        }
        printf("WTA %d: %f\n", i, (processCalc.pcb.finish_time - processCalc.arrival_time) / processCalc.runtime);
        totalWTA += (processCalc.pcb.finish_time - processCalc.arrival_time) / processCalc.runtime;
        printf("WT %d: \n", processCalc.pcb.waiting_time);
        totalWT += processCalc.pcb.waiting_time;
        enqueue(finished, processCalc);
    }

    // Calc avg. weighted turnaround time
    float avg_WTA = totalWTA / details.processesNum;
    printf("This is Avg WTA = %0.4f\n", avg_WTA);
    float sum = 0;
    for (int i = 0; i < details.processesNum; i++)
    {
        Process processCalc = dequeue(finished);
        float temp1 = (processCalc.pcb.finish_time - processCalc.arrival_time) / processCalc.runtime;
        float temp2 = pow(temp1 - avg_WTA, 2);
        sum += temp2;
        enqueue(finished, processCalc);
    }
    float variance = sum / details.processesNum;
    float StdDev = sqrt(variance);

    // prints avg weighted turnaround time in file
    fprintf(perf_file, "Avg WTA = %0.4f\n", avg_WTA);
    fflush(perf_file);
    // Calc avg. waiting time
    float avg_WT = totalWT / details.processesNum;
    // Prints avg. waiting time in file
    printf("This is avg waiting = %0.4f\n", avg_WT);
    fprintf(perf_file, "Avg Waiting = %0.4f\n", avg_WT);
    fflush(perf_file);
    printf("STDWTA %f\n", StdDev);
    fprintf(perf_file, "Std WTA = %0.4f\n", StdDev);
    fflush(perf_file);
    // fclose(perf_file);

    // TODO: Implement standard deviation
    destroyClk(true);
}
