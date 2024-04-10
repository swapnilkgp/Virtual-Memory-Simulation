#include "header.h"
struct sembuf pop = {0, -1, 0}, vop = {0, 1, 0};
#define sem_wait(s) semop(s, &pop, 1)
#define sem_signal(s) semop(s, &vop, 1)
int *semid_process, semid_over;
int *k;
int shmid_k;
int lock;
int push(int mq_id, int data)
{
    // Create a message buffer

    sem_wait(lock);
    struct msg_buffer message;
    // sg_type = 1; // Message type (arbitrary)
    message.msg_data = data;
    message.msg_type = 2;

    // Send the message
    if (msgsnd(mq_id, &message, sizeof(message), 0) == -1)
    {
        perror("msgsnd");
        return -1; // Error occurred
    }
    sem_signal(lock);
    return 0; // Success
}

// Function to pop an integer from the message queue
int erase(int mq_id)
{
    // Receive the message
    struct msg_buffer message;
    if (msgrcv(mq_id, &message, sizeof(message), 2, 0) == -1)
    {
        perror("msgrcv");
        return -1; // Error occurred
    }
    // Return the data from the received message
    return message.msg_data;
}
void initSemaphore(int *semid, int key, int initValue)
{

    if ((*semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666)) != -1)
    {

        int arg = initValue;
        if (semctl(*semid, 0, SETVAL, arg) == -1)
        {
            perror("semctl");
            exit(EXIT_FAILURE);
        }
    }
    else
    {

        if ((*semid = semget(key, 1, 0)) == -1)
        {
            perror("semget");
            exit(EXIT_FAILURE);
        }
    }
    return;
}

void ATTACH()
{
    shmid_k = shmget(k_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shmid_k == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    k = (int *)shmat(shmid_k, NULL, 0);
    if (k == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    semid_process = (int *)malloc((*k) * sizeof(int));
    for (int i = 0; i < *k; i++)
    {
        initSemaphore(semid_process + i, process_KEY + i, 0);
    }
    initSemaphore(&semid_over, semidover_KEY, 0);
    initSemaphore(&lock, lock_KEY, 1);
}
void DETACH()
{
    for (int i = 0; i < *k; i++)
    {
        if (semid_process[i] != -1)
        {
            if (semctl(semid_process[i], 0, IPC_RMID) == -1)
            {
                perror("semctl");
                exit(EXIT_FAILURE);
            }
        }
    }
    free(semid_process);
}
void sighandler(int sig)
{
    DETACH();
    exit(0);
}
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Less args\n");
        return -1;
    }
    int MQ1 = atoi(argv[1]);
    int MQ2 = atoi(argv[2]);
    signal(SIGINT, sighandler);
    ATTACH();
    printf("\033[1;36mScheduler is ALIVE !!\n");
    int ndone = 0;
    while (1)
    {

        if (ndone == (*k))
        {
            sem_signal(semid_over);
        }
        int curr_process = erase(MQ1);
        printf("\033[1;36mProcess - %d is scheduled by scheduler\033[0m\n", curr_process);
        fflush(stdout);
        sem_signal(semid_process[curr_process]);
        if (curr_process == -1)
        {
            fprintf(stderr, "Failed to pop data from MQ1.\033[0m\n");
            continue; // Continue waiting
        }
        int msg_type = erase(MQ2);
        if (msg_type == 2)
        {
            printf("\033[1;36mProcess - %d is terminated. Erased permanently from Ready queue\033[0m\n", curr_process);
            ndone++;
        }
        else
        {
            push(MQ1, curr_process);
            printf("\033[1;36mProcess - %d is inserted at back of ready queue\033[0m\n", curr_process);
        }
    }
    return 0;
}
