//2018112752 임재윤
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>

#define ROWS 2
#define COLS 10

typedef struct {
        int command;
        int seatno;
        int seats[ROWS][COLS];
        int result;
} BUS_INFO;

void* bus_reservation();
void* send_bus(void* arg);
void* recv_bus(void* arg);
void print_bus_seat();
void error_handling(char* msg);

BUS_INFO bus;
sem_t snd, rcv, prt;

int main(int argc, char *argv[]){
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t prt_thread, snd_thread, rcv_thread;
	//initiate semaphores in a way to start print semaphore first
	sem_init(&prt, 0, 1);
	sem_init(&snd, 0, 0);
	sem_init(&rcv, 0, 0);

	if(argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

	//initiate thread	
	pthread_create(&prt_thread, NULL, bus_reservation, NULL);	
	pthread_create(&snd_thread, NULL, send_bus, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_bus, (void*)&sock);
	//wait untill all threads are down
	pthread_join(snd_thread, NULL);
	pthread_join(rcv_thread, NULL);
	pthread_join(prt_thread, NULL);
	//clean up semaphores
	sem_destroy(&snd);
	sem_destroy(&rcv);
	sem_destroy(&prt);
	//close socket
	close(sock);
	return 0;
}

void* send_bus(void *arg){
	int sock = *((int*)arg);
	while(1){
		sem_wait(&snd);
		write(sock, &bus, sizeof(bus));
		sem_post(&rcv);		//run receive thread
		if(bus.command == 4)
			break;
	}
	return NULL;
}

void* recv_bus(void *arg){
	int sock = *((int*)arg);
	while(1){
		sem_wait(&rcv);
		int str_len = read(sock, &bus, sizeof(bus));
		if(str_len == -1)
			return (void*)-1;
		if(bus.command == 4){		//if it is exit, finish
			//sem_post(&prt);
			break;
		}
		print_bus_seat();
		sem_post(&prt);			//run print thread
	}
	return NULL;
}

void* bus_reservation() {
	while(1){
		sem_wait(&prt);
		printf("1: inquiry, 2: reservation, 3: cancellation, 4: quit: ");
		scanf("%d", &bus.command);
		switch(bus.command){
			case 1:		//inquiry
				sem_post(&snd);
				break;
			case 2:		//reservation
				printf("Input seat number: ");
				scanf("%d", &bus.seatno);
				bus.seatno--;
				sem_post(&snd);
				break;
			case 3:		//cancellation
				printf("Input seat number for cancellation: ");
				scanf("%d", &bus.seatno);
				bus.seatno--;
				sem_post(&snd);
				break;
			case 4:		//exit
				sem_post(&snd);
				printf("Quit.\n");
				break;
		}
		if(bus.command == 4)
			break;
	}
	return NULL;
}



void print_bus_seat(){
	for(int i = 0; i < ROWS; i++){
		printf("----------------------------------------------------------------------------\n");
		for(int j = 0; j < COLS; j++)
			printf("%d\t", j + 1 + i * 10);
		printf("\n");
		for(int j = 0; j < COLS; j++)
			printf("%d\t", bus.seats[i][j]);
		printf("\n");
	}
	printf("----------------------------------------------------------------------------\n");
	switch(bus.result){		//print result
		case 0:
			printf("Operation success.\n");
			break;
		case -1:
			printf("Wrong seat number.\n");
			break;
		case -2:
			printf("Reservation failed. (The seat was already reserved.)\n");
			break;
		case -3:
			printf("Cancellation failed. (The seat was not reserved.)\n");
			break;
		case -4:
			printf("Cancellation failed. (The seat was reserved by another person.)\n");
			break;
	}
}

void error_handling(char *msg){
	printf("%s\n", msg);
	exit(1);
}
