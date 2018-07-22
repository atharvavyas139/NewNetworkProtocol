#include "net_lib.h"
void my_cpy(unsigned char *b1,unsigned char *b2)
{
  int i;
  for(i=0;i<1024;++i)
    {
      b1[i]=b2[i];
    }
}
void mysig(int sig)
{
  pid_t pid;
  if (sig == SIGALRM)
    {
      alarm_fired = 1;
    }
}
void send_hello_ack_fopen()
{
  char hello_msg[BUFSIZE],filename[BUFSIZE],size[BUFSIZE],ack_mesg[BUFSIZE],tcounts[BUFSIZE];
  int i;
  bzero(hello_msg,BUFSIZE);
  bzero(ack_mesg,BUFSIZE);
  bzero(filename,BUFSIZE);
  bzero(size,BUFSIZE);
  bzero(buf, BUFSIZE);
  n = recvfrom(sockfd,hello_msg, BUFSIZE, 0,(struct sockaddr *) &sending_addr, &sending_len);
  if (n < 0)
    error("ERROR in recvfrom");
  char *token = strtok(hello_msg, ",");
  i=0;
  while (token != NULL && i<=3){
    printf("%s \n", token);
    if(i==1){
      strcpy(filename,token);
    }
    if(i==2)strcpy(size,token);
    if(i==3)strcpy(tcounts,token);
    token = strtok(NULL, ",");
    i++;
  }
  fileSize=atoi(size);
  //total_bytes=fileSize;
  tcount=atoi(tcounts);
  printf("tcount:%d\n",tcount);
  strcpy(ack_mesg,"ACK MSG\n");
  n=sendto(sockfd,ack_mesg,strlen(ack_mesg),0,(struct sockaddr *) &sending_addr, sending_len);
  if(n<0)
    printf("error in sendto");
  printf("ack sent\n");
  printf("%d size\n",fileSize);
  char cwd[1024],buffer[256];
  strcpy(cwd,filename);
  fd = fopen(cwd, "wb");
}
void file_input_hello()
{
  char hello_msg[BUFSIZE],fileName[BUFSIZE],hello[10],ack_mesg[BUFSIZE];
  int filesize;
  strcpy(hello,"hello\0");
  printf("Please enter filename: ");
  bzero(fileName,BUFSIZE);
  scanf("%s",fileName);
  fd=fopen(fileName,"rb");
  while(fd==NULL){
    printf("File do no exist, Enter again : ");
    bzero(fileName,BUFSIZE);
    scanf("%s",fileName);
    fd=fopen(fileName,"rb");
  }
  struct stat st;
  stat(fileName, &st);
  filesize= st.st_size;
  total_bytes=filesize;////
  if(filesize%1016==0)//2 commas and two ints
    tcount=filesize/1016;
  else
    tcount=filesize/1016+1;
  sprintf(hello_msg,"%s,%s,%d,%d",hello,fileName,filesize,tcount);
  printf("%s,",hello_msg);
  /*sending hello_msg*/
  sending_len = sizeof(sending_addr);

  n = sendto(sockfd,hello_msg,BUFSIZE, 0, &sending_addr, sending_len);//sending server hello_msg

  if (n < 0) 
    error("ERROR in sendto");

  n = recvfrom(sockfd,ack_mesg,strlen(ack_mesg), 0,&sending_addr, &sending_len);//receiving acknowledge msg
  if(n<0)
    printf("error in recv from\n");
  printf("%s",ack_mesg);
}
void act_as_server(char *port)
{
  ccount=0;
  portno = atoi(port);

  /* 
   * socket: create the socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /*
   * main loop: wait for a datagram, then echo it
   */
  sending_len = sizeof(sending_addr);
  struct timeval timeout;      
  timeout.tv_sec = 1;
  timeout.tv_usec = 10;
}
void act_as_client(char *host,char *port)
{
  hostname = host;
  portno = atoi(port);

  /* socket: create the socket */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* gethostbyname: get the server's DNS entry */
  server = gethostbyname(hostname);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host as %s\n", hostname);
    exit(0);
  }

  /* build the server's Internet address */
  bzero((char *) &sending_addr, sizeof(sending_addr));
  sending_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
	(char *)&sending_addr.sin_addr.s_addr, server->h_length);
  sending_addr.sin_port = htons(portno);

  //making receiver blocking 
  //for RECEIVE timeout use SO_RCVTIMEO   
  struct timeval timeout;      
  timeout.tv_sec = 1;
  timeout.tv_usec = 10;

  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
		  sizeof(timeout)) < 0)
    error("setsockopt failed\n");
}
void initialize_network()
{
  initialize1(&sq);//initializing sender buffer
  initialize1(&rq);//initializing receiver buffer 
  if (pthread_mutex_init(&crlock, NULL) != 0)
    {
      printf("\n window mutex init has failed\n");
    }
  if (pthread_mutex_init(&slock, NULL) != 0)
    {
      printf("\n sender mutex init has failed\n");
    }
  if (pthread_mutex_init(&rlock, NULL) != 0)
    {
      printf("\n receiver  mutex init has failed\n");
    }
  initialize(&dq);
  initialize1(&sq);
  initialize1(&rq);
  berror=0;
  //////////////////////creating threads here //////////////////////
  pthread_attr_init(&attr);
  pthread_create(&receivert,&attr,receiver_thread,NULL);
  pthread_create(&congestion_controlt,&attr,congestion_control,NULL);
}
void exit_as_server()
{
  while(ccountr<fileSize || rqsize>0|| sqsize>0)
  {
    
  }
  sleep(1);
  printf("about to exit\n");
  ///////////////////canceling receiver thread here /////////////////
 
  pthread_cancel(receivert);
  pthread_cancel(congestion_controlt);
  pthread_mutex_destroy(&crlock);
  pthread_mutex_destroy(&slock);
  pthread_mutex_destroy(&rlock);
}
void exit_as_client()
{
    ///////////////////canceling receiver thread here /////////////////
  while(ccount<total_bytes || sqsize>0 || rqsize>0 )
  {

  }
  sleep(1);
  pthread_cancel(receivert);
  pthread_cancel(congestion_controlt);
  pthread_mutex_destroy(&crlock);
  pthread_mutex_destroy(&slock);
  pthread_mutex_destroy(&rlock);
}
void* congestion_control(void *param)
{
  while(ccount < total_bytes){
    printf("ccount:%d rqsize_receiver:%lu \r",ccount,rqsize_receiver);
    ll=(int)wsize-dsize;
    if(!berror && sqsize>0)//that is when there is no packet loss
      for(i=0;i<ll && scount<=total_bytes;++i)//if size of queue greater than wsize then no reading needed
	{
	  unsigned char buff[1024]={0};
	  unsigned long rqsize_remaining=(MAX1-rqsize)/1024;
	  bzero(buff,1024);
	  buff[0] = (scount >> 24);
	  buff[1] = (scount >> 16);
	  buff[2] = (scount >> 8) ;
	  buff[3] = scount;
	  buff[4] = 0;//0 for data packet and 1 for ack
	  buff[5] = rqsize_remaining >> 16;//only 24 bits needed as max rqsize < 2^20 kb
	  buff[6] = rqsize_remaining >> 8;
	  buff[7] = rqsize_remaining;
	  int tcount=8;
	  int nread =8;
	  pthread_mutex_lock(&slock);//--
	  while(tcount<1024  && sqsize>0)
	    {
	      buff[tcount]=dequeueF1(&sq);
	      ++tcount;
	      --sqsize;
	      ++nread;
	    }
	  pthread_mutex_unlock(&slock);//--
	  scount+=nread-8;
	  my_type temp;
	  my_cpy(temp.buff,buff);
	  temp.nread=nread;
	  pthread_mutex_lock(&crlock);//--
	  enqueueR(&dq,temp);
	  ++dsize;
	  pthread_mutex_unlock(&crlock);//--
	  printf("dsize: %d,wsize:%d",dsize,(int)wsize);
	  printf("Sending the file\n");
	  //printf("%s\n",temp.buff);
	  sendto(sockfd,buff,nread, 0, &sending_addr, sending_len);
	}
   if(alarm_fired || dupcount>=3)//packet loss encountered
      {
       
        pthread_mutex_lock(&crlock);//--
	if(alarm_fired)
	  {
	    printf("alarm fired \n");
	    alarm_fired=0;
	    if((int)wsize>6)
	      {
		ssthresh=wsize/2;
		wsize=1;//needs to be made to 1
	      }
	  }
	if(dupcount>=3)
	  {
	    dupcount=0;
	  printf("fast retransmitting now\n");
	  ssthresh=wsize/2;
	  wsize=ssthresh;
	  }
        dequeue tdq;
        initialize(&tdq);
        for(i=0;i<(int)wsize && i<dsize && i<rqsize_receiver;++i) //in the extreme case when window size is high but no further packet transfers needed
	  {
            
            my_type t=dequeueF(&dq);
            //printf("T.buff[3]:%u",t.buff[3]);
            printf("SSending the file\n");
            sendto(sockfd,t.buff,t.nread, 0, &sending_addr,sending_len);//handle nread
            enqueueF(&tdq,t);
          }
        for(i=0;i<(int)wsize && i<dsize && i<rqsize_receiver;++i)
	  {
            
            my_type t=dequeueF(&tdq);
            enqueueF(&dq,t);
	  }
        berror=1;
        pthread_mutex_unlock(&crlock);//--
      }
    if (ccount==total_bytes){//identifies last part of file 
      printf("end of file\n");
      break;
    }
  }
}
int app_send(int bytes,unsigned char *buffs)
{
  int i;
  while(1)//wait till sender does not have sufficient size
  {
    int lenter=0;
    pthread_mutex_lock(&slock);
    if(MAX1-sqsize-bytes>0)
    {
      
      for(i=0;i<bytes;++i)
      {
        enqueueR1(&sq,buffs[i]);
        ++sqsize;
      }
      
      lenter=1;

    }
    pthread_mutex_unlock(&slock);
    if(lenter)
      return 1;
  }
}
int app_recv(int bytes,unsigned char *buffs)
{
  int i;
  while(1)//wait till receiver has sufficient size
  {
    int lenter=0;
    pthread_mutex_lock(&rlock); 
    if(rqsize-bytes>=0)
      {
	for(i=0;i<bytes;++i)
	  {
       
	    buffs[i]=dequeueF1(&rq);
	    --rqsize;
	    
	  }
	lenter=1;
      }
    pthread_mutex_unlock(&rlock);
    if(lenter)
      return 1;
  }
}
void *receiver_thread(void *param)
{
   
    //int dupbreak=0;
    // dupcount=0;
  
  while(1)
    {
      printf("sqsize:%d\n",sqsize);
      printf("rqsize:%d\n",rqsize);
      printf("dsize:%d dupcount:%d\n",dsize,dupcount);
      alarm(SLEEP_VAL);//timer started here 
      (void) signal(SIGALRM, mysig);
      unsigned char rbuff[1024]={0};//recieved ackowledgement buff
      n = recvfrom( sockfd,rbuff,1024, 0, &sending_addr, &sending_len);
      if(n==-1)
	continue;
      tmpcount=rbuff[3] | (rbuff[2] << 8) | (rbuff[1] << 16) | (rbuff[0] << 24);
      printf("rqsize:%d\n",rqsize);
      if(rbuff[4]==0)//if data packet put in receive buffer
	{
	  pthread_mutex_lock(&crlock);//--
	  unsigned char recvBuff[1024]={0};
	  alarm(0);
	  printf("data packet received");
	  int i=8;
	  ////now sending acknowledgement of data packet ////////////
	  //bytesReceived =n;
	  //rcount=tmpcount;;
	  //ccount=ccountr;
	  printf("%lu recieved\n",tmpcount);
	  printf("ccountrr:%lu\n",ccountr);
	  
	  //printf("%lu",ccount);
	  double r= ((double) rand() / (RAND_MAX));
	  if(r>=drop_prob)//only for testing rempve it afterwards
	    {
	      unsigned long rqsize_remaining=(MAX1-rqsize)/1024;
	      if(tmpcount==ccountr)//only when received packet has not been received before we put in the receive buffer
		{
		  pthread_mutex_lock(&rlock); 
		  for(i=8;i<n;++i)
		    {
		      enqueueR1(&rq,rbuff[i]);
		      ++rqsize;
		     
		    }
		  pthread_mutex_unlock(&rlock);
		  printf("tmpcount:%lu",tmpcount);
		  printf("n:%d\n",n);
		  ccountr=tmpcount+n-8;
		  printf("ccountr:%lu\n",ccountr);
		  printf("This packet is new\n");
		  recvBuff[0] = (ccountr >> 24);
		  recvBuff[1] = (ccountr >> 16);
		  recvBuff[2] = (ccountr >> 8) ;
		  recvBuff[3] = ccountr;
		  recvBuff[4]= 1;//1 for ack
		  recvBuff[5] = (rqsize_remaining >> 16);//remaining window size will be written here
		  recvBuff[6] = (rqsize_remaining >> 8);
		  recvBuff[7] = (rqsize_remaining >> 0) ;
		  unsigned long tmp=recvBuff[7] | (recvBuff[6] << 8) | (recvBuff[5] << 16);
		  printf("tmp:%lu\n",tmp);
		   n=sendto(sockfd,recvBuff,8,0,(struct sockaddr *) &sending_addr,sending_len);
		  printf("acknowledge sent for:%lu\n",ccountr);
		}
	      else
		{
		  recvBuff[0] = (ccountr >> 24);
		  recvBuff[1] = (ccountr >> 16);
		  recvBuff[2] = (ccountr >> 8) ;
		  recvBuff[3] = ccountr;
		  recvBuff[4]= 1;//1 for ack
		  recvBuff[5] = (rqsize_remaining >> 16);//remaining window size will be written here
		  recvBuff[6] = (rqsize_remaining >> 8);
		  recvBuff[7] = (rqsize_remaining >> 0) ;
		  unsigned long tmp=recvBuff[7] | (recvBuff[6] << 8) | (recvBuff[5] << 16);
		  printf("btmp:%lu\n",tmp);
		  n=sendto(sockfd,recvBuff,8,0,(struct sockaddr *) &sending_addr,sending_len);
		  printf("acknowledge sent for:%lu\n",ccountr);
		}
	    }
	  else
	    {
	      printf("byte sequence number:%lu dropped\n",tmpcount);
	    }
	  pthread_mutex_unlock(&crlock);//--
	}
      else
	{
	  rqsize_receiver=rbuff[7] | (rbuff[6] << 8) | (rbuff[5] << 16);//updating receiver queue in the case of acknowledgement
          printf("alarm_fire value %d",alarm_fired);
          printf("received ackowledgement of:%lu from server\n",tmpcount);
          pthread_mutex_lock(&crlock);//--
	  
          if(tmpcount>rcount)//received acknowledgement is cumulative ack
            {//reset timer here
	       dupcount=0;
              printf("entered here \n");
              alarm(0);
              int cnt=1;//since these packets have already been acknowwledged removing them from queue
              for(i=0;i<cnt;++i)
                {
                  
		  my_type t=dequeueF(&dq);
                  --  dsize;
                }
                
              if(wsize <= ssthresh)//multiplicative increase
                wsize+=1;
              else
                wsize+=(double)1/wsize;//(double)cnt/ssthresh;//additive increase
              printf("dsize:%d,wsize:%d\n",dsize,(int)wsize);
              rcount=tmpcount;
              ccount=rcount;
             
              printf("rcount:%lu",rcount);
              berror=0;
               
              
            }
          else//increasing the dupacks
            {
              ++dupcount;
	      printf("//////////************dup count increased\n");
            }
	  pthread_mutex_unlock(&crlock);//--
	}
      
    }
}
void initialize(dequeue *P)
{
  P->rear=-1;
  P->front=-1;
}
 
int empty(dequeue *P)
{
  if(P->rear==-1)
    return(1);
    
  return(0);
}
 
int full(dequeue *P)
{
  if((P->rear+1)%MAX==P->front)
    return(1);
    
  return(0);
}
 
void enqueueR(dequeue *P,my_type x)
{
  if(empty(P))
    {
      P->rear=0;
      P->front=0;
      P->data[0]=x;
    }
  else
    {
      P->rear=(P->rear+1)%MAX;
      P->data[P->rear]=x;
    }
}
 
void enqueueF(dequeue *P,my_type x)
{
  if(empty(P))
    {
      P->rear=0;
      P->front=0;
      P->data[0]=x;
    }
  else
    {
      P->front=(P->front-1+MAX)%MAX;
      P->data[P->front]=x;
    }
}
 
my_type dequeueF(dequeue *P)
{
  my_type x;
    
  x=P->data[P->front];
    
  if(P->rear==P->front)    //delete the last element
    initialize(P);
  else
    P->front=(P->front+1)%MAX;
    
  return(x);
}
 
my_type dequeueR(dequeue *P)
{
  my_type x;
    
  x=P->data[P->rear];
    
  if(P->rear==P->front)
    initialize(P);
  else
    P->rear=(P->rear-1+MAX)%MAX;
        
  return(x);
}
 
void print(dequeue *P)
{
  if(empty(P))
    {
      printf("\nQueue is empty!!");
      exit(0);
    }
    
  int i;
  i=P->front;
    
  while(i!=P->rear)
    {
      printf("\n%u",(P->data[i]).buff[3]);
      i=(i+1)%MAX;
    }
    
  printf("\n%u\n",(P->data[P->rear]).buff[3]);
}

void initialize1(dequeue1 *P)
{
  P->rear=-1;
  P->front=-1;
}
 
int empty1(dequeue1 *P)
{
  if(P->rear==-1)
    return(1);
    
  return(0);
}
 
int full1(dequeue1 *P)
{
  if((P->rear+1)%MAX1==P->front)
    return(1);
    
  return(0);
}
 
void enqueueR1(dequeue1 *P,unsigned char x)
{
  if(empty1(P))
    {
      P->rear=0;
      P->front=0;
      P->data[0]=x;
    }
  else
    {
      P->rear=(P->rear+1)%MAX1;
      P->data[P->rear]=x;
    }
}
 
void enqueueF1(dequeue1 *P,unsigned char x)
{
  if(empty1(P))
    {
      P->rear=0;
      P->front=0;
      P->data[0]=x;
    }
  else
    {
      P->front=(P->front-1+MAX1)%MAX1;
      P->data[P->front]=x;
    }
}
 
unsigned char dequeueF1(dequeue1 *P)
{
  unsigned char x;
    
  x=P->data[P->front];
    
  if(P->rear==P->front)    //delete the last element
    initialize1(P);
  else
    P->front=(P->front+1)%MAX1;
    
  return(x);
}
 
unsigned char dequeueR1(dequeue1 *P)
{
  unsigned char x;
    
  x=P->data[P->rear];
    
  if(P->rear==P->front)
    initialize1(P);
  else
    P->rear=(P->rear-1+MAX1)%MAX1;
        
  return(x);
}
 
void print1(dequeue1 *P)
{
  if(empty1(P))
    {
      printf("\nQueue is empty!!");
      exit(0);
    }
    
  int i;
  i=P->front;
    
  while(i!=P->rear)
    {
      printf("\n%u",(P->data[i]));
      i=(i+1)%MAX1;
    }
    
  printf("\n%u\n",(P->data[P->rear]));
}
