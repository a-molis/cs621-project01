#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "constants.h"
#include "cJSON.h"

CONFIG config_new (char *config_str)
{
  printf("starting config");
  CONFIG config = malloc (sizeof (struct CONFIG_DATA));
  if (!config)
    {
      perror ("Unable to malloc new config struct");
      abort ();
    }
  cJSON *json = cJSON_ParseWithLength (config_str, strlen (config_str));

  // Required config parameters
  cJSON *server_ip = cJSON_GetObjectItem (json, "server_ip");
  cJSON *udp_source_port = cJSON_GetObjectItem (json, "udp_source_port");
  cJSON *udp_dest_port = cJSON_GetObjectItem (json, "udp_dest_port");
  cJSON *tcp_dest_head_syn_port = cJSON_GetObjectItem (json, "tcp_dest_head_syn_port");
  cJSON *tcp_dest_tail_syn_port = cJSON_GetObjectItem (json, "tcp_dest_tail_syn_port");
  cJSON *tcp_probing_port = cJSON_GetObjectItem (json, "tcp_probing_port");

  if (!server_ip || !udp_source_port || !udp_dest_port || !tcp_dest_head_syn_port || !tcp_dest_tail_syn_port
      || !tcp_probing_port)
    {
      free (config);
      perror ("Missing required config parameter");
      abort ();
    }
  // TODO add error check if value string present and that atoi works
  strcpy (config->server_ip, server_ip->valuestring);
  config->udp_source_port = udp_source_port->valueint;
  config->udp_dest_port = udp_dest_port->valueint;
  config->tcp_dest_head_syn_port = tcp_dest_head_syn_port->valueint;
  config->tcp_dest_tail_syn_port = tcp_dest_tail_syn_port->valueint;
  config->tcp_probing_port = tcp_probing_port->valueint;


  // Optional config parameters
  cJSON *udp_payload_size = cJSON_GetObjectItem (json, "udp_payload_size");
  cJSON *inter_measure_time = cJSON_GetObjectItem (json, "inter_measure_time");
  cJSON *udp_packet_train_len = cJSON_GetObjectItem (json, "udp_packet_train_len");
  cJSON *udp_packet_ttl = cJSON_GetObjectItem (json, "udp_packet_ttl");

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

  return config;
}