#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"

int main(int argc, char **argv){
  int socket_id;
  char buffer[256];
  int i;

  socket_id = socket( AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in sock;
  sock.sin_family = AF_INET;
  sock.sin_port = htons(MY_PORT);
  inet_aton("127.0.0.1", &(sock.sin_addr));
  bind(socket_id, (struct sockaddr*)&sock, sizeof(sock));

  i = connect(socket_id, (struct sockaddr*)&sock, sizeof(sock));
  printf("<client> Connect returned: %d\n", i);
  while(1){
    printf("Send to Server: ");
    fgets(buffer,sizeof(buffer),stdin);
    *strchr( buffer, '\n' ) = 0;
    
    write(socket_id, buffer, 255);
    read(socket_id, buffer, 255);
    printf("<client> Received: %s\n", buffer);
  }
  
	return 0;
}
