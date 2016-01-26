#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
	
#define PORT 8003
#define BUFSIZE 1024

void send_to_all(int j, int i, int sockfd, int nbytes_recvd, char *recv_buf, fd_set *master){
	if (FD_ISSET(j, master)){
		if (j != sockfd && j != i) {
			write(j, recv_buf, nbytes_recvd);
		}
	}
}
		
void send_recv(int i, fd_set *master, int sockfd, int fdmax){

	int nbytes_recvd, j;
	char recv_buf[BUFSIZE], buf[BUFSIZE];
	
	nbytes_recvd = read(i, recv_buf, BUFSIZE);
	printf("Read: %s\n", recv_buf); // data doens't rly matter here, just as long as it'sbeing sent
	close(i);
	FD_CLR(i, master);
	for(j = 0; j <= fdmax; j++){
		send_to_all(j, i, sockfd, nbytes_recvd, recv_buf, master );
	}	
}
		
void connection_accept(fd_set *master, int *fdmax, int sockfd, struct sockaddr_in *client_addr){
	socklen_t addrlen;
	int newsockfd;
	
	addrlen = sizeof(struct sockaddr_in);
	newsockfd = accept(sockfd, (struct sockaddr *)client_addr, &addrlen);

	FD_SET(newsockfd, master);
	if(newsockfd > *fdmax){
		*fdmax = newsockfd;
	}
}
	
void connect_request(int *sockfd, struct sockaddr_in *my_addr){
		
	*sockfd = socket(AF_INET, SOCK_STREAM, 0);
		
	my_addr->sin_family = AF_INET;
	my_addr->sin_port = htons(PORT);
	my_addr->sin_addr.s_addr = INADDR_ANY;
		
	bind(*sockfd, (struct sockaddr *)my_addr, sizeof(struct sockaddr));
	listen(*sockfd, 10) == -1);
}

int main(){
	fd_set master;
	fd_set read_fds;
	int fdmax, i;
	int sockfd= 0;
	struct sockaddr_in my_addr, client_addr;
	
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	connect_request(&sockfd, &my_addr);
	FD_SET(sockfd, &master);
	
	fdmax = sockfd;
	while(1){
		read_fds = master;
		select(fdmax+1, &read_fds, NULL, NULL, NULL);
		
		for (i = 0; i <= fdmax; i++){
			if (FD_ISSET(i, &read_fds)){
				if (i == sockfd)
					connection_accept(&master, &fdmax, sockfd, &client_addr);
				else
					send_recv(i, &master, sockfd, fdmax);
			}
		}
	}
	return 0;
}

