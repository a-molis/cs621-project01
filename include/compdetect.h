#include "config.h"

#ifndef _COMPDETECT_H_
#define _COMPDETECT_H_

int client_pre_probe (CONFIG config, char *config_str);
CONFIG server_pre_probe (int port);
int client_probe(CONFIG config);
int server_probe (CONFIG config);
int receive_low_entropy_data (UDP_HANDLER udp_client, CONFIG config);

#endif //_COMPDETECT_H_
