#include <stdio.h> // for perror
#include <stdint.h>
#include <string.h> // for memset
#include <stdlib.h> // for atoi
#include <unistd.h> // for close
#include <arpa/inet.h> // for htons
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket
#include <pthread.h>
#include <iostream>
#include <list>

using namespace std;

typedef struct {
	int childid;
	int all_echo;
	list<int>* thread_list;
} MultipleArg;

pthread_mutex_t mutex;

void dump(uint8_t* str, int num){
	for(int i = 0; i < num ; ++i){
		printf("%x ", str[i]);
	}
}

list<int> user_list;

void *server_listen(void *arg){
	pthread_mutex_lock(&mutex);
	(*((MultipleArg *)arg)->thread_list).push_front(((MultipleArg *)arg)->childid);
	int i = ((MultipleArg *)arg)->all_echo;
	list<int>::iterator itor = (*((MultipleArg *)arg)->thread_list).begin();
	list<int>::iterator itor2 = (*((MultipleArg *)arg)->thread_list).begin();
	cout << (*((MultipleArg *)arg)->thread_list).size() << "size" << endl;
	pthread_mutex_unlock(&mutex);

	if(i == 0){
		cout << "not broadcast" << endl;
		while (true) {
			const static int BUFSIZE = 1024;
			char buf[BUFSIZE];

			ssize_t received = recv(((MultipleArg *)arg)->childid, buf, BUFSIZE - 1, 0);
			if (received == 0 || received == -1) {
				perror("recv failed");
				break;
			}
			buf[received] = '\0';
			printf("%s\n", buf);

			if(strcmp(buf, "quit") == 0){
				printf("disconnected\n");
				break;
			}

			ssize_t sent = send(((MultipleArg *)arg)->childid, buf, strlen(buf), 0);
			if (sent == 0) {
				perror("send failed");
				break;
			}
		}
	} else if (i == 1){
		cout << "broadcast" << endl;
		while (true) {
			const static int BUFSIZE = 1024;
			char buf[BUFSIZE];
			ssize_t received = recv(((MultipleArg *)arg)->childid, buf, BUFSIZE - 1, 0);
			if (received == 0 || received == -1) {
				perror("recv failed");
				break;
			}
			buf[received] = '\0';
			printf("%s\n", buf);

			if(strcmp(buf, "quit") == 0){
				printf("disconnected\n");
				break;
			}
			pthread_mutex_lock(&mutex);
			for(itor2 = (*((MultipleArg *)arg)->thread_list).begin(); itor2 != (*((MultipleArg *)arg)->thread_list).end(); ++itor2){
				ssize_t sent = send(*itor2, buf, strlen(buf), 0);
				if (sent == 0) {
					perror("send failed");
					break;
				}
			}
			pthread_mutex_unlock(&mutex);
		}
	}
	pthread_mutex_lock(&mutex);
	(*((MultipleArg *)arg)->thread_list).erase(itor);
	printf("arg : %x", arg);
	free(arg);
	pthread_mutex_unlock(&mutex);
}

int main(int argc, char* argv[]) {

	if(argc > 3 || argc == 1){
		printf("wrong argumnets");
		return -1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket failed");
		return -1;
	}

	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,  &optval , sizeof(int));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr));
	if (res == -1) {
		perror("bind failed");
		return -1;
	}

	res = listen(sockfd, 2);
	if (res == -1) {
		perror("listen failed");
		return -1;
	}

	pthread_mutex_init(&mutex, NULL);

	while (true) {
		pthread_t p_thread;
		struct sockaddr_in addr;
		int thr_id = 0;
		socklen_t clientlen = sizeof(sockaddr);

		int childfd = accept(sockfd, reinterpret_cast<struct sockaddr*>(&addr), &clientlen);
		if (childfd < 0) {
			perror("ERROR on accept");
			break;
		}
		printf("connected\n");

		MultipleArg* argument = (MultipleArg *)malloc(sizeof(MultipleArg));
		
		argument->childid = childfd;
		argument->all_echo = (argc == 3 && strcmp(argv[2], "-b") == 0) ? 1 : 0;
		argument->thread_list = &user_list;

		printf("list: %x   thread : %x", user_list, argument->thread_list);

		thr_id = pthread_create(&p_thread, NULL, server_listen, (void *)argument);

		if(thr_id < 0){
			perror("thread create error");
			exit(0);
		}
	}

	close(sockfd);

	return 1;
}
