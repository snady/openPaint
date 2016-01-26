#ifndef SERVER_H
#define SERVER_H

#define MY_PORT 8003
#define PIPE_RDWR

#define PIPE_RD 0
#define PIPE_WR 1

void send_data( int pipefd[10][2], char* buffer );

void add_pipe( int pipefd[10][2] );

#endif
