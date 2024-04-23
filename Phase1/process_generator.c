#include "headers.h"

#define MAX_LINE_LENGTH 100

void clearResources(int);
void read_processes(Process **processes, int *num_processes)
{
    FILE *file = fopen("processes.txt", "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", "processes.txt");
        exit(EXIT_FAILURE);
    }

    int capacity = 10;
    int num_lines = 0;
    char line[MAX_LINE_LENGTH];

    // Count the number of lines in the file
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (line[0] != '#' && line[0] != '\n')
        {
            num_lines++;
        }
    }

    // Move file pointer back to the beginning of the file
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the processes array
    *processes = (Process *)malloc(num_lines * sizeof(Process));
    if (*processes == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    *num_processes = 0;

    // Read processes from the file
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (line[0] == '#' || line[0] == '\n')
        {
            continue; // Skip comments and empty lines
        }

        Process process;
        int fields_read = sscanf(line, "%d\t%d\t%d\t%d", &process.id, &process.arrival_time, &process.runtime, &process.priority);
        if (fields_read != 4)
        {
            fprintf(stderr, "Invalid line format: %s\n", line);
            exit(EXIT_FAILURE);
        }

        (*processes)[*num_processes] = process;
        (*num_processes)++;

        if (*num_processes >= num_lines)
        {
            // Reallocate memory if needed
            capacity *= 2;
            *processes = (Process *)realloc(*processes, capacity * sizeof(Process));
            if (*processes == NULL)
            {
                fprintf(stderr, "Memory reallocation failed\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // initialize semaphore
    int semid1 = semget(server_sem_key, 1, IPC_CREAT | 0666);
    if (semid1 == -1)
    {
        perror("semget");
    }

    union Semun semun;

    semun.val = 0; /* initial value of the semaphore, Binary semaphore */
    if (semctl(semid1, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }

    // initialize message queue

    key_t key_id;
    int msgq_id, send_val;

    key_id = ftok("keyfile", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    printf("Message Queue ID = %d\n", msgq_id);

    struct msgbuff message;

    message.mtype = 7; /* arbitrary value */

    // 1. Read the input files.
    Process *processes;
    int num_processes;

    read_processes(&processes, &num_processes);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int algo;
    int quanta;
    printf("Please enter the desired scheuling algorithm: \n (1) for RR \n (2) for SRTN \n (3) for HPF\n");
    scanf("%d", &algo);
    if (algo == 1)
    {
        printf("Please enter the quanta for the RR algorithm\n");
        scanf("%d", &quanta);
    }
    // 3. Initiate and create the scheduler and clock processes.
    int pid = fork();
    if (pid < 0)
    {
        // Fork failed
        perror("Fork failed");
        exit(-1);
    }
    else if (pid == 0)
    {
        // Child process
        char *const args[] = {"./clk.out", NULL}; // No extra arguments
        execv("./clk.out", args);                 // Execute "../clk.out" with no extra arguments
        // If execv succeeds, code beyond this point will not be executed.
        // If execv fails, we print an error message.
        perror("Execv failed");
        exit(EXIT_FAILURE);
    }
    int pid2 = fork();
    if (pid2 < 0)
    {
        // Fork failed
        perror("Fork failed");
        exit(-1);
    }
    else if (pid2 == 0)
    {
        // Child process
        char *const args[] = {"./scheduler.out", NULL}; // No extra arguments
        execv("./scheduler.out", args);                 // Execute "../clk.out" with no extra arguments
        // If execv succeeds, code beyond this point will not be executed.
        // If execv fails, we print an error message.
        perror("Execv failed");
        exit(EXIT_FAILURE);
    }
    // 4. Use this function after creating the clock process to initialize clock
    down(semid1);
    initClk();
    // To get time use this
    int currentTime = getClk();
    printf("current time is %d\n", currentTime);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    int processCounter = 0;
    printf("%d \n", processes[processCounter].arrival_time);
    while (1)
    {
        
        currentTime = *shmaddr;
        printf("current time: %d \n", currentTime);
        // handle if many processes arrived at the same time
        if (currentTime == processes[processCounter].arrival_time)
        {
            printf("in the condition\n");
            // Pass the process object to the message queue
            message.process = processes[processCounter];
            send_val = msgsnd(msgq_id, &message, sizeof(message.process) - sizeof(long), !IPC_NOWAIT);
            if (send_val == -1)
            {
                perror("Error sending message");
                exit(EXIT_FAILURE);
            }
            else
            {
                printf("message sent\n");
            }
            processCounter++;
            
        }
        sleep(1);
    }
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
}
