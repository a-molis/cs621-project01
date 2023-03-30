/**
 * Constants file for storing constants and defaults for the CONFIG.
 */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// The max TCP size data size for the project.
static const int MAX_TCP_SIZE = 1024;

// Default value for udp_payload_size in the config json.
static const int UDP_PAYLOAD_SIZE = 1000;

// Default for inter_measure_time in config json.
static const int INTER_MEASURE_TIME = 15;

// Default for udp_packet_train_len in config json.
static const int UDP_PACKET_TRAIN_LEN = 6000;

// Default for udp_packet_ttl in config json.
static const int UDP_PACKET_TTL = 255;

// Timeout time to recv data in udp packet train in part 1.
static const int UPP_TIMEOUT = 5;

// This is the max size of a raw packet used for part 2.  Default value for raw_packet_size in config.
static const int RAW_PACKET_SIZE = 4096;

// Default for tcp_src_syn_port in config.  This is the port used for the source port for sending the UDP packet train in part 2.
static const int TCP_SRC_SYN_PORT = 12059;

// Total number of RST packets expected to receive.
static const int RST_PACKET_TOTAL = 4;

// The threshold value to say if compression is detected.
static const int THRESHOLD = 100;

// The default value for recv_device in the config. This is used for the device name for the raw TCP socket.
static const char *DEFAULT_DEVICE = "enp1s0";

// This is the random file name.
static const char *random_file = "random_file";

// This is used for printing out high or low entropy type for the train_type enum.
static char *train_type_str[2] = { "low", "high"};

// Enum used to differentiate if high or low entropy data is sent or received.
enum train_type
{
    low = 0,
    high = 1,
};

/**
 * This is used for raw packet window size.
 * This is based off a window size the OS picked previously for part 1 packets.
 */
static unsigned short WINDOW_SIZE = 64240;

// Max value for seq num in raw TCP packet.
static unsigned int MAX_SEQ_NUM = 500;

#endif //_CONSTANTS_H_
