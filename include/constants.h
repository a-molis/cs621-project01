

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_
static const int MAX_TCP_SIZE = 1024;
static const int MAX_UDP_SIZE = 1000;
static const int UDP_PAYLOAD_SIZE = 1000;
static const int INTER_MEASURE_TIME = 15;
static const int UDP_PACKET_TRAIN_LEN = 6000;
static const int UDP_PACKET_TTL = 255;
static const int UPP_TIMEOUT = 2;

// TODO move to config
static const char *random_file = "random_file";
static char *train_type_str[2] = { "low", "high"};

enum train_type
{
    low = 0,
    high = 1,
};

#endif //_CONSTANTS_H_
