/**
 * Header file for config functions
 */
#include <stddef.h>

#ifndef CONFIG_H_
#define CONFIG_H_

// Struct to store config json data.
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
    int raw_packet_size;
    int tcp_src_syn_port;
    char recv_device[16];
};

// Renames a struct CONFIG_DATA pointer to CONFIG.
typedef struct CONFIG_DATA *CONFIG;

/**
 * Creates a new config file from a string representation of a config json
 * @param config_str The config as a string.
 * @return The config struct pointer as a CONFIG type or NULL if there was an error.
 */
CONFIG config_new (char *config_str);

/**
 * Destroys/frees the config struct.
 * @param config The config struct to destroy.
 */
void config_destroy (CONFIG config);

/**
 * Opens a file with a path and stores the file in the buf.
 * @param path The path to the file to read.
 * @param buf The buffer to store the opened_file.
 * @return Returns 0 if there is no error 1 otherwise.
 */
int open_file (char *path, char *buf);

/**
 * Opens the config from the config_path.
 * Convert the config json into a CONFIG struct.
 * The string representation of the json is stored in the buf.
 * @param config_path The path to the config file.
 * @param buf The buffer to store the json as a string.
 * @return The config struct pointer as a CONFIG type or NULL if there was an error.
 */
CONFIG get_config (char *config_path, char *buf);

#endif //CONFIG_H_
