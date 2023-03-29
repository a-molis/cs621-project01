/**
 * Header file for tcp_sock_handler functions.
 * These functions are set up as a framework for setting up TCP connections and sending/receiving data with TCP.
 */
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

/**
 * Creates a new TCP_HANDLER.
 * A TCP_HANDLER is a wrapper for a sock file descriptor.
 * The functions in this header abstract the socket file descriptor TCP functions using a TCP_HEADER.
 * @param socket_fd The socket file descriptor
 * @return The output TCP_HANDLER or NULL if there was an error.
 */
TCP_HANDLER new_tcp_handler(int socket_fd);

/**
 * Sends data over a TCP socket using a TCP_HANDLER.
 * This function takes in a buf_len for the amount of data to send
 * @param handler The TCP_HANDLER to use for sending data over TCP.
 * @param buf The buffer to send.
 * @param buf_len The length of the buffer.
 * @return Returns 0 if there is no error, any other value indicates an error.
 */
int tcp_sendn(TCP_HANDLER handler, char *buf, int buf_len);

/**
 * Receives data using a TCP_HANDLER.
 * The function tcp_recvn receives data with length buf_len.
 * @param handler The TCP_HANDLER struct.
 * @param buf The buffer of to store the data.
 * @param buf_len The length of the data to receive.
 * @return Returns the total amount of data received.
 */
int tcp_recvn(TCP_HANDLER handler, char *buf, int buf_len);

/**
 * This function sends data with a TCP_HANDLER.
 * This functions allows sending of data of buf_len so
 * data can be received dynamically without tcp_recv knowing the size of the data to receive upfront.
 *
 * This should be used with tcp_recv on the other end.
 * Two TCP packets are sent. The first packet always contains 4 bytes of data.
 * Then the data are sent in a second TCP packet.
 * tcp_recv does the reverse of this and
 * @param handler The TCP_HANDLER
 * @param buf The buffer that has data to send.
 * @param buf_len The length of the buffer.
 * @return Returns 0 if there is no error, any other value indicates an error.
 */
int tcp_send(TCP_HANDLER handler, char *buf, int buf_len);

/**
 * Reads data off of a socket with a TCP_HEADER.
 * This function is to be used in conjunction with tcp_send.
 * This allows data to be read off of the socket without knowing how much data to read.
 * However, the buffer needs to be large enough to store the data. So this will work if the buffer is large.
 *
 * First 4 bytes are read off of the socket to get the size of the data.
 * Then another packet is read using the size in the first recv to get the data in the buffer.
 *
 * The size of the buffer is then stored in output_len.
 *
 * @param handler The TCP_HANDLER to read data.
 * @param buf The buffer to store the data.
 * @param output_len The length of data stored in the buffer.
 * @return Returns 0 if there is no error, any other value indicates an error.
 */
int tcp_recv(TCP_HANDLER handler, char *buf, int *output_len);

/**
 * Destroys a TCP_HANDLER and frees the memory.
 * @param handler The TCP_HANDLER to to free/destroy.
 * @return
 */
int destroy_tcp_handler(TCP_HANDLER handler);

/**
 * Creates a TCP_SERVER struct. A TCP_SERVER is a wrapper for a TCP_HANDLER for a server.
 * This function just sets up the TCP_SERVER and tcp_server_start sets up the server.
 *
 * @param port The port used by the server for the connection.
 * @return The TCP_SERVER struct pointer.
 */
TCP_SERVER tcp_new_server(int port);

/**
 * Starts up a TCP_SERVER by creating a socket, setting socket options, binding and then listening.
 * @param server The TCP_SERVER to start.
 * @return Returns 0 if there is no error, any other value indicates an error.
 */
int tcp_server_start(TCP_SERVER server);

/**
 * Sets up an accept on the TCP_SERVER and returns a TCP_HANDLER for the new connection.
 * @param server The TCP_SERVER to use to get the next connection.
 * @return The TCP_HANDLER if there are no errors, NULL if there are errors.
 */
TCP_HANDLER tcp_server_next_connection(TCP_SERVER server);

/**
 * Destroys a TCP_SERVER by freeing all of the memory allocated for the server
 * @param server The TCP_SERVER to destroy
 * @return Returns 0 if there is no error, 1 if there is an error.
 */
int destroy_tcp_sever(TCP_SERVER server);

/**
 * Sets up a new TCP_CLIENT_CONN. A TCP_CLIENT_CONN is a pointer to a TCP_CLIENT_HANDLER struct.
 * This struct wraps up a TCP_HANDLER and is used for a client to connect to a TCP_SERVER.
 * This function just sets up the TCP_CLIENT_CONN and tcp_client_connect initializes the connection with the server.
 *
 * @param host_ip The IPv4 address of the host server.
 * @param port The port the server is bound to.
 * @return This returns a TCP_CLIENT_CONN, if an error occurs NULL is returned.
 */
TCP_CLIENT_CONN tcp_new_client(char *host_ip, unsigned short port);

/**
 * Connects with a TCP_CLIENT_CONN to a TCP_SERVER that is ready to accept a new connection.
 * @param client The TCP_CLIENT_CONN to connect.
 * @return Returns 0 if there is no error, 1 if there is an error.
 */
int tcp_client_connect(TCP_CLIENT_CONN client);

/**
 * Destroys a TCP_CLIENT_CONN by freeing the memory allocated for the TCP_CLIENT_CONN.
 * @param client The TCP_CLIENT_CONN to destory.
 * @return Returns 0 if there is no error, 1 if there is an error.
 */
int destroy_tcp_client(TCP_CLIENT_CONN client);

#endif
