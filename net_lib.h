#ifndef _network_lib_h
#define _network_lib_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/stat.h>
#include <signal.h>
#include <openssl/md5.h>
#include <pthread.h>

#define BUFSIZE 1024
#define SLEEP_VAL 1
static int alarm_fired = 0;
#define MAX 1000
#define MAX1 100000000
typedef struct m
{
  unsigned char buff[1024];
  int nread;
}my_type;
//////////////////////defining queue struct/////////////////////////
typedef struct dequeue
{
  my_type data[MAX];
  int rear,front;
}dequeue;
typedef struct dq2
{
  unsigned char data[MAX1];
  int front,rear;
}dequeue1;

////////////////////prototypes for queue function //////////////////
void initialize(dequeue *p);
int empty(dequeue *p);
int full(dequeue *p);
void enqueueR(dequeue *p,my_type x);
void enqueueF(dequeue *p,my_type x);
my_type dequeueF(dequeue *p);
my_type dequeueR(dequeue *p);
void print(dequeue *p);

void initialize1(dequeue1 *p);
int empty1(dequeue1 *p);
int full1(dequeue1 *p);
void enqueueR1(dequeue1 *p,unsigned char x);
void enqueueF1(dequeue1 *p,unsigned char x);
unsigned char dequeueF1(dequeue1 *p);
unsigned char dequeueR1(dequeue1 *p);
void print1(dequeue1 *p);


////////////////////global variable declaration ///////////////////////////
int dsize=0;//size od congestion window
int sqsize=0;//size of sender buffer
int rqsize=0;//size of receiver buffer
dequeue1 sq;//sender queue
dequeue1 rq;//receiver queue
dequeue dq;//queue for handling windows
pthread_mutex_t slock;//lock for acessing sender queue
pthread_mutex_t rlock;//lock for acessing receiver queue
pthread_mutex_t salock;//sender availability lock
pthread_mutex_t ralock;//receiver availability lock
pthread_mutex_t crlock;//lock for other global variables
int sockfd, portno, n,i,ll;
int serverlen;
int fileSize;
int sending_len;
struct sockaddr_in serveraddr;
struct hostent *server;
char *hostname;
unsigned long tcount;//total count of packets
unsigned long total_bytes=0;//total number of bytes
unsigned long ccount=0;//current count of packet that sender is expecting ack of
unsigned long ccountr=0;//current count receiver is expecting of
unsigned long rcount=0;//rcount is the byte number that is now acknowledged
unsigned long scount=0;//the byte which we will be sending
unsigned long tmpcount;
unsigned long rqsize_receiver=1000;//rqsize remaining on receiver in terms of kb
int ssthresh=500;//initial ssthresh is very big
int dupcount=0;//cont of dup acks
int berror;//denotes if packet loss was there
int bl=0;
double wsize=3;//denotes window size
int sending_len; /* byte size of client's address */
struct sockaddr_in sending_addr; /* client addr */
struct hostent *hostp; /* client host info */
char *hostaddrp; /* dotted decimal host addr string */
int optval; /* flag value for setsockopt */
FILE * fd;
int n; /* message byte size */
int buf[BUFSIZE];
pthread_attr_t attr;
pthread_t receivert;//thread for receiver
pthread_t congestion_controlt;//thread for congestion control
float drop_prob=0.0;
///////////////////////////prototype definitions ////////////////////////////
void my_cpy(unsigned char *b1,unsigned char *b2);
void error(char *msg);
void mysig(int sig);
void initialize_network();
void exit_as_server();
void exit_as_client();
void act_as_server(char *port);
void act_as_client(char *host,char *port);
void* congestion_control(void *param);
int app_send(int bytes,unsigned char *buffs);
int app_recv(int bytes,unsigned char *buffs);
void *receiver_thread(void *param);
void file_input_hello();
void send_hello_ack_fopen();
#endif
