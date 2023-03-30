/**
 * These functions support opening a json config file and deserializing it into a CONFIG.
 */
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "constants.h"
#include "cJSON.h"

/**
 * Parses optional config parameters.
 * @param config The CONFIG to add the parameters to.
 * @param json The cJSON struct that contains the json data.
 * @return Returns 0 if there are no errors, 1 otherwise.
 */
int parse_optional_params (CONFIG config, const cJSON *json);

/**
 * Parses required config parameters.
 * @param config The CONFIG to add the parameters to.
 * @param json The cJSON struct that contains the json data.
 * @return Returns 0 if there are no errors, 1 otherwise.
 */
int parse_required_params (CONFIG config, const cJSON *json);

CONFIG
config_new (char *config_str)
{
  CONFIG config = malloc (sizeof (struct CONFIG_DATA));
  if (config == NULL)
    {
      perror ("Unable to malloc new config struct");
      return NULL;
    }
  cJSON *json = cJSON_ParseWithLength (config_str, strlen (config_str));
  if (json == NULL)
    {
      perror ("Error parsing config");
      free (config);
      return NULL;
    }
  if (parse_required_params (config, json))
    {
      perror ("Error parsing required params");
      return NULL;
    }
  parse_optional_params (config, json);
  cJSON_Delete(json);
  return config;
}

void
config_destroy (CONFIG config)
{
  if (config)
    free (config);
}

int
parse_required_params (CONFIG config, const cJSON *json)
{
  // Required config parameters
  cJSON *server_ip = cJSON_GetObjectItem (json, "server_ip");
  cJSON *udp_source_port = cJSON_GetObjectItem (json, "udp_source_port");
  cJSON *udp_dest_port = cJSON_GetObjectItem (json, "udp_dest_port");
  cJSON *tcp_dest_head_syn_port = cJSON_GetObjectItem (json, "tcp_dest_head_syn_port");
  cJSON *tcp_dest_tail_syn_port = cJSON_GetObjectItem (json, "tcp_dest_tail_syn_port");
  cJSON *tcp_probing_port = cJSON_GetObjectItem (json, "tcp_probing_port");
  cJSON *client_ip = cJSON_GetObjectItem (json, "client_ip");

  if (!server_ip || !udp_source_port || !udp_dest_port || !tcp_dest_head_syn_port || !tcp_dest_tail_syn_port
      || !tcp_probing_port || !client_ip)
    {
      free (config);
      perror ("Missing required config parameter");
      return 1;
    }
  strcpy (config->server_ip, server_ip->valuestring);
  config->udp_source_port = udp_source_port->valueint;
  config->udp_dest_port = udp_dest_port->valueint;
  config->tcp_dest_head_syn_port = tcp_dest_head_syn_port->valueint;
  config->tcp_dest_tail_syn_port = tcp_dest_tail_syn_port->valueint;
  config->tcp_probing_port = tcp_probing_port->valueint;
  strcpy (config->client_ip, client_ip->valuestring);
  return 0;
}

int
parse_optional_params (CONFIG config, const cJSON *json)
{
  // Optional config parameters
  cJSON *udp_payload_size = cJSON_GetObjectItem (json, "udp_payload_size");
  cJSON *inter_measure_time = cJSON_GetObjectItem (json, "inter_measure_time");
  cJSON *udp_packet_train_len = cJSON_GetObjectItem (json, "udp_packet_train_len");
  cJSON *udp_packet_ttl = cJSON_GetObjectItem (json, "udp_packet_ttl");
  cJSON *raw_packet_size = cJSON_GetObjectItem (json, "raw_packet_size");
  cJSON *tcp_src_syn_port = cJSON_GetObjectItem (json, "tcp_src_syn_port");
  cJSON *recv_device = cJSON_GetObjectItem (json, "recv_device");

  if (!udp_payload_size || udp_payload_size->valueint == 0)
    config->udp_payload_size = UDP_PAYLOAD_SIZE;
  else
    config->udp_payload_size = udp_payload_size->valueint;

  if (!inter_measure_time || inter_measure_time->valueint == 0)
    config->inter_measure_time = INTER_MEASURE_TIME;
  else
    config->inter_measure_time = inter_measure_time->valueint;

  if (!udp_packet_train_len || udp_packet_train_len->valueint == 0)
    config->udp_packet_train_len = UDP_PACKET_TRAIN_LEN;
  else
    config->udp_packet_train_len = udp_packet_train_len->valueint;

  if (!udp_packet_ttl || udp_packet_ttl->valueint == 0)
    config->udp_packet_ttl = UDP_PACKET_TTL;
  else
    config->udp_packet_ttl = udp_packet_ttl->valueint;

  if (!raw_packet_size || raw_packet_size->valueint == 0)
    config->raw_packet_size = RAW_PACKET_SIZE;
  else
    config->raw_packet_size = raw_packet_size->valueint;

  if (!tcp_src_syn_port || tcp_src_syn_port->valueint == 0)
    config->tcp_src_syn_port = TCP_SRC_SYN_PORT;
  else
    config->tcp_src_syn_port = tcp_src_syn_port->valueint;
  if (!recv_device)
    strcpy (config->recv_device, DEFAULT_DEVICE);
  else
    strcpy (config->recv_device, recv_device->valuestring);

  return 0;
}

CONFIG
get_config (char *config_path, char *buf)
{
  int opened = open_file(config_path, buf);
  if (opened)
    {
      perror ("Failed to open config file");
      return NULL;
    }
  return config_new(buf);
}

int
open_file (char *path, char *buf)
{
  FILE *fd = fopen(path, "r");
  if (!fd)
    {
      printf("Failed to open file %s\n", path);
      return 1;
    }
  // TODO move to constants
  int size = 1024;
  size_t offset = 0;
  while (fgets(buf + offset, size, fd) != NULL)
    {
      offset = strlen(buf);
    }
  if (fclose (fd))
    {
      perror ("Unable to close file");
      return 1;
    }
  return 0;
}