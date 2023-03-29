/**
 * Header file for public functions for part 1 and 2 business logic
 */
#include "config.h"
#include "constants.h"
#include "udp_sock_handler.h"

#ifndef _COMPDETECT_H_
#define _COMPDETECT_H_

/**
 * Runs the client pre probe stage for part 1.
 * @param config The config struct.
 * @param config_str The string representation of the config to be sent to the server.
 * @return returns 0 if there is no error, 1 otherwise.
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
 * @return returns 0 if there is no error, 1 otherwise.
 */
int client_probe(CONFIG config);

/**
 * Runs the server probe stage.
 * @param config The config struct.
 * @param result A string buffer to store the result information within.
 *               This data is then sent to client in post probe stage.
 * @return returns 0 if there is no error, 1 otherwise.
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
 * @return returns 0 if there is no error, 1 otherwise.
 */
int recv_udp_train (UDP_HANDLER client_handler, CONFIG config, enum train_type t, char result[], double *mss);

/**
 * Runs server post probe. The result from the recv_udp_train call is sent in this function to the client.
 * @param config The config struct.
 * @param result The buffer for the result stats and compression detection info.
 * @return returns 0 if there is no error, 1 otherwise.
 */
int server_post_probe (CONFIG config, char *result);

/**
 * Runs the client post probe stage to get the results back from the server.
 * @param config The config struct.
 * @return returns 0 if there is no error, 1 otherwise.
 */
int client_post_probe (CONFIG config);

/**
 * Runs compression detection for part 2.
 * @param config The config struct.
 * @return returns 0 if there is no error, 1 otherwise.
 */
int compdetect_single (CONFIG config);

/**
 * Runs compression detection by opening up the config json and turning the config json into a CONFIG.
 * Then compdetect_single is run to run compression detection.
 * @param config_path The path to the configuration json file.
 * @return returns 0 if there is no error, 1 otherwise.
 */
int run_compdetect (char *config_path);

#endif //_COMPDETECT_H_
