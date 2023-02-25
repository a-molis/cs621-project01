#include <sys/socket.h>

#ifndef _UDP_SOCK_HANDLER_H_
#define _UDP_SOCK_HANDLER_H_

struct UDP_SOCKET_HANDLER
{
    int sockfd;
    struct sockaddr_in *addr;
    socklen_t addr_len;
};

typedef struct UDP_SOCKET_HANDLER *UDP_HANDLER;

struct UDP_SERVER_HANDLER
{
    int port;
    UDP_HANDLER handler;
};

struct UDP_CLIENT_HANDLER
{
    char *host;
    int port;
    UDP_HANDLER handler;
};

typedef struct UDP_SERVER_HANDLER *UDP_SERVER;
typedef struct UDP_CLIENT_HANDLER *UDP_CLIENT_CONN;

UDP_HANDLER udp_new_handler(int socket_fd);
int udp_sendto_n(UDP_HANDLER handler, char *buf, int buf_len);
int udp_recvfrom_n(UDP_HANDLER handler, char *buf, int buf_len);
int udp_sendto(UDP_HANDLER handler, char *buf, int buf_len);
int udp_recvfrom(UDP_HANDLER handler, char **buf);
int udp_destroy_handler(UDP_HANDLER handler);

// Server functions
UDP_SERVER udp_new_server(int port);
int udp_server_start(UDP_SERVER server);
UDP_HANDLER udp_server_next_connection(UDP_SERVER server);
int udp_server_destroy (UDP_SERVER server);

// Client functions
UDP_CLIENT_CONN udp_new_client(char *host, unsigned short port);
int udp_client_connect(UDP_CLIENT_CONN client);
int udp_destroy_client(UDP_CLIENT_CONN client);


typedef struct UDP_SERVER_HANDLER *UDP_SERVER;
typedef struct UDP_CLIENT_HANDLER *UDP_CLIENT_CONN;

#endif //_UDP_SOCK_HANDLER_H_
