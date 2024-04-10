#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/wait.h>
typedef struct pg_entry
{
    int valid_bit;
    int frame_no;
} pg_entry;
#define PROB 0.01
#define SM1_KEY ftok(".", 100)
#define frame_alloc_KEY ftok(".", 101)
#define start_alloc_KEY ftok(".", 102)
#define SM2_KEY ftok(".", 103)
#define m_KEY ftok(".", 104)
#define k_KEY ftok(".", 105)
#define f_KEY ftok(".", 106)
#define process_KEY ftok(".", 1000)
#define MQ1_KEY ftok(".", 108)
#define MQ2_KEY ftok(".", 109)
#define MQ3_KEY ftok(".", 110)
#define M_KEY ftok(".", 111) // Shared memory key for M
#define P_KEY ftok(".", 112) // Shared memory key for P
#define semidover_KEY ftok(".", 113) 
#define lock_KEY ftok(".", 114)
#define frameval_KEY ftok(".", 115)
struct msg_buffer
{
    long msg_type;
    int msg_data;
};