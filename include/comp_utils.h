/**
 * Header file for functions for part 1 and 2 business logic
 */
#include <sys/timeb.h>
#include <signal.h>
#include "config.h"
#include "constants.h"
#include "udp_sock_handler.h"

#ifndef _COMPDETECT_H_
#define _COMPDETECT_H_

/**
 * Runs the client pre probe stage for part 1.
 * @param config The config struct.
 * @param config_str The string representation of the config to be sent to the server.
 * @return Returns 0 if there is no error, 1 otherwise.
 */
int client_pre_probe (CONFIG config, char *config_str);

/**
 * Runs the server pre probe stage for part 1.
 * @param port The port to use for the server.
 * @return The config struct pointer as a CONFIG type or NULL if there was an error.
 */
CONFIG server_pre_probe (int port);

/**
 * Runs the client probe stage for part 1.
 * @param config The config struct.
 * @return Returns 0 if there is no error, 1 otherwise.
 */
int client_probe (CONFIG config);

/**
 * Runs the server probe stage.
 * @param config The config struct.
 * @param result A string buffer to store the result information within.
 *               This data is then sent to client in post probe stage.
 * @return Returns 0 if there is no error, 1 otherwise.
 */
int server_probe (CONFIG config, char *result);

/**
 * Receives a UDP packet train. The result info for stats is stored in the result buffer.
 * The time it took to receive the first and last UDP packet is stored in mss.
 * The train_type t is either high or low for the type of data received.
 *
 * @param client_handler The client UDP_HANDLER used for receiving data.
 * @param config The config struct.
 * @param t The train_type, low for low entropy data, high for high entropy data.
 * @param result The buffer for the result stats.
 * @param mss The time it took from the first packet received to the last packet received.
 * @return Returns 0 if there is no error, 1 otherwise.
 */
int recv_udp_train (UDP_HANDLER client_handler, CONFIG config, enum train_type t, char result[], double *mss);

/**
 * Runs server post probe. The result from the recv_udp_train call is sent in this function to the client.
 * @param config The config struct.
 * @param result The buffer for the result stats and compression detection info.
 * @return Returns 0 if there is no error, 1 otherwise.
 */
int server_post_probe (CONFIG config, char *result);

/**
 * Runs the client post probe stage to get the results back from the server.
 * @param config The config struct.
 * @return Returns 0 if there is no error, 1 otherwise.
 */
int client_post_probe (CONFIG config);

/**
 * Runs compression detection for part 2.
 * @param config The config struct.
 * @return Returns 0 if there is no error, 1 otherwise.
 */
int compdetect_single (CONFIG config);

/**
 * Runs compression detection by opening up the config json and turning the config json into a CONFIG.
 * Then compdetect_single is run to run compression detection.
 * @param config_path The path to the configuration json file.
 * @return Returns 0 if there is no error, 1 otherwise.
 */
int run_compdetect (char *config_path);

/**
 * Function for sending a UDP packet train for use in parts 1 and 2.
 * A UDP_CLIENT_CONN is used to send the data.
 * The data in the buf are sent and assumed to be of length udp_payload_size from the conifg.
 * This can work with both high and low entropy data. The data just needs to be in the buf.
 *
 * @param udp_client The UDP_CLIENT_CONN used to send the data.
 * @param config The config json data.
 * @param buf The buffer containing the data to send.
 * @param t The train_type either high or low for low or high entropy data.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int send_udp_train (UDP_CLIENT_CONN udp_client, CONFIG config, enum train_type t, char *buf);

/**
 * Gets high entropy data to fill send in the high entropy packet train.
 * The data are read from the random_file in the config.
 *
 * @param config The CONFIG to use.
 * @param data The buffer to store the high entropy data.
 *             This assumes the data buffer is of length udp_payload_size in the config.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int get_high_entropy_data (CONFIG config, char data[]);

/**
 * Sends a raw syn packet with the socket sockfd.  This function both creates and sends the data.
 * @param config The CONFIG to use.
 * @param sockfd The socket to send data with.
 * @param sin The struct sockaddr_in for the sending machine.
 * @param sout The struct sockaddr_in for the server/port to send the data to.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int send_tcp_syn (CONFIG config, int sockfd, struct sockaddr_in *sin, struct sockaddr_in *sout);

/**
 * Creates a new syn packet for a raw TCP packet.
 * This resource was reviewed for making this function
 * https://github.com/MaxXor/raw-sockets-example/blob/6bf7f8bb550ccbe9e3b29d2cc632c9b91197fdd6/rawsockets.c#L49
 *
 * @param sin The struct sockaddr_in for the sending connection.
 * @param sout The struct sockaddr_in for the receiving connection.
 * @param packet The buffer to store the packet.
 * @param packet_len The length of the pacekt.
 * @param id The id of the packet.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int new_syn_packet (struct sockaddr_in *sin, struct sockaddr_in *sout, char *packet, int packet_len, int id);

/**
 * Sends a raw TCP syn packet using the connection info from sout.
 * @param sockfd The socket file descriptor.
 * @param sout The struct sockaddr_in for the receiving connection.
 * @param packet The packet to send.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int send_syn_packet (int sockfd, struct sockaddr_in *sout, char *packet);

/**
 * Creates a raw socket for the interface and puts the socket file descriptor in sockfd.
 * @param sockfd The place to store the socket file descriptor.
 *               Upon completion of this function this will point to a socket file descriptor.
 * @param interface The network interface to use for the raw socket.
 * @param config The configuration.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int create_raw_socket (int *sockfd, char *interface, CONFIG config);

/**
 * This creates a valid checksum for a IPv4 or TCP header.
 * This is from https://github.com/MaxXor/raw-sockets-example/blob/6bf7f8bb550ccbe9e3b29d2cc632c9b91197fdd6/rawsockets.c#L24 .
 * @param buf The buffer to create the checksum with.
 * @param size The size of the buf.
 * @return The checksum.
 */
unsigned short checksum (const char *buf, unsigned size);

// Struct for the RST receiver thread arguments.
struct rst_listener_args
{
    int *count;
    struct timeb *recv_times;
    int *sockfd;
    CONFIG config;
    struct sockaddr_in *sin;
    struct sockaddr_in *head_sockaddr_in;
};

/**
 * This function sets up a listener for receiving RST packets over a raw TCP socket.
 * The listener is ran in a separate thread.
 * @param rst_listener_thread The pointer to the thread id.
 * @param args The arguments to send to the listener thread.
 * @param config The config to use.
 * @param recv_times An array of timeb for use of storing the RST received times.
 *                   This has a length of RST_PACKET_TOTAL in the constants.h
 * @param sockfd The raw TCP socket to listen for the RST packets.
 * @param sin The struct sockaddr_in for the listeners machine.
 * @param head_sockaddr_in The struct sockaddr_in for the tcp head port/host
 * @return Returns 0 if the listener was set up without errors, 1 otherwise.
 */
int start_rst_listener (pthread_t *rst_listener_thread, struct rst_listener_args *args,
                        CONFIG config, struct timeb *recv_times, int sockfd, struct sockaddr_in *sin,
                        struct sockaddr_in *head_sockaddr_in);

/**
 * Prints the results of the RST packets received.
 * @param recv_times The array of received times.
 * @param rst_count The number of RST packets to check.
 */
void print_results (struct timeb *recv_times, const int rst_count);

/**
 * Computes the time between two struct timeb
 * @param time_1 start time
 * @param time_2 end time
 * @return The time diff
 */
double compute_time_diff (struct timeb time_1, struct timeb time_2);

/**
 * Gets low entropy data from the client.
 * @param client_handler The UDP_HANDLER for the client connection on the server.
 * @param config The config data.
 * @param type The train_type either high or low for low or high entropy data.
 * @param result The result data for reporting purposes. This is an empty buffer that is populated.
 * @param low_entropy_duration The result for the time it takes for the low entropy data to send.
 * @return Returns 0 if no errors occurred, 1 otherwise.
 */
int get_low_entropy_data (UDP_HANDLER client_handler, CONFIG config,
                          enum train_type type, char *result, double *low_entropy_duration);

/**
 * Prints out if compression is detected.
 * @param result The result that stores the information if compression is detected.
 * @param low_entropy_duration The low entropy duration in milliseconds.
 * @param high_entropy_duration The high entropy duration in milliseconds.
 * @return Returns 0 if no errors, 1 otherwise.
 */
int process_comp (char *result, double low_entropy_duration, double high_entropy_duration);

/**
 * Processes the received times for UDP packets in part 1.
 * The result is stored in the result buffer and the mss value for the time it takes to receive the packet train.
 * This works for high or low entropy data, this is differentiated by the enum train_type t
 *
 * @param config The configuration data.
 * @param recv_times An array of timeb for use of storing the RST received times.
 *                   This has a length of RST_PACKET_TOTAL in the constants.h
 * @param mss A pointer to store the time in milliseconds to receive the packet train.
 * @param result The result message buffer that stores the result information.
 * @param t The type either high or low for low or high entropy data.
 * @return Returns 0 if no errors, 1 otherwise.
 */
int process_train (CONFIG config, struct timeb recv_times[], double *mss, char result[], enum train_type t);

/**
 * Sends a train of UDP high and low entropy data for part 2.
 * This sends both packet trains as well as sending the raw TCP sockets.
 *
 * @param config The configuration data.
 * @param sockfd The raw socket file descriptor used for sending data.
 * @param sin The struct sockaddr_in struct for the sending machine.
 * @param head_sockaddr_in The struct sockaddr_in for the TCP SYN head port/host info.
 * @param tail_sockaddr_in The struct sockaddr_in for the TCP SYN tail port/host info.
 * @param udp_client The UDP_CLIENT used to send the UDP packet train data.
 * @return Returns 0 if no errors, 1 otherwise.
 */
int send_single_train (CONFIG config, int sockfd, struct sockaddr_in *sin, struct sockaddr_in *head_sockaddr_in,
                       struct sockaddr_in *tail_sockaddr_in, UDP_CLIENT_CONN udp_client);

/**
 * Helper function to create struct sockaddr_in's for creating and receiving raw TCP packets.
 *
 * @param config The configuration data.
 * @param sin The struct sockaddr_in struct for the sending machine.
 * @param head_sockaddr_in The struct sockaddr_in for the TCP SYN head port/host info.
 * @param tail_sockaddr_in The struct sockaddr_in for the TCP SYN tail port/host info.
 * @return Returns 0 if no errors, 1 otherwise.
 */
int setup_sockaddrs (CONFIG config, struct sockaddr_in *sin,
                     struct sockaddr_in *head_sockaddr_in, struct sockaddr_in *tail_sockaddr_in);

/**
 * Sets up all of the structs and sockets raw a raw TCP socket.  This also creates the UDP_CLIENT_CONN.
 * All of the inputs other than the config should be created beforehand and they are populated with a pointer for each variable.
 * @param config The configuration data.
 * @param sin The struct sockaddr_in pointer for the sending machine.
 * @param head_sockaddr_in The struct sockaddr_in pointer for the TCP SYN head port/host info.
 * @param tail_sockaddr_in The struct sockaddr_in pointer for the TCP SYN tail port/host info.
 * @param udp_client The UDP_CLIENT_CONN pointer that is created in this funciton.
 * @param sockfd The raw socket id pointer.
 * @return Returns 0 if no errors, 1 otherwise.
 */
int setup_raw_socket_conns (CONFIG config, struct sockaddr_in *sin, struct sockaddr_in *head_sockaddr_in,
                            struct sockaddr_in *tail_sockaddr_in, UDP_CLIENT_CONN *udp_client, int *sockfd);

/**
 * Closes the receiver thread for the RST packets.
 * @param rst_listener_thread The pointer to the thread id.
 * @return Returns 0 if no errors, 1 otherwise.
 */
int close_recv_thread (pthread_t *rst_listener_thread, CONFIG config);

/**
 * Handler to send signal to RST recv thread to end if timeout has occurred.
 * @param input The input pointer to pass args to the handler thread.
 */
void stop_thread (union sigval input);


#endif //_COMPDETECT_H_
