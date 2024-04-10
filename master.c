#include "header.h"
struct sembuf pop = {0, -1, 0}, vop = {0, 1, 0};
#define sem_wait(s) semop(s, &pop, 1)
#define sem_signal(s) semop(s, &vop, 1)
int *M, *P;
int *m, *f, *k;
int *frame_alloc, *start_alloc, *SM2;
pg_entry *SM1;
int lock;

int MQ1, MQ2, MQ3;
int shmid_P, shmid_M, shmid_m, shmid_f, shmid_k, shmid_frame_alloc, shmid_start_alloc, shmid_SM1, shmid_SM2;
int semid_over, semid_process;
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

    shmid_M = shmget(M_KEY, (*k) * sizeof(int), IPC_CREAT | 0666);
    if (shmid_M == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    M = (int *)shmat(shmid_M, NULL, 0);
    if (M == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    shmid_P = shmget(P_KEY, (*k) * sizeof(int), IPC_CREAT | 0666);
    if (shmid_P == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    P = (int *)shmat(shmid_P, NULL, 0);
    if (P == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    initSemaphore(&lock, lock_KEY, 1);
}

void ATTACH1()
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
    shmid_m = shmget(m_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shmid_m == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    m = (int *)shmat(shmid_m, NULL, 0);
    if (m == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    shmid_f = shmget(f_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shmid_f == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    f = (int *)shmat(shmid_f, NULL, 0);
    if (f == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
}

void ATTACH2()
{
    shmid_frame_alloc = shmget(frame_alloc_KEY, (*k) * sizeof(int), IPC_CREAT | 0666);
    if (shmid_frame_alloc == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    frame_alloc = (int *)shmat(shmid_frame_alloc, NULL, 0);
    if (frame_alloc == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    shmid_start_alloc = shmget(start_alloc_KEY, (*k) * sizeof(int), IPC_CREAT | 0666);
    if (shmid_start_alloc == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    start_alloc = (int *)shmat(shmid_start_alloc, NULL, 0);
    if (start_alloc == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    shmid_SM1 = shmget(SM1_KEY, (*k) * (*m) * sizeof(pg_entry), IPC_CREAT | 0666);
    if (shmid_SM1 == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    SM1 = (pg_entry *)shmat(shmid_SM1, NULL, 0);
    if (SM1 == (pg_entry *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < (*k) * (*m); i++)
    {
        SM1[i].valid_bit = 0;
        SM1[i].frame_no = -1;
    }
    shmid_SM2 = shmget(SM2_KEY, (*f) * sizeof(int), IPC_CREAT | 0666);
    if (shmid_SM2 == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    SM2 = (int *)shmat(shmid_SM2, NULL, 0);
    if (SM2 == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < *f; i++)
    {
        SM2[i] = -1;
    }
    initSemaphore(&semid_over, semidover_KEY, 0);
    MQ1 = msgget(MQ1_KEY, IPC_CREAT | 0666);
    if (MQ1 == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    MQ2 = msgget(MQ2_KEY, IPC_CREAT | 0666);
    if (MQ2 == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    MQ3 = msgget(MQ3_KEY, IPC_CREAT | 0666);
    if (MQ3 == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
}

void DETACH()
{
    // Detach and destroy shared memory segments
    shmdt(M);
    shmctl(shmid_M, IPC_RMID, NULL);
    shmdt(P);
    shmctl(shmid_P, IPC_RMID, NULL);
    shmdt(k);
    shmctl(shmid_k, IPC_RMID, NULL);
    shmdt(m);
    shmctl(shmid_m, IPC_RMID, NULL);
    shmdt(f);
    shmctl(shmid_f, IPC_RMID, NULL);
    shmdt(frame_alloc);
    shmctl(shmid_frame_alloc, IPC_RMID, NULL);
    shmdt(start_alloc);
    shmctl(shmid_start_alloc, IPC_RMID, NULL);
    shmdt(SM1);
    shmctl(shmid_SM1, IPC_RMID, NULL);
    shmdt(SM2);
    shmctl(shmid_SM2, IPC_RMID, NULL);

    // Destroy semaphores
    semctl(semid_over, 0, IPC_RMID);
    semctl(lock, 0, IPC_RMID);

    // Destroy message queues
    msgctl(MQ1, IPC_RMID, NULL);
    msgctl(MQ2, IPC_RMID, NULL);
    msgctl(MQ3, IPC_RMID, NULL);
}
int generate_random_int(int min_value, int max_value)
{
    return rand() % (max_value - min_value + 1) + min_value;
}
int main(int argc, char *argv[])
{
    ATTACH1();
    printf("Enter process number, virtual address space size and frame number: ");
    scanf("%d%d%d", k, m, f);
    if (*f < *k)
    {
        printf("Too less frames\n");
        return 0;
    }

    printf("Master Process Alive !!!\n");
    printf("CMD line arguments --> process: %d, max_pages : %d, frames : %d\n", *k, *m, *f);
    ATTACH2();
    ATTACH();
    for (int i = 0; i < *f; i++)
    {
        SM2[i] = -1;
    }
    for (int i = 0; i < ((*k) * (*m)); i++)
    {
        SM1[i].valid_bit = 0;
    }

    int schedPID = fork();
    if (schedPID == 0)
    {
        char cMQ1[20];
        sprintf(cMQ1, "%d", MQ1);
        char cMQ2[20];
        sprintf(cMQ2, "%d", MQ2);
        execlp("./scheduler", "scheduler", cMQ1, cMQ2, NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    printf("Scheduler (PID = %d) created\n", schedPID);
    int mmuPID = fork();
    if (mmuPID == 0)
    {
        char cMQ2[20];
        sprintf(cMQ2, "%d", MQ2);
        char cMQ3[20];
        sprintf(cMQ3, "%d", MQ3);
        char cSM1[20];
        sprintf(cSM1, "%d", shmid_SM1);
        char cSM2[20];
        sprintf(cSM2, "%d", shmid_SM2);
        // execlp("./mmu", "mmu", cMQ2, cMQ3, cSM1, cSM2, NULL);
        execl("/usr/bin/xterm", "/usr/bin/xterm", "-T", "MMU", "-e", "./mmu", cMQ2, cMQ3, cSM1, cSM2, NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    printf("MMU (PID = %d) created\n", mmuPID);

    // -----------------------------------------------------
    srand(time(NULL)); // Seed the random number generator
    int sum_M = 0;
    for (int i = 0; i < *k; i++)
    {
        M[i] = generate_random_int(1, *m);
        sum_M += M[i];
    }
    for (int i = 0; i < *k; i++)
    {
        frame_alloc[i] = 1;
        if (sum_M - (*k) > 0)
            frame_alloc[i] += ((M[i] - 1) * ((*f) - (*k))) / (sum_M - (*k));
    }
    for (int i = 0; i < *k; i++)
    {
        if (i == 0)
            start_alloc[i] = 0;
        else
            start_alloc[i] = start_alloc[i - 1] + frame_alloc[i - 1];
    }
    printf("Num of pages for processes : ");
    for (int i = 0; i < *k; i++)
    {
        printf("%d ", M[i]);
    }
    printf("\n");
    printf("Frame allocation for processes : ");
    for (int i = 0; i < *k; i++)
    {
        printf("%d ", frame_alloc[i]);
    }
    printf("\n");
    printf("Starting indices for process-frames : ");
    for (int i = 0; i < *k; i++)
    {
        printf("%d ", start_alloc[i]);
    }
    printf("\n");

    int cpid[*k];
    for (int i = 0; i < *k; i++)
    {
        P[i] = generate_random_int(2 * M[i], 10 * M[i]);
        char ref_string[1000];
        int length = 0;
        for (int j = 0; j < P[i]; j++)
        {
            if (j > 0)
            {
                length += sprintf(ref_string + length, " ");
            }
            int page_no = generate_random_int(1, M[i]);
            double invalid = (double)rand() / RAND_MAX;
            if (invalid <= PROB)
            {
                page_no = generate_random_int(M[i] + 1, *m + 1);
            }
            length += sprintf(ref_string + length, "%d", page_no);
        }
        ref_string[length] = '\0'; // Add null terminator
        pid_t pid = fork();
        cpid[i] = pid;
        if (pid == -1)
        {
            // Error occurred
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Child process
            char idx[10];
            sprintf(idx, "%d", i);
            char cMQ1[20];
            sprintf(cMQ1, "%d", MQ1);
            char cMQ3[20];
            sprintf(cMQ3, "%d", MQ3);
            execlp("./process", "process", idx, ref_string, cMQ1, cMQ3, NULL);
            perror("execlp process");
            exit(EXIT_FAILURE);
        }
        printf("Master forks Process - %d\n", i);
        usleep(250000);
    }
    sem_wait(semid_over);
    sleep(10);
    kill(mmuPID, SIGINT);
    printf("Master kills MMU !!!\n");
    kill(schedPID, SIGINT);
    printf("Master kills scheduler !!!\n");
    waitpid(mmuPID, NULL, 0);
    waitpid(schedPID, NULL, 0);
    for (int i = 0; i < *k; i++)
    {
        waitpid(cpid[i], NULL, 0);
    }
    printf("All processes dead\n");
    printf("Master returns\n");
    DETACH();
}
