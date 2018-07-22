all:	userver uclient 
userver:nserver.c net_lib.c net_lib.h;
	gcc nserver.c -o userver -lssl -lcrypto -w -lpthread;
uclient:udpclient.c net_lib.c net_lib.h ;
	gcc udpclient.c -o uclient -lssl -lcrypto -w -lpthread;
clean:	;
	rm uclient userver;
