#include "net_lib.c"
int main(int argc, char **argv) {
  /* check command line arguments */
  if (argc != 3) {
    fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
    exit(0);
  }
  act_as_client(argv[1],argv[2]);
  file_input_hello();
  initialize_network();
  unsigned long my_temp=0;
  unsigned char buff[4096]={0};
  bzero(buff,4096);
  while(my_temp<total_bytes)
  {
    int nread =fread(buff,1,4096,fd);
    //printf("buff sent%s\n",buff);
    app_send(nread,buff);
    my_temp+=nread;
  }
  exit_as_client();
  fclose(fd);
  
  drop_prob=0.05;
  printf("we are here\n");
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));
  struct timeval timeout; 
  timeout.tv_sec =0;
  timeout.tv_usec =0;
  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
		  sizeof(timeout)) < 0)
	{
		printf("error in set sock");
	}
  send_hello_ack_fopen();
  initialize_network();
  my_temp=0;
  bzero(buff,4096);//receiving will be here
  while(my_temp<fileSize)
  {
    if(my_temp+4096<fileSize)
      {
	app_recv(4096,buff);
	fwrite(buff, 1,4096,fd);
	//printf("bufffff write:%s\n",buff);
	my_temp+=4096;
      }
    else
      {
	app_recv(fileSize-my_temp,buff);
	fwrite(buff, 1,fileSize-my_temp,fd);
	//printf("bufffff write:%s\n",buff);
	my_temp=fileSize;
      }
  }
  exit_as_server();
  fclose(fd);
  return 0;
}
