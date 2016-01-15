#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(){
  int socket_id, socket_client;

  //create socket
  socket_id = socket(AF_INET, SOCK_STREAM, 0);

  //bind to port
  struct sockaddr_in listener;
  listener.sin_family = AF_INET;     //socket type IPv4
  listener.sin_port = htons(MY_PORT);
  listener.sin_addr.s_addr = INADDR_ANY;   //bind to any incoming address
  bind(socket_id, (struct sockaddr*)&listener, sizeof(listener));

  listen(socket_id, 1);
  printf("<server> Listening\n");

  socket_client = accept(socket_id, NULL, NULL);
  printf("<server> Connected: %d\n", socket_client);

  write(socket_client, "Hello", 6);
  
  return 0;
}
