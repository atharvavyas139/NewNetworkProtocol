#include "net_lib.c"
int main(int argc, char **argv) {
 /* 
   * check command line arguments 
   */
  drop_prob=0.005;
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port_for_server>\n", argv[0]);
    exit(1);
  }
  act_as_server(argv[1]);
  /*
      read the hello msg
  */
  printf("we are here\n");
  send_hello_ack_fopen();
  initialize_network();
  int my_temp=0;
  unsigned char buff[4096]={0};
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

  drop_prob=0.00;
  struct timeval timeout;      
  timeout.tv_sec = 1;
  timeout.tv_usec = 10;
  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
		  sizeof(timeout)) < 0)
    error("setsockopt failed\n");
  file_input_hello();
  initialize_network();
  my_temp=0;
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
  return 0;
}
