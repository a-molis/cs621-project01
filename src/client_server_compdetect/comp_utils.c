/**
 * Functions for part 1 and 2 business logic
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/timeb.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <linux/if.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include "comp_utils.h"
#include "config.h"
#include "constants.h"
#include "tcp_sock_handler.h"
#include "udp_sock_handler.h"

/**
 * Sets the packet id
 * @param buf The buffer to set the id.
 * @param num The number for the id.
 */
void set_packet_id (char *buf, uint16_t num);

int
client_pre_probe (CONFIG config, char *config_str)
{
  TCP_CLIENT_CONN client_conn = tcp_new_client (config->server_ip, config->tcp_probing_port);
  int connected = tcp_client_connect (client_conn);
  if (connected)
    {
      perror ("Client Failed to connect to server in pre probe");
      return 1;
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

CONFIG
server_pre_probe (int port)
{
  TCP_SERVER server = tcp_new_server (port);
  if (server == NULL)
    {
      perror ("Error creating TCP server");
      return NULL;
    }
  if (tcp_server_start (server))
    {
      if (destroy_tcp_sever (server))
        printf ("Error destroying server");
      perror ("Unable to start server in pre probe");
      return NULL;
    }
  TCP_HANDLER client_handler = tcp_server_next_connection (server);
  if (client_handler == NULL)
    {
      if (destroy_tcp_sever (server))
        printf ("Error destroying server");
      perror ("Failed to create client_handler in server_pre_probe");
      return NULL;
    }
  char buf[MAX_TCP_SIZE];
  int buf_len = 0;
  if (tcp_recv (client_handler, buf, &buf_len))
    {
      if (destroy_tcp_sever (server) | destroy_tcp_handler (client_handler))
        printf ("error destroying server or handler");
      perror ("Server failed to get config str from client");
      return NULL;
    }
  buf[buf_len] = '\0';

  if (destroy_tcp_sever (server))
    {
      perror ("Server failed to close tcp conn in pre probe");
      return NULL;
    }
  if (destroy_tcp_handler (client_handler))
    {
      perror ("Server failed to close client_handler in pre probe");
      return NULL;
    }
  CONFIG config = config_new (buf);
  if (config == NULL)
    {
      perror ("Server failed to convert string config from client into config struct\n");
      return NULL;
    }
  return config;
}

int
client_probe (CONFIG config)
{
  UDP_CLIENT_CONN udp_client = udp_new_client (config->server_ip, config->udp_dest_port);
  if (udp_client == NULL)
    {
      perror ("Failed to create udp_client");
      return 1;
    }
  if (udp_client_connect_bind (udp_client, config->udp_source_port))
    {
      if (udp_destroy_client (udp_client))
        {
          printf ("Failed to destroy upd client");
        }
      perror ("Client failed to set up UDP connection with server in client probe stage\n");
      return 1;
    }
  char buf[config->udp_payload_size];
  bzero (buf, config->udp_payload_size);
  if (send_udp_train (udp_client, config, low, buf))
    {
      if (udp_destroy_client (udp_client))
        {
          printf ("Failed to destroy upd client");
        }
      perror ("Client failed to send low entropy data");
      return 1;
    }
  sleep (config->inter_measure_time);
  char high_data[config->udp_payload_size];
  if (get_high_entropy_data (config, high_data))
    {
      if (udp_destroy_client (udp_client))
        {
          printf ("Failed to destroy upd client");
        }
      perror ("Unable to open high entropy data");
      return 1;
    }
  if (send_udp_train (udp_client, config, high, high_data))
    {
      if (udp_destroy_client (udp_client))
        {
          printf ("Failed to destroy upd client");
        }
      perror ("Client failed to send low entropy data");
      return 1;
    }
  if (udp_destroy_client (udp_client))
    {
      perror ("Failed to destroy upd client");
      return 1;
    }
  return 0;
}

int
get_high_entropy_data (CONFIG config, char data[])
{
  bzero (data, config->udp_payload_size);
  FILE *fd = fopen (random_file, "r");
  if (!fd)
    {
      printf ("Failed to open file %s\n", random_file);
      return 1;
    }
  int read_len = config->udp_payload_size - 2;
  if (fread (data + 2, sizeof (char), read_len, fd) < read_len)
    {
      perror ("Unable to read data from high entropy file\n");
      return 1;
    }
  if (fclose (fd))
    {
      perror ("Unable to close file");
      return 1;
    }
  return 0;
}

void signal_handler ()
{
  return;
}

int
server_probe (CONFIG config, char *result)
{
  UDP_SERVER udp_server = udp_new_server (config->udp_dest_port);
  if (udp_server == NULL)
    {
      perror ("Unable to get new UDP server for server probe");
      return 1;
    }
  if (udp_server_start (udp_server))
    {
      if (udp_server_destroy (udp_server))
        printf ("Failed to destroy udp_server\n");
      perror ("Failed to start UDP server for server probe");
      return 1;
    }
  UDP_HANDLER client_handler = udp_server_next_connection (udp_server);
  if (client_handler == NULL)
    {
      if (udp_server_destroy (udp_server) | udp_destroy_handler (client_handler))
        printf ("Failed to destroy udp_server\n");
      perror ("Failed to get next client connection for UDP probe");
      return 1;
    }
  double low_entropy_duration;
  if (get_low_entropy_data (client_handler, config, low, result, &low_entropy_duration)) {
      if (udp_server_destroy (udp_server) | udp_destroy_handler (client_handler))
        printf ("Failed to destroy udp_server\n");
      perror ("error getting low entropy data");
      return 1;
  }
  signal (SIGALRM, signal_handler);
  int timeout = config->inter_measure_time * 1.3;
  alarm (timeout);
  double high_entropy_duration;
  size_t len = strlen (result);
  if (recv_udp_train (client_handler, config, high, result + len, &high_entropy_duration))
    {
      if (udp_server_destroy (udp_server) | udp_destroy_handler (client_handler))
        printf ("Failed to destroy udp_server\n");
      perror ("Client failed to send high entropy data");
      return 1;
    }
  if (udp_server_destroy (udp_server) | udp_destroy_handler (client_handler))
    {
      perror ("Failed to destroy udp_server or client_handler in server probe stage");
      return 1;
    }
  if (process_comp (result, low_entropy_duration, high_entropy_duration))
    {
      perror ("Failed to get entropy detection");
      return 1;
    }
  return 0;
}

int
process_comp (char *result, double low_entropy_duration, double high_entropy_duration)
{
  int len = strlen (result);
  sprintf (result + len, "Compression detected: ");
  len = strlen (result);
  if (high_entropy_duration - low_entropy_duration > THRESHOLD)
    sprintf (result + len, "True\n");
  else
    sprintf (result + len, "False\n");
  alarm (0);
  return 0;
}

int
get_low_entropy_data (UDP_HANDLER client_handler, CONFIG config, enum train_type type,
                      char *result, double *low_entropy_duration)
{
  signal (SIGALRM, signal_handler);
  int timeout = config->inter_measure_time / 3;
  alarm (timeout);
  if (recv_udp_train (client_handler, config, low, result, low_entropy_duration))
    {
      perror ("Client failed to send low entropy data");
      return 1;
    }
  size_t len = strlen (result);
  if (len == 0)
    {
      printf ("len %lu\n", len);
      perror ("Failed to get result for low entropy data\n");
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

int send_udp_train (UDP_CLIENT_CONN udp_client, CONFIG config, enum train_type t, char *buf)
{
  uint16_t packet_id = 0;
  for (int i = 0; i < config->udp_packet_train_len; i++, packet_id++)
    {
      set_packet_id (buf, packet_id);
      udp_sendto_n (udp_client->handler, buf, config->udp_payload_size);
    }
  return 0;
}

int
recv_udp_train (UDP_HANDLER client_handler, CONFIG config, enum train_type t, char result[], double *mss)
{
  char buf[config->udp_payload_size];
  struct timeb recv_times[config->udp_packet_train_len];
  bzero (recv_times, sizeof (struct timeb) * config->udp_packet_train_len);
  for (int i = 0; i < config->udp_packet_train_len; i++)
    {
      int received = udp_recvfrom_n (client_handler, buf, config->udp_payload_size);
      uint16_t packet_id = 0;
      get_packet_id (buf, &packet_id);

      if (received == EINTR)
        {
          break;
        }
      if (!received)
        {
          struct timeb recv_time;
          ftime (&recv_time);
          recv_times[packet_id] = recv_time;
        }
    }
  if (process_train (config, recv_times, mss, result, high))
    {
      perror ("Error processing train");
      return 1;
    }
  return 0;
}

int
process_train (CONFIG config, struct timeb recv_times[], double *mss, char result[], enum train_type t)
{
  int start = -1;
  int end = -1;
  for (int i = 0; i < config->udp_packet_train_len; i++)
    {
      if (recv_times[i].millitm != 0)
        {
          start = i;
          break;
        }
    }
  for (int i = config->udp_packet_train_len - 1; i > 0; i--)
    {
      if (recv_times[i].millitm != 0)
        {
          end = i;
          break;
        }
    }
  if (start != -1 && end != -1)
    {
      *mss = (1000 * difftime(recv_times[end].time, recv_times[start].time)) +
             recv_times[end].millitm -  recv_times[start].millitm;
      sprintf (result, "It took %.f ms between packet between packets %d and %d for %s entropy data\n",
               *mss, start, end, train_type_str[t]);
    }
  else
    {
      printf ("could not get start or end time for %s entropy data\n", train_type_str[t]);
      sprintf (result, "Error getting %s entropy data\n", train_type_str[t]);
      *mss = -1;
      return 1;
    }
  return 0;
}

int
server_post_probe (CONFIG config, char *result)
{
  TCP_SERVER server = tcp_new_server (config->tcp_probing_port);
  if (server == NULL)
    {
      perror ("Error creating new TCP_SERVER");
      return 1;
    }
  if (tcp_server_start (server))
    {
      if (destroy_tcp_sever (server))
        printf ("Failed to destroy TCP_SERVER\n");
      perror ("Unable to start server in pre probe");
      return 1;
    }
  TCP_HANDLER client_handler = tcp_server_next_connection (server);
  if (client_handler == NULL)
    {
      if (destroy_tcp_sever (server))
        printf ("Failed to destroy TCP_SERVER\n");
      perror ("Unable to create TCP_HANDLER in pre probe");
      return 1;
    }
  if (tcp_send (client_handler, result, strlen (result)))
    {
      if (destroy_tcp_sever (server) | destroy_tcp_handler (client_handler))
        printf ("Failed to destroy TCP_SERVER or client handler\n");
      perror ("Server failed to send post probe data");
      return 1;
    }
  if (destroy_tcp_sever (server) | destroy_tcp_handler (client_handler))
    {
      perror ("Server failed to close tcp conn in post-probe");
      return 1;
    }
  return 0;
}

int
client_post_probe (CONFIG config)
{
  TCP_CLIENT_CONN client_conn = tcp_new_client (config->server_ip, config->tcp_probing_port);
  if (client_conn == NULL)
    {
      perror ("Unable to create TCP_CLIENT_CONN in client post probe\n");
      return 1;
    }
  if (tcp_client_connect(client_conn))
    {
      perror ("Client Failed to connect to server in post probe");
      return 1;
    }
  int buf_len = 0;
  char buf[MAX_TCP_SIZE];
  if (tcp_recv (client_conn->handler, buf, &buf_len))
    {
      perror ("Client failed to get results from server in post probe");
      return 1;
    }
  buf[buf_len] = '\0';
  printf ("\nResults from server: \n%s", buf);
  if (destroy_tcp_client (client_conn))
    {
      perror ("Client failed to destroy client socket handler in post probe");
      return 1;
    }
  return 0;
}

int
compdetect_single (CONFIG config)
{
  struct sockaddr_in sin;
  struct sockaddr_in head_sockaddr_in;
  struct sockaddr_in tail_sockaddr_in;
  UDP_CLIENT_CONN udp_client;
  pthread_t rst_listener_thread;
  struct rst_listener_args *args = (struct rst_listener_args *) malloc (sizeof (struct rst_listener_args));
  if (args == NULL)
    {
      perror ("Unable to malloc rst listener args");
      return 1;
    }
  struct timeb *recv_times = (struct timeb *) malloc (sizeof (struct timeb) * RST_PACKET_TOTAL);
  if (recv_times == NULL)
    {
      perror ("Unable to create recv_times array");
      free (args);
      return 1;
    }
  memset (recv_times, 0, sizeof (struct timeb) * RST_PACKET_TOTAL );
  int sockfd = -1;
  if (setup_raw_socket_conns (config, &sin, &head_sockaddr_in, &tail_sockaddr_in, &udp_client, &sockfd))
    {
      perror ("Error setting up raw socket conns");
      return 1;
    }
  if (start_rst_listener (&rst_listener_thread, args, config, recv_times, sockfd, &sin, &head_sockaddr_in))
    {
      perror ("Failed to set up thread for receiving RST packets");
      if (udp_destroy_client (udp_client))
        printf ("Failed to destroy client");
      close (sockfd);
      free (args);
      free (recv_times);
      return 1;
    }
  if (send_single_train (config, sockfd, &sin, &head_sockaddr_in, &tail_sockaddr_in, udp_client))
    {
      perror ("Failed to send packet train for compdetect");
      if (udp_destroy_client (udp_client))
        printf ("Failed to destroy client");
      free (args);
      close (sockfd);
      free (recv_times);
      return 1;
    }
  if (udp_destroy_client (udp_client))
    {
      perror ("Failed to set up thread for receiving RST packets");
      free (args);
      free (recv_times);
      close (sockfd);
      perror ("Failed to destroy upd client");
      return 1;
    }
  if (close_recv_thread (&rst_listener_thread, config))
    {
      perror ("Error closing thread for recv");
      free (args);
      close (sockfd);
      free (recv_times);
      return 1;
    }
  free (args);
  close (sockfd);
  free (recv_times);
  return 0;
}

// Struct for argument data to the stop_thread handler.
struct thread_info
{
  pthread_t rst_listener_thread;
};

void
stop_thread (union sigval input)
{
  struct thread_info *thread_data = (struct thread_info *) input.sival_ptr;
  printf ("Timeout reached for receiving RST packets\n");
  pthread_kill (thread_data->rst_listener_thread, SIGALRM);
}

int
close_recv_thread (pthread_t *rst_listener_thread, CONFIG config)
{
  // Reviewed this source on how to create a timer
  // https://opensource.com/article/21/10/linux-timers
  timer_t id = 0;
  struct thread_info data;
  data.rst_listener_thread = *rst_listener_thread;
  struct sigevent sig;
  memset (&sig, 0, sizeof (sig));
  struct itimerspec timer;
  int timeout_duration = config->inter_measure_time / 3;
  timer.it_value.tv_sec = timeout_duration;
  timer.it_value.tv_nsec = 0;
  timer.it_interval.tv_nsec = 0;
  timer.it_interval.tv_sec = 0;
  sig.sigev_notify = SIGEV_THREAD;
  sig.sigev_notify_function = &stop_thread;
  sig.sigev_value.sival_ptr = &data;

  if (timer_create (CLOCK_REALTIME, &sig, &id))
    {
      perror ("Error creating timer for rst listener");
      return 1;
    }
  if (timer_settime (id, 0, &timer, NULL))
    {
      perror ("Error starting timer for rst listener");
      return 1;
    }

  if (pthread_join (*rst_listener_thread, NULL))
    {
      perror ("Error joining thread for rst listener");
      return 1;
    }
  return 0;
}

int
setup_raw_socket_conns (CONFIG config, struct sockaddr_in *sin, struct sockaddr_in *head_sockaddr_in,
                        struct sockaddr_in *tail_sockaddr_in, UDP_CLIENT_CONN *udp_client, int *sockfd)
{
  *udp_client = udp_new_client (config->server_ip, config->udp_dest_port);
  if ((*udp_client) == NULL)
    {
      perror ("Failed to create udp_client");
      return 1;
    }
  if (udp_client_connect_bind ((*udp_client), config->udp_source_port))
    {
      if (udp_destroy_client ((*udp_client)))
        printf ("Failed to destroy client");
      perror ("Client failed to set up UDP connection with server in client probe stage\n");
      return 1;
    }
  int sock = (*udp_client)->handler->sockfd;
  int ttl = config->udp_packet_ttl;
  if (setsockopt (sock, IPPROTO_IP, IP_TTL, &ttl, sizeof (ttl)) < 0)
    {
      perror ("Failed to set socket option for UDP TTL");
      if (udp_destroy_client (*udp_client))
        printf ("Failed to destroy client");
      return 1;
    }
  if (setup_sockaddrs (config, sin, head_sockaddr_in, tail_sockaddr_in))
    {
      perror ("Failed to setup sockaddrs");
      if (udp_destroy_client ((*udp_client)))
        printf ("Failed to destroy client");
      return 1;
    }
  char *device = config->recv_device;
  if (create_raw_socket (sockfd, device, config))
    {
      perror ("Unable to create raw socket");
      if (udp_destroy_client ((*udp_client)))
        printf ("Failed to destroy client");
      return 1;
    }
  return 0;
}

int
setup_sockaddrs (CONFIG config, struct sockaddr_in *sin,
                 struct sockaddr_in *head_sockaddr_in, struct sockaddr_in *tail_sockaddr_in)
{
  memset (sin, 0, sizeof ((*sin)));
  sin->sin_addr.s_addr = inet_addr (config->client_ip);
  sin->sin_port = htons (config->tcp_src_syn_port);
  sin->sin_family = AF_INET;

  memset (head_sockaddr_in, 0, sizeof ((*head_sockaddr_in)));
  head_sockaddr_in->sin_addr.s_addr = inet_addr (config->server_ip);
  head_sockaddr_in->sin_port = htons (config->tcp_dest_head_syn_port);
  head_sockaddr_in->sin_family = AF_INET;

  memset (tail_sockaddr_in, 0, sizeof ((*tail_sockaddr_in)));
  tail_sockaddr_in->sin_addr.s_addr = inet_addr (config->server_ip);
  tail_sockaddr_in->sin_port = htons (config->tcp_dest_tail_syn_port);
  tail_sockaddr_in->sin_family = AF_INET;
  return 0;
}

int
send_single_train (CONFIG config, int sockfd, struct sockaddr_in *sin, struct sockaddr_in *head_sockaddr_in,
                   struct sockaddr_in *tail_sockaddr_in, UDP_CLIENT_CONN udp_client)
{
  if (send_tcp_syn (config, sockfd, sin, head_sockaddr_in))
    {
      perror ("Failed to send_tcp_syn packet");
      return 1;
    }
  char buf[config->udp_payload_size];
  bzero (buf, config->udp_payload_size);
  if (send_udp_train (udp_client, config, low, buf))
    {
      perror ("Client failed to send low entropy data");
      return 1;
    }
  if (send_tcp_syn (config, sockfd, sin, tail_sockaddr_in))
    {
      perror ("Failed to send_tcp_syn packet");
      return 1;
    }
  sleep (config->inter_measure_time);
  if (send_tcp_syn (config, sockfd, sin, head_sockaddr_in))
    {
      perror ("Failed to send_tcp_syn packet");
      return 1;
    }
  char high_data[config->udp_payload_size];
  if (get_high_entropy_data (config, high_data))
    {
      perror ("Unable to open high entropy data");
      return 1;
    }
  if (send_udp_train (udp_client, config, high, high_data))
    {
      perror ("Client failed to send low entropy data");
      return 1;
    }
  if (send_tcp_syn (config, sockfd, sin, tail_sockaddr_in))
    {
      perror ("Failed to send_tcp_syn packet");
      return 1;
    }
  return 0;
}

void
recv_rst (void *inputs)
{
  signal (SIGALRM, signal_handler);
  struct rst_listener_args *args = (struct rst_listener_args*) inputs;
  int sock = *args->sockfd;

  ssize_t received;
  char buf[args->config->raw_packet_size];
  memset (buf, 0, args->config->raw_packet_size);
  uint16_t tcp_dest_head_syn_port = htons (args->config->tcp_dest_head_syn_port);
  uint16_t tcp_dest_tail_syn_port = htons (args->config->tcp_dest_tail_syn_port);
  int count = 0;
  while (count < RST_PACKET_TOTAL)
    {
      received = recvfrom (sock, buf, args->config->raw_packet_size, 0, NULL, NULL);
      if (received == 0)
        {
          printf ("connection closed\n");
          break;
        }
      else if (received < 0)
        {
          break;
        }
      if (errno){
        printf ("break %d\n", errno);
        break;
      }
      struct iphdr *ip = (struct iphdr *) buf;
      struct tcphdr *tcp = (struct tcphdr *) (buf + (ip->ihl * 4));
      if (tcp->dest == args->sin->sin_port && tcp->source == tcp_dest_head_syn_port && tcp->rst)
        {
          struct timeb recv_time;
          ftime (&recv_time);
          if (count > 1)
            *(args->recv_times + 2) = recv_time;
          else
            *(args->recv_times + 0) = recv_time;
          count += 1;
        }

      else if (tcp->dest == args->sin->sin_port && tcp->source == tcp_dest_tail_syn_port && tcp->rst)
        {
          struct timeb recv_time;
          ftime (&recv_time);
          if (count  > 1)
            *(args->recv_times + 3) = recv_time;
          else
            *(args->recv_times + 1) = recv_time;
          count += 1;
        }
    }
  print_results (args->recv_times, RST_PACKET_TOTAL);
  printf ("\n");
}

void
print_results (struct timeb *recv_times, const int rst_count)
{
  printf ("results:  \n");
  for (int i = 0; i < rst_count; i++)
    {
      struct timeb current_time = *(recv_times + i);
      if (current_time.millitm == 0)
        {
          printf ("Failed to detect due to insufficient information\n");
          return;
        }
    }
  double low_entropy_duration = compute_time_diff (*recv_times, *(recv_times + 1));
  double high_entropy_duration = compute_time_diff (*(recv_times + 2), *(recv_times + 3));
  printf ("Time between low entropy packets %.f mss and between high entropy packets %.f mss\n",
         low_entropy_duration, high_entropy_duration);
  printf ("Compression detected on link: ");
  if (high_entropy_duration - low_entropy_duration > THRESHOLD)
    printf ("True\n");
  else
    printf("False\n");
}

double
compute_time_diff (struct timeb time_1, struct timeb time_2)
{
  return (1000 * difftime(time_2.time, time_1.time)) + time_2.millitm - time_1.millitm;
}

int
start_rst_listener (pthread_t *rst_listener_thread, struct rst_listener_args *args,
                    CONFIG config, struct timeb *recv_times, int sockfd, struct sockaddr_in *sin,
                    struct sockaddr_in *head_sockaddr_in)
{
  int count = 0;
  args->config = config;
  args->recv_times = recv_times;
  args->sockfd = &sockfd;
  args->count = &count;
  args->sin = sin;
  args->head_sockaddr_in = head_sockaddr_in;
  if (pthread_create (rst_listener_thread, NULL, (void *) &recv_rst, (void *) args))
    {
      perror ("Error creating RST recv thread");
      return 1;
    }
  return 0;
}

int
send_tcp_syn (CONFIG config, int sockfd, struct sockaddr_in *sin, struct sockaddr_in *sout)
{
  char *packet = malloc (sizeof (char) * config->raw_packet_size);
  if (packet == NULL)
    {
      perror ("Error allocating packet with malloc");
      return 1;
    }
  if (new_syn_packet (sin, sout, packet, config->raw_packet_size, 1))
    {
      perror ("Error creating syn packet");
      free (packet);
      return 1;
    }
  if (send_syn_packet (sockfd, sout, packet))
    {
      perror ("Error sending raw packet for head syn packet");
      free (packet);
      return 1;
    }
  free (packet);
  return 0;
}

// pseudo header form TCP rfc and from
// https://github.com/MaxXor/raw-sockets-example/blob/6bf7f8bb550ccbe9e3b29d2cc632c9b91197fdd6/rawsockets.c#L12
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

int
new_syn_packet (struct sockaddr_in *sin, struct sockaddr_in *sout, char *packet, int packet_len, int id)
{
  bzero (packet, packet_len);
  struct iphdr *ip = (struct iphdr *) packet;
  struct tcphdr *tcp = (struct tcphdr *) (packet + sizeof (struct iphdr));
  bzero (tcp, sizeof (struct tcphdr));

  ip->version = 4;
  ip->ihl = 5;
  ip->ttl = 32;
  ip->tot_len = sizeof (struct tcphdr) + sizeof (struct iphdr);
  ip->id = htonl (id);
  ip->protocol = IPPROTO_TCP;
  ip->daddr = sout->sin_addr.s_addr;
  ip->saddr = sin->sin_addr.s_addr;

  tcp->source = sin->sin_port;
  tcp->dest = sout->sin_port;
  tcp->seq = htonl (rand() % MAX_SEQ_NUM);
  tcp->ack_seq = htonl (0);
  tcp->syn = 1;
  tcp->window = htons (WINDOW_SIZE);
  tcp->doff = 5;

  struct pseudo_header tcp_pseudo_header;
  tcp_pseudo_header.source_address = sin->sin_addr.s_addr;
  tcp_pseudo_header.dest_address = sout->sin_addr.s_addr;
  tcp_pseudo_header.protocol = IPPROTO_TCP;
  tcp_pseudo_header.placeholder = 0;
  tcp_pseudo_header.tcp_length = htons (sizeof (struct tcphdr));
  int pseudo_size = sizeof (struct pseudo_header) + sizeof (struct tcphdr);
  char* pseudo_packet = malloc (pseudo_size);
  if (pseudo_packet == NULL)
    {
      perror ("Failed to allocate memory for pseudo packet");
      return 1;
    }
  memcpy (pseudo_packet, (void *) &tcp_pseudo_header, sizeof (struct pseudo_header));
  memcpy (pseudo_packet + sizeof (struct pseudo_header), (void *) tcp, sizeof (struct tcphdr));
  tcp->check = checksum ((const char *) pseudo_packet, pseudo_size);
  ip->check = checksum (packet, ip->tot_len);
  free (pseudo_packet);
  return 0;
}

// This function is from
// https://github.com/MaxXor/raw-sockets-example/blob/6bf7f8bb550ccbe9e3b29d2cc632c9b91197fdd6/rawsockets.c#L24
unsigned short
checksum (const char *buf, unsigned size)
{
  unsigned sum = 0, i;

  /* Accumulate checksum */
  for (i = 0; i < size - 1; i += 2)
    {
      unsigned short word16 = *(unsigned short *) &buf[i];
      sum += word16;
    }

  /* Handle odd-sized case */
  if (size & 1)
    {
      unsigned short word16 = (unsigned char) buf[i];
      sum += word16;
    }

  /* Fold to get the ones-complement result */
  while (sum >> 16) sum = (sum & 0xFFFF)+(sum >> 16);

  /* Invert to get the negative in ones-complement arithmetic */
  return ~sum;
}

int
send_syn_packet (int sockfd, struct sockaddr_in *sout, char *packet)
{
  struct iphdr *ip = (struct iphdr *) packet;
  int sent  = sendto (sockfd, packet, ip->tot_len, 0, (struct sockaddr *) sout, sizeof (*sout));
  if (sent < 0)
    {
      perror ("Error sending syn packet");
      return 1;
    }

  return 0;
}

int
create_raw_socket (int *sockfd, char *interface, CONFIG config)
{
  *sockfd = socket (AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (*sockfd < 0)
    {
      perror ("Failed to open raw socket");
      return 1;
    }
  int one = 1;
  const int *val = &one;
  if (setsockopt (*sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
    {
      close (*sockfd);
      perror ("Failed to set socket opt for raw socket");
      return 1;
    }
  struct ifreq ifr;
  memset (&ifr, 0, sizeof (struct ifreq));
  strcpy (ifr.ifr_ifrn.ifrn_name, interface);
  if (ioctl (*sockfd, SIOCGIFFLAGS, &ifr) == -1)
    {
      close (*sockfd);
      perror ("Unable to get flags for network interface");
      return 1;
    }
  ifr.ifr_ifru.ifru_flags |= IFF_PROMISC;
  if (ioctl (*sockfd, SIOCGIFFLAGS, &ifr) == -1)
    {
      close (*sockfd);
      perror ("Unable to set network interface to promiscuous mode");
      return 1;
    }
  return 0;
}