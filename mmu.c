#include "header.h"
struct sembuf pop = {0, -1, 0}, vop = {0, 1, 0};
#define sem_wait(s) semop(s, &pop, 1)
#define sem_signal(s) semop(s, &vop, 1)
int *M, *P;
int *m, *f, *k;
int *frame_alloc, *start_alloc, *SM2;
pg_entry *SM1;
int timestamp;
int MQ1, MQ2, MQ3;
int shmid_P, shmid_M, shmid_m, shmid_f, shmid_k, shmid_frame_alloc, shmid_start_alloc, shmid_SM1, shmid_SM2, shmid_frameval;
int *semid_process;
int lock;
int *frameval;
FILE *fd;
int *A, *B;
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
void ATTACH1()
{
    // printf("ATTACH1\n");
    SM1 = (pg_entry *)shmat(shmid_SM1, NULL, 0);
    if (SM1 == (pg_entry *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    SM2 = (int *)shmat(shmid_SM2, NULL, 0);
    if (SM2 == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    initSemaphore(&lock, lock_KEY, 1);
    // printf("ATTACH1\n");
}
void ATTACH2()
{
    // printf("ATTACH2\n");
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

    semid_process = (int *)malloc((*k) * sizeof(int));
    for (int i = 0; i < *k; i++)
    {
        initSemaphore(semid_process + i, process_KEY + i, 0);
    }

    // printf("ATTACH2\n");
}

void PFH(int process_no, int page_no)
{
    int mnval = SM2[start_alloc[process_no]];
    int replaced_idx = start_alloc[process_no];
    for (int i = start_alloc[process_no]; i < start_alloc[process_no] + frame_alloc[process_no]; i++)
    {
        if (SM2[i] < mnval)
        {
            mnval = SM2[i];
            replaced_idx = i;
        }
    }
    if (SM2[replaced_idx] != -1)
    {
        for (int i = 0; i < (*m); i++)
        {
            int validbit = SM1[process_no * (*m) + i].valid_bit;
            int frame_no = SM1[process_no * (*m) + i].frame_no;
            if (frame_no == replaced_idx && validbit)
            {
                SM1[process_no * (*m) + i].valid_bit = 0;
            }
        }
    }

    SM2[replaced_idx] = timestamp;
    SM1[process_no * (*m) + page_no - 1].valid_bit = 1;
    SM1[process_no * (*m) + page_no - 1].frame_no = replaced_idx;
}
void printDATA()
{
    for (int i = 0; i < *k; i++)
    {
        printf("Information for Process - %d\n", i);
        printf("\t\tNumber of page faults : %d\n", B[i]);
        printf("\t\tNumber of invalid page references : %d\n", A[i]);
        fprintf(fd, "Information for Process - %d\n", i);
        fflush(fd);
        fprintf(fd, "\t\tNumber of page faults : %d\n", B[i]);
        fflush(fd);
        fprintf(fd, "\t\tNumber of invalid page references : %d\n", A[i]);
        fflush(fd);
    }
    fclose(fd);
}
void DETACH()
{
    if (shmdt(frameval) == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    // Remove the shared memory segment
    if (shmctl(shmid_frameval, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
}
int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        printf("Less args\n");
        return -1;
    }
    printf("\033[1;34mMMU is ALIVE !!.\033[0m\n");
    MQ2 = atoi(argv[1]);
    MQ3 = atoi(argv[2]);
    shmid_SM1 = atoi(argv[3]);
    shmid_SM2 = atoi(argv[4]);
    ATTACH1();
    ATTACH2();
    A = (int *)calloc(*k, sizeof(int));
    B = (int *)calloc(*k, sizeof(int));
    fd = fopen("result.txt", "w");
    fflush(stdout);
    int ndone = 0;
    while (1)
    {
        if (ndone == *k)
        {
            printDATA();
            DETACH();
        }
        int process_no = erase(MQ3);
        int page_no = erase(MQ3);
        printf("\033[1;34mGlobal Ordering {Timestamp: %d, Process_no : %d, page_no : %d} as request\033[0m\n", timestamp, process_no, page_no);
        fprintf(fd, "Global Ordering {Timestamp: %d, Process_no : %d, page_no : %d} as request\n", timestamp, process_no, page_no);
        fflush(fd);
        timestamp++;
        if (page_no == -9)
        {
            ndone++;
            push(MQ2, 2);
        }
        else if (page_no > M[process_no] || page_no <= 0)
        {
            A[process_no]++;
            printf("\033[1;34mInvalid Page Reference {Timestamp: %d, Process_no : %d, page_no : %d} as request\033[0m\n", timestamp, process_no, page_no);
            fprintf(fd, "Invalid Page Reference {Timestamp: %d, Process_no : %d, page_no : %d} as request\n", timestamp, process_no, page_no);
            fflush(fd);

            // push(MQ3, -2);
            *frameval = -2;
            sem_signal(semid_process[process_no]);
            push(MQ2, 2);
            ndone++;
        }
        else
        {
            int frame_no = SM1[process_no * (*m) + page_no - 1].frame_no;
            int valid_bit = SM1[process_no * (*m) + page_no - 1].valid_bit;
            if (valid_bit)
            {
                // push(MQ3, frame_no);
                *frameval = frame_no;

                sem_signal(semid_process[process_no]);
            }
            else
            {
                B[process_no]++;
                printf("\033[1;34mPage Fault {Timestamp: %d, Process_no : %d, page_no : %d} as request\033[0m\n", timestamp, process_no, page_no);
                fprintf(fd, "Page Fault {Timestamp: %d, Process_no : %d, page_no : %d} as request\n", timestamp, process_no, page_no);
                fflush(fd);

                PFH(process_no, page_no);
                *frameval = -1;
                sem_signal(semid_process[process_no]);
                push(MQ2, 1);
            }
        }
    }
    return 0;
}