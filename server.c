#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"

int main(){

  char buffer[256];
  int socket_id, socket_client;
  int my_port = MY_PORT;

  socket_id = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in listener;
  listener.sin_family = AF_INET;     //socket type IPv4
  listener.sin_port = htons(my_port);
  listener.sin_addr.s_addr = INADDR_ANY;   //bind to any incoming address

  bind(socket_id, (struct sockaddr*)&listener, sizeof(listener));

  listen(socket_id, 1);
  printf("<server> Listening\n");
  
  /*
  int socket_id, socket_client;

  //create socket
  socket_id = socket(AF_INET, SOCK_STREAM, 0);

  //bind to port
  struct sockaddr_in listener;
  listener.sin_family = AF_INET;     //socket type IPv4
  listener.sin_port = htons(MY_PORT);
  listener.sin_addr.s_addr = INADDR_ANY;   //bind to any incoming address
  bind(socket_id, (struct sockaddr*)&listener, sizeof(listener));
  */

  while(1){
    socket_client = accept(socket_id, NULL, NULL);
    printf("<server> Connected: %d\n", socket_client);

    int chpid = fork();
        
    if( chpid == 0 ) {
      while( read(socket_client, buffer, 255) ){
        close(socket_id);
        write(socket_client, "Spaghetti Received", 255);
        printf("<server> Recieved %s\n",buffer);
      }
      break;
    }else{
      close(socket_client);
      continue;
    }
  }
  return 0;
}
