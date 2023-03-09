#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcp_sock_handler.h>
#include <udp_sock_handler.h>
#include <strings.h>
#include <unistd.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>
#include "compdetect.h"
#include "config.h"
#include "constants.h"

// TODO test with running client first for all steps

int send_low_entropy_data (UDP_CLIENT_CONN udp_client, CONFIG config);
int client_pre_probe (CONFIG config, char *config_str)
{
  TCP_CLIENT_CONN client_conn = tcp_new_client (config->server_ip, config->tcp_probing_port);
  int connected = tcp_client_connect(client_conn);
  if (connected)
    {
      perror ("Client Failed to connect to server in pre probe");
      abort ();
    }
  int sent = tcp_send (client_conn->handler, config_str, strlen (config_str));
  if (sent)
    {
      perror ("Client failed to send config as string");
      return 1;
    }
  if (destroy_tcp_client (client_conn))
    {
      perror ("Client failed to destroy client socket handler");
      return 1;
    }
  return 0;
}

CONFIG server_pre_probe (int port)
{
  printf("starting server pre-probe stage\n");
  TCP_SERVER server = tcp_new_server (port);
  int started = tcp_server_start (server);
  printf("started %d\n", started);
  if (started)
    {
      perror ("Unable to start server in pre probe");
      abort ();
    }
  printf("TCP server started in server pre-probe stage\n");
  TCP_HANDLER client_handler = tcp_server_next_connection (server);
  char buf[MAX_TCP_SIZE];
  int buf_len = 0;
  int received = tcp_recv (client_handler, buf, &buf_len);
  if (received)
    {
      perror ("Server failed to get config str from client");
      abort ();
    }
  buf[buf_len] = '\0';
  printf ("server received config\n\n %s \n", buf);

  if (destroy_tcp_sever (server) || destroy_tcp_handler (client_handler))
    {
      perror ("Server failed to close tcp conn in pre probe");
      abort ();
    }
  printf ("Server converting config str into config struct\n");
  CONFIG config = config_new (buf);
  if (config == NULL)
    {
      perror ("Server failed to convert string config from client into config struct\n");
      return 1;
    }

  printf ("Server converted config str into config struct\n");
  return config;
}

int client_probe(CONFIG config)
{
  printf("Starting client pre probe on ip %s on port %d\n", config->server_ip, config->udp_dest_port);
  UDP_CLIENT_CONN udp_client = udp_new_client (config->server_ip, config->udp_dest_port);
  if (udp_client == NULL)
    {
      perror ("Failed to create udp_client");
      return 1;
    }
  printf("Setting up client upd connection\n");
  if (udp_client_connect (udp_client))
    {
      perror ("Client failed to set up UDP connection with server in client probe stage\n");
    }
  printf("Client set up client upd connection\n");
  int low = send_low_entropy_data (udp_client, config);
  if (low)
    {
      perror ("Client failed to send low entropy data");
      return 1;
    }
  if (udp_destroy_client (udp_client))
    {
      perror ("Failed to destroy upd client");
      return 1;
    }
  printf("Client pre probe stage successfully finished\n");
  return 0;
}



int server_probe (CONFIG config)
{
  printf("Starting server probe stage on port %d\n", config->udp_dest_port);
  UDP_SERVER udp_server = udp_new_server (config->udp_dest_port);
  if (udp_server == NULL)
    {
      perror ("Unable to get new UDP server for server probe");
      return 1;
    }
  printf("Starting server probe server\n");
  int start = udp_server_start (udp_server);
  if (start)
    {
      perror ("Failed to start UDP server for server probe");
      return 1;
    }

  printf("Started server probe server\n");
  UDP_HANDLER client_handler = udp_server_next_connection (udp_server);
  if (client_handler == NULL)
    {
      perror ("Failed to get next client connection for UDP probe");
      return 1;
    }
  printf("Server set up new UDP connection with client probe\n");
  int low = receive_low_entropy_data (client_handler, config);
  if (low)
    {
      perror ("Client failed to send low entropy data");
      return 1;
    }
  if (udp_server_destroy (udp_server))
    {
      perror ("Failed to destroy udp_server in server probe stage");
      return 1;
    }
  if (udp_destroy_handler (client_handler))
    {
      perror("Failed to destroy udp client handler in server probe stage");
      return 1;
    }
  return 0;
}

void
set_packet_id (char *buf, uint16_t num)
{
  buf[0] = (num >> 8) & 0xFF;
  buf[1] = num & 0xFF;
}

void
get_packet_id (char *buf, uint16_t *num)
{
  *num = (buf[0] & 0xFF) << 8 | (buf[1] & 0xFF);
}

int send_low_entropy_data (UDP_CLIENT_CONN udp_client, CONFIG config)
{
  sleep(1);
  printf("starting to send low entropy data\n");
  uint16_t packet_id = 0;
  char buf[config->udp_payload_size];
  bzero (buf, config->udp_payload_size);
  int sent_success = 0;
  int sent_failed = 0;
  for (int i = 0; i < config->udp_packet_train_len; i++, packet_id++)
    {
      usleep (100);
      printf("sending packet id %d\n", packet_id);
      set_packet_id (buf, packet_id);
      int sent = udp_sendto_n (udp_client->handler, buf, config->udp_payload_size);
      if (sent)
        sent_failed++;
      else
        sent_success++;
    }
  printf("Sent low entropy data from client with %d success %d failed\n", sent_success, sent_failed);
  return 0;
}

int receive_low_entropy_data (UDP_HANDLER client_handler, CONFIG config)
{
  printf ("Server starting low entropy receive\n");
  char buf[config->udp_payload_size];
  struct timeb recv_times[config->udp_packet_train_len];
  bzero (recv_times, sizeof (struct timeb) * config->udp_packet_train_len);
  int sent_success = 0;
  int sent_failed = 0;
  for (int i = 0; i < config->udp_packet_train_len; i++)
    {
      int received = udp_recvfrom_n (client_handler, buf, config->udp_payload_size);
      uint16_t packet_id = 0;
      get_packet_id (buf, &packet_id);

      if (received)
        sent_failed++;
      else
        {
          printf ("trying to add packet id %d\n", packet_id);
          struct timeb recv_time;
          ftime(&recv_time);
          recv_times[packet_id] = recv_time;
          sent_success++;
          printf ("added packet id %d\n", packet_id);
        }
    }
  printf ("ended receive\n");
  int start = -1;
  int end = -1;
  for (int i = 0; i < config->udp_packet_train_len; i++)
    {
      if (recv_times[i].millitm != 0)
          start = i;
    }
  for (int i = config->udp_packet_train_len; i > 0; i--)
    {
      if (recv_times[i].millitm != 0)
          end = i;
    }
  if (start != -1 && end != -1)
    {
      printf ("start time %s", asctime(gmtime(&recv_times[start].time)));
      printf ("end time %s", asctime(gmtime(&recv_times[end].time)));
      printf ("It took %.f seconds between packet %d and %d\n", difftime(recv_times[start].time, recv_times[end].time), start, end);
    }
  else
    printf ("could not get start or end time\n");
  printf ("Sent low entropy data received from client with %d success %d failed\n", sent_success, sent_failed);
  return 0;
}
