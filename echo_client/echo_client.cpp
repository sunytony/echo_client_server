#include <stdio.h> // for perror
#include <string.h> // for memset
#include <stdlib.h>
#include <unistd.h> // for close
#include <arpa/inet.h> // for htons
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket
#include <pthread.h>



void * client_listen(void * sockfd){
	const static int BUFSIZE = 1024;
	char buf[BUFSIZE];
	printf("sockfd = %d", *((int*)sockfd));
	while(true){
		ssize_t received = recv(*((int*)sockfd), buf, BUFSIZE - 1, 0);
		if (received == 0 || received == -1) {
			perror("recv failed");
			break;
		}
		buf[received] = '\0';
		printf("%s\n", buf);
	}
}

int main(int argc, char *argv[]) {
	pthread_t p_thread;
	unsigned char ip[4];

	if(argc != 3){
		perror("argument error");
		return -1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		perror("socket failed");
		return -1;
	}

	sscanf(argv[1], "%u.%u.%u.%u", ip, ip+1, ip+2, ip+3);
	
	printf("%x ",*((int *)ip));
	printf("hello");
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	addr.sin_addr.s_addr = *((int *)ip);
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr));
	if (res == -1) {
		perror("connect failed");
		return -1;
	}
	printf("connected\n");

	int thr_id = pthread_create(&p_thread, NULL, client_listen, (void *)&sockfd);

	if(thr_id < 0){
		perror("thread create error");
		return -1;
	}
	while (true) {
		const static int BUFSIZE = 1024;
		char buf[BUFSIZE];
		scanf("%s", buf);
		
		ssize_t sent = send(sockfd, buf, strlen(buf), 0);
		if (sent == 0) {
			perror("send failed");
			break;
		}
		if (strcmp(buf, "quit") == 0) break;
	}
	pthread_detach(p_thread);
	pthread_cancel(p_thread);

	close(sockfd);
}
