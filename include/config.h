#include <stddef.h>

#ifndef CONFIG_H_
#define CONFIG_H_

struct CONFIG_DATA
{
    char server_ip[16];
    char client_ip[16];
    int udp_source_port;
    int udp_dest_port;
    int tcp_dest_head_syn_port;
    int tcp_dest_tail_syn_port;
    int tcp_probing_port;
    int udp_payload_size;
    int inter_measure_time;
    int udp_packet_train_len;
    int udp_packet_ttl;
    int tcp_packet_size;
    int tcp_src_syn_port;
};

typedef struct CONFIG_DATA *CONFIG;

CONFIG config_new (char *config_str);
void config_destroy(CONFIG config);
int open_file (char *path, char *buf);
CONFIG get_config (char *config_path, char *buf);

#endif //CONFIG_H_
