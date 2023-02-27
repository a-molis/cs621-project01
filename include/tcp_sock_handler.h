#include <stddef.h>

#ifndef TCP_SOCK_HANDLER_H_
#define TCP_SOCK_HANDLER_H_

struct TCP_SOCKET_HANDLER 
{
  int sockfd;
};

typedef struct TCP_SOCKET_HANDLER *TCP_HANDLER;

struct TCP_SERVER_HANDLER
{
  int port;
  TCP_HANDLER handler;
};

struct TCP_CLIENT_HANDLER
{
    char *host_ip;
    int server_port;
    TCP_HANDLER handler;
};

typedef struct TCP_SERVER_HANDLER *TCP_SERVER;
typedef struct TCP_CLIENT_HANDLER *TCP_CLIENT_CONN;

TCP_HANDLER new_tcp_handler(int socket_fd);
int tcp_sendn(TCP_HANDLER handler, char *buf, int buf_len);
int tcp_recvn(TCP_HANDLER handler, char *buf, int buf_len);
int tcp_send(TCP_HANDLER handler, char *buf, int buf_len);
int tcp_recv(TCP_HANDLER handler, char *buf, int *output_len);
int destroy_tcp_handler(TCP_HANDLER handler);

// Server functions
TCP_SERVER tcp_new_server(int port);
int tcp_server_start(TCP_SERVER server);
TCP_HANDLER tcp_server_next_connection(TCP_SERVER server);
int destroy_tcp_sever(TCP_SERVER server);

// Client functions
TCP_CLIENT_CONN tcp_new_client(char *host_ip, unsigned short port);
int tcp_client_connect(TCP_CLIENT_CONN client);
int destroy_tcp_client(TCP_CLIENT_CONN client);

#endif
