#include <stddef.h>

#ifndef CONFIG_H_
#define CONFIG_H_

struct CONFIG_DATA
{
    char server_ip[16];
    int udp_source_port;
    int udp_dest_port;
    int tcp_dest_head_syn_port;
    int tcp_dest_tail_syn_port;
    int tcp_probing_port;
    int udp_payload_size;
    int inter_measure_time;
    int udp_packet_train_len;
    int udp_packet_ttl;
};

typedef struct CONFIG_DATA *CONFIG;

CONFIG config_new (char *config_str);
void config_destroy(CONFIG config);

#endif //CONFIG_H_
