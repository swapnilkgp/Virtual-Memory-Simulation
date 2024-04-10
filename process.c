#include "header.h"
struct sembuf pop={0,-1,0}, vop={0,1,0};
#define sem_wait(s) semop(s, &pop, 1)
#define sem_signal(s) semop(s, &vop, 1)
int *semid_process, *frameval;
int *k;
int shmid_k, shmid_frameval;
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
    shmid_frameval = shmget(frameval_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shmid_frameval == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    frameval = (int *)shmat(shmid_frameval, NULL, 0);
    if (frameval == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
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
    initSemaphore(&lock,lock_KEY,1);

}

int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        return -1;
    }
    ATTACH();
    int pid = atoi(argv[1]);
    semid_process = (int*)malloc((*k)*sizeof(int));
    for (int i = 0; i < *k; i++)
    {
        initSemaphore(semid_process+i, process_KEY + i, 0);
    }
    printf("\033[1;35mProcess %d is ALIVE !!\033[0m\n", pid);
    printf("\033[1;35mReference str for process - %d: %s\033[0m\n", pid, argv[2]);
    char *word;
    int page_no;
    word = strtok(argv[2], " \0");
    int MQ1 = atoi(argv[3]);
    int MQ3 = atoi(argv[4]);
   
    push(MQ1, pid);
    printf("\033[1;35mProcess-%d puts itself in Ready Queue\033[0m\n", pid);
    sem_wait(semid_process[pid]);
    while (word != NULL)
    {
        int page_no = atoi(word);
        printf("\033[1;35mProcess-%d requesting Page No : %d from MMU\033[0m\n", pid, page_no);
        push(MQ3, pid);
        push(MQ3, page_no);
        sem_wait(semid_process[pid]);
        int frame_no = *frameval;    
        printf("\033[1;35mProcess-%d receives reply frame No : %d from MMU\033[0m\n", pid, frame_no);
        if (frame_no == -1)
            sem_wait(semid_process[pid]);
        else if (frame_no == -2)
        {
            exit(EXIT_FAILURE);
        }
        else
        {
            word = strtok(NULL, " \0");
        }
    }
    push(MQ3, pid);
    push(MQ3, -9);
}

