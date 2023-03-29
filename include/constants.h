/**
 * Constants file for storing constants and defaults for the CONFIG.
 */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// The max TCP size data size for the project.
static const int MAX_TCP_SIZE = 1024;

// Default value for udp_payload_size in the config json.
static const int UDP_PAYLOAD_SIZE = 1000;
static const int INTER_MEASURE_TIME = 15;
static const int UDP_PACKET_TRAIN_LEN = 6000;
static const int UDP_PACKET_TTL = 255;
static const int UPP_TIMEOUT = 5;
static const int RAW_PACKET_SIZE = 4096;
static const int TCP_SRC_SYN_PORT = 12059;
static const int RST_PACKET_TOTAL = 4;
static const int THRESHOLD = 100;
static const char *DEFAULT_DEVICE = "enp1s0";

// TODO move to config
static const char *random_file = "random_file";
static char *train_type_str[2] = { "low", "high"};

enum train_type
{
    low = 0,
    high = 1,
};

#endif //_CONSTANTS_H_
