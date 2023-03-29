/**
 * Header file for UDP socket operations and connections.
 */
#include <sys/socket.h>

#ifndef _UDP_SOCK_HANDLER_H_
#define _UDP_SOCK_HANDLER_H_

// Wrapper struct to wrap up a UDP socket, and sockaddr_in.
struct UDP_SOCKET_HANDLER
{
    int sockfd;
    struct sockaddr_in *addr;
    socklen_t addr_len;
};

// Names UDP_SOCKET_HANDLER pointer as UDP_HANDLER for simplicity.
typedef struct UDP_SOCKET_HANDLER *UDP_HANDLER;

// Wrapper struct for server socket/connection information.
struct UDP_SERVER_HANDLER
{
    int port;
    int sockfd;
    struct sockaddr_in *addr;
    socklen_t addr_len;
};

// Wrapper struct for client socket/connection with server.
struct UDP_CLIENT_HANDLER
{
    char *ip_address;
    int port;
    UDP_HANDLER handler;
};

// Renames the struct UDP_SERVER_HANDLER pointer as UDP_SERVER.
typedef struct UDP_SERVER_HANDLER *UDP_SERVER;

// Renames struct UDP_CLIENT_HANDLER pointer as UDP_CLIENT_CONN.
typedef struct UDP_CLIENT_HANDLER *UDP_CLIENT_CONN;

/**
 * Creates a new UDP_HANDLER based on a socket file descriptor.
 * This is created on the heap so it needs to be destroyed with udp_destroy_handler.
 *
 * @param socket_fd The socket file descriptor to use in the wrapper.
 * @return A UDP_HANDLER if no errors, NULL otherwise.
 */
UDP_HANDLER udp_new_handler(int socket_fd);

/**
 * Sends data over a UDP connection using a UDP_HANDLER.
 * @param handler The UDP_HANDLER.
 * @param buf The buffer to send.
 * @param buf_len The length of the buffer.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int udp_sendto_n(UDP_HANDLER handler, char *buf, int buf_len);

/**
 * Receives UDP data over a UDP connection using a UDP_HANDLER.
 * @param handler The UDP_HANDLER.
 * @param buf The buffer to store the received data.
 * @param buf_len The length of the data to receive.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int udp_recvfrom_n(UDP_HANDLER handler, char *buf, int buf_len);

/**
 * Destroys a UDP_HANDLER by freeing its memory.
 * @param handler The UDP_HANDLER to free
 * @return Returns 0 if there is no error 1 otherwise.
 */
int udp_destroy_handler(UDP_HANDLER handler);

/**
 * Creates a new UDP_SERVER based on a port.
 * This just sets up the handlers memory.
 * The UDP_SERVER is allocated on the heap so it needs to be destroyed with udp_server_destroy.
 *
 * @param port The port to use for the UDP_SERVER connection.
 * @return Returns 0 if there is no error 1 otherwise.
 */
UDP_SERVER udp_new_server(int port);

/**
 * Starts a UDP_SERVER connection by creating a socket, bind and allocates memory for the struct sockaddr_in.
 * @param server The UDP_SERVER to start.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int udp_server_start(UDP_SERVER server);

/**
 * Creates a new UDP_HANDLER to use with the UDP_SERVER.
 * This allocates memory on the heap so this needs to be destroyed with udp_destroy_handler.
 * First a UDP_SERVER is created then setup then the server.
 * Then a UDP_HANDLER is created to handle all of the connections.
 *
 * @param server The UDP_SERVER to get a new UDP_HANDLER.
 * @return A new UDP_HANDLER if there are no errors.
 */
UDP_HANDLER udp_server_next_connection(UDP_SERVER server);

/**
 * Frees all of the memory for a UDP_SERVER and it's variables.
 * @param server The UDP_SERVER to destroy.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int udp_server_destroy (UDP_SERVER server);

/**
 * Creates a new UDP_CLIENT_CONN.
 * A UDP_CLIENT_CONN is a wrapper for a UDP_HANDLER to use for a client to communicate with a server over UDP.
 * First a new UDP_CLIENT is created with this function.
 * Then udp_client_connect is called to create the socket and connection data.
 * This allocates memory on the heap so it needs to be destroyed with udp_destroy_client to free the memory.
 *
 * @param ip_address The IPv4 address of the server to use.
 * @param port The port of the server to connect to.
 * @return Returns a UDP_CLIENT_CONN if no errors, NULL otherwise.
 */
UDP_CLIENT_CONN udp_new_client(char *ip_address, unsigned short port);

/**
 * Creates the connection socket and other data for the UDP_CLIENT_CONN to work with a server.
 * A UDP_CLIENT_CONN should be first created with the function udp_new_client.
 * @param client The UDP_CLIENT_CONN to start.
 * @return Returns 0 if there are no error 1 otherwise.
 */
int udp_client_connect(UDP_CLIENT_CONN client);

/**
 * Frees the memory for a UDP_CLIENT_CONN.
 * @param client The UDP_CLIENT_CONN to free.
 * @return Returns 0 if there are no error 1 otherwise.
 */
int udp_destroy_client(UDP_CLIENT_CONN client);

#endif //_UDP_SOCK_HANDLER_H_
