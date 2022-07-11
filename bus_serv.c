//2018112752 임재윤
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define ROWS 2
#define COLS 10
#define MAX_CLNT 4

typedef struct{
	int command;
	int seatno;
	int seats[ROWS][COLS];
	int result;
} BUS_INFO;

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx, mutex;
BUS_INFO bus;

void* handle_clnt(void *arg);
void send_bus();
void error_handling(char *msg);

int main(int argc, char *argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t tid;
	if(argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	pthread_mutex_init(&mutx, NULL);
	pthread_mutex_init(&mutex, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while(1){
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);

		pthread_create(&tid, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(tid);
		printf("Connected client IP: %s, clnt_sock=%d\n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);
	}
	close(serv_sock);
	printf("its finished\n");
	return 0;
}

void* handle_clnt(void* arg){
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	BUS_INFO temp;
	
	while(1){
		if(!read(clnt_sock, &temp, sizeof(temp)))
			error_handling("read() error");
		switch(temp.command){
			case 1:
				bus.result = 0;
				break;
			case 2:
				if(temp.seatno >= 0 && temp.seatno < 20){
					int row = temp.seatno / 10;
					int col = temp.seatno % 10;
					if(bus.seats[row][col] == 0){
						bus.seats[row][col] = clnt_sock;
						bus.result = 0;
					}
					else
						bus.result = -2;
				}
				else
					bus.result = -1;
				break;
			case 3:
				if(temp.seatno >= 0 && temp.seatno < 20){
                                        int row = temp.seatno / 10;
                                        int col = temp.seatno % 10;
                                        if(bus.seats[row][col] == clnt_sock){
                                                bus.seats[row][col] = 0;
						bus.result = 0;
					}
					else if(bus.seats[row][col] == 0)
						bus.result = -3;
                                        else
                                                bus.result = -4;
                                }
                                else
                                        bus.result = -1;
				break;
			case 4:		//command is exit, so return temp, not bus, because bus is global variable. we can't edit bus.command cuz it will end all other clients.
				write(clnt_sock, &temp, sizeof(temp));
				break;
		}
		if(temp.command == 4)
			break;
		write(clnt_sock, &bus, sizeof(bus));
	}
	pthread_mutex_lock(&mutx);
	for(i = 0; i < clnt_cnt; i++){
		if(clnt_sock == clnt_socks[i]){
			printf("clnt_sock=%d closed\n", clnt_socks[i]);
			while(i < clnt_cnt - 1){
				clnt_socks[i] = clnt_socks[i + 1];
				i++;
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

void error_handling(char *msg){
	printf("%s\n", msg);
	exit(1);
}
