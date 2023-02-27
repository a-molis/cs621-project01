#include "config.h"

#ifndef _COMPDETECT_H_
#define _COMPDETECT_H_

int client_pre_probe (CONFIG config, char *config_str);
CONFIG server_pre_probe (int port);


#endif //_COMPDETECT_H_
