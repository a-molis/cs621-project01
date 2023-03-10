#include "config.h"
#include "constants.h"

#ifndef _COMPDETECT_H_
#define _COMPDETECT_H_

int client_pre_probe (CONFIG config, char *config_str);
CONFIG server_pre_probe (int port);
int client_probe(CONFIG config);
int server_probe (CONFIG config);
int recv_udp_train (UDP_HANDLER client_handler, CONFIG config, enum train_type t);

#endif //_COMPDETECT_H_
