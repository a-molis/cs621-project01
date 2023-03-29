/**
 * Header file for
 */
#include "config.h"
#include "constants.h"
#include "udp_sock_handler.h"

#ifndef _COMPDETECT_H_
#define _COMPDETECT_H_

int client_pre_probe (CONFIG config, char *config_str);
CONFIG server_pre_probe (int port);
int client_probe(CONFIG config);
int server_probe (CONFIG config, char *result);
int recv_udp_train (UDP_HANDLER client_handler, CONFIG config, enum train_type t, char result[], double *mss);
int server_post_probe (CONFIG config, char *result);
int client_post_probe (CONFIG config);
int compdetect_single (CONFIG config);
int run_compdetect (char *config_path);

#endif //_COMPDETECT_H_
