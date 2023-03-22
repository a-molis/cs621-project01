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

// TODO test with running client first for all steps
int send_udp_train (UDP_CLIENT_CONN udp_client, CONFIG config, enum train_type t, char *buf);
int get_high_entropy_data (CONFIG p_data, char data[]);

// TODO move to raw_comp file
int send_head_tcp_syn (CONFIG config, int sockfd, struct sockaddr_in *sin, struct sockaddr_in *sout);
int new_syn_packet (struct sockaddr_in *sin, struct sockaddr_in *sout, char *packet, int packet_len, int id);

int send_syn_packet (int sockfd, struct sockaddr_in *sout, char *packet);
int create_raw_socket (int *sockfd, char *interface);
/**
 * This is from https://github.com/MaxXor/raw-sockets-example/blob/6bf7f8bb550ccbe9e3b29d2cc632c9b91197fdd6/rawsockets.c#L24
 * @param buf The buffer to create the checksum2 with
 * @param size The size of the buf
 * @return the checksum2
 */
unsigned short checksum2(const char *buf, unsigned size);

struct rst_listener_args {
    int *count;
    struct timeb *recv_times;
    int *sockfd;
    CONFIG config;
    struct sockaddr_in *sin;
    struct sockaddr_in *head_sockaddr_in;
};

int start_rst_listener (pthread_t *rst_listenter_thread, struct rst_listener_args *args);
int
client_pre_probe (CONFIG config, char *config_str)
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
  char buf[config->udp_payload_size];
  bzero (buf, config->udp_payload_size);
  int send_low = send_udp_train (udp_client, config, low, buf);
  if (send_low)
    {
      perror ("Client failed to send low entropy data");
      return 1;
    }
  sleep (config->inter_measure_time);
  char high_data[config->udp_payload_size];
  if (get_high_entropy_data(config, high_data))
    {
      perror ("Unable to open high entropy data");
      return 1;
    }
  if (send_udp_train (udp_client, config, high, high_data))
    {
      perror ("Client failed to send low entropy data");
      return 1;
    }
  if (udp_destroy_client (udp_client))
    {
      perror ("Failed to destroy upd client");
      return 1;
    }
  sleep(5);
  printf("Client pre probe stage successfully finished\n");
  return 0;
}

int get_high_entropy_data (CONFIG config, char data[])
{
  bzero (data, config->udp_payload_size);
  FILE *fd = fopen(random_file, "r");
  if (!fd)
    {
      printf("Failed to open file %s\n", random_file);
      return 1;
    }
  int read_len = config->udp_payload_size - 2;
  if (fread(data + 2, sizeof (char), read_len, fd) < read_len)
    {
      perror ("Unable to read data from high entropy file\n");
      return 1;
    }
  if (fclose (fd))
    {
      perror ("Unable to close file");
      abort ();
    }
  printf("\n %s\n", data);
  return 0;
}

void signal_handler ()
{
  write (STDOUT_FILENO, "Timeout\n", 8);
}

int server_probe (CONFIG config, char *result)
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

  signal (SIGALRM, signal_handler);
  alarm (UPP_TIMEOUT);

  if (recv_udp_train (client_handler, config, low, result))
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
  signal (SIGALRM, signal_handler);
  alarm (20);
  if (recv_udp_train (client_handler, config, high, result + len))
    {
      perror ("Client failed to send high entropy data");
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
  alarm (0);
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
  sleep(1);
  printf("starting to send %s entropy data\n", train_type_str[t]);
  uint16_t packet_id = 0;
  int sent_success = 0;
  int sent_failed = 0;
  for (int i = 0; i < config->udp_packet_train_len; i++, packet_id++)
    {
      printf("sending packet id %d\n", packet_id);
      set_packet_id (buf, packet_id);
      int sent = udp_sendto_n (udp_client->handler, buf, config->udp_payload_size);
      if (sent)
        sent_failed++;
      else
        sent_success++;
    }
  printf("Sent %s entropy data from client with %d success %d failed\n", train_type_str[t], sent_success, sent_failed);
  return 0;
}

int recv_udp_train (UDP_HANDLER client_handler, CONFIG config, enum train_type t, char result[])
{
  printf ("Server starting low entropy receive\n");
  char buf[config->udp_payload_size];
  struct timeb recv_times[config->udp_packet_train_len];
  bzero (recv_times, sizeof (struct timeb) * config->udp_packet_train_len);
  int sent_success = 0;
  int sent_failed = 0;
  for (int i = 0; i < config->udp_packet_train_len; i++)
    {
      printf("trying to receive %s entropy data from train\n", train_type_str[t]);
      int received = udp_recvfrom_n (client_handler, buf, config->udp_payload_size);
      uint16_t packet_id = 0;
      get_packet_id (buf, &packet_id);

      if (received == EINTR)
        {
          printf ("%s entropy timeout\n", train_type_str[t]);
          break;
        }
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
      printf ("start time %d\n", start);
      printf ("end time %d\n", end);
      double ms = (1000 * difftime(recv_times[end].time, recv_times[start].time)) +
        recv_times[end].millitm -  recv_times[start].millitm;
      sprintf (result, "It took %.f ms between packet between packets %d and %d for %s entropy data\n", ms, start, end, train_type_str[t]);
      printf (result);
    }
  else
    {
      printf ("could not get start or end time for %s entropy data\n", train_type_str[t]);
      sprintf (result, "Error getting %s entropy data\n", train_type_str[t]);
      return 1;
    }
  printf ("Sent %s entropy data received from client with %d success %d failed\n", train_type_str[t], sent_success, sent_failed);
  return 0;
}

int
server_post_probe (CONFIG config, char *result)
{
  printf("starting server post-probe stage\n");
  TCP_SERVER server = tcp_new_server (config->tcp_probing_port);
  if (tcp_server_start (server))
    {
      perror ("Unable to start server in pre probe");
      abort ();
    }
  printf("TCP server started in server post-probe stage\n");
  TCP_HANDLER client_handler = tcp_server_next_connection (server);
  if (tcp_send (client_handler, result, strlen (result)))
    {
      perror ("Server failed to send post probe data");
      return 1;
    }
  if (destroy_tcp_sever (server) || destroy_tcp_handler (client_handler))
    {
      perror ("Server failed to close tcp conn in post-probe");
      abort ();
    }
}

int
client_post_probe (CONFIG config)
{
  TCP_CLIENT_CONN client_conn = tcp_new_client (config->server_ip, config->tcp_probing_port);
  if (tcp_client_connect(client_conn))
    {
      perror ("Client Failed to connect to server in post probe");
      abort ();
    }
  int buf_len = 0;
  char buf[MAX_TCP_SIZE];
  int received = tcp_recv (client_conn->handler, buf, &buf_len);
  if (received)
    {
      perror ("Client failed to get results from server in post probe");
      abort ();
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
      return 1;
    }
  printf("Client set up udp client upd connection\n");

  // TODO close socket
  struct sockaddr_in sin;
  memset (&sin, 0, sizeof (sin));
  // TODO update to use inet_pton or check if inet_addr == -1
  sin.sin_addr.s_addr = inet_addr (config->client_ip);
  sin.sin_port = htons (config->tcp_src_syn_port);
  sin.sin_family = AF_INET;

  struct sockaddr_in head_sockaddr_in;
  memset (&head_sockaddr_in, 0, sizeof (head_sockaddr_in));
  // TODO update to use inet_pton or check if inet_addr == -1
  head_sockaddr_in.sin_addr.s_addr = inet_addr (config->server_ip);
  head_sockaddr_in.sin_port = htons (config->tcp_dest_head_syn_port);
  head_sockaddr_in.sin_family = AF_INET;

  struct sockaddr_in tail_sockaddr_in;
  memset (&tail_sockaddr_in, 0, sizeof (tail_sockaddr_in));
  // TODO update to use inet_pton or check if inet_addr == -1
  tail_sockaddr_in.sin_addr.s_addr = inet_addr (config->server_ip);
  tail_sockaddr_in.sin_port = htons (config->tcp_dest_tail_syn_port);
  tail_sockaddr_in.sin_family = AF_INET;

  int sockfd = -1;

  // TODO move device name to config file
  char *device = "enp1s0";
  if ( create_raw_socket (&sockfd, device))
    {
      perror ("Unable to create raw socket");
      return 1;
    }
  pthread_t rst_listener_thread;
  int count = 0;

  struct rst_listener_args *args = malloc (sizeof (struct rst_listener_args));
  if (args == NULL)
    {
      perror ("Unable to malloc rst listener args");
      return 1;
    }
  struct timeb *recv_times = malloc (sizeof (struct timeb) * RST_PACKET_TOTAL);
  if (recv_times == NULL)
    {
      perror ("Unable to create recv_times array");
      free (args);
      return 1;
    }
  args->config = config;
  args->recv_times = recv_times;
  args->sockfd = &sockfd;
  args->count = &count;
  args->sin = &sin;
  args->head_sockaddr_in = &head_sockaddr_in;
  if (start_rst_listener (&rst_listener_thread, args))
    {
      perror("Failed to set up thread for receiving RST packets");
      free (args);
      free (recv_times);
      return 1;
    }

  printf ("raw_packet_size: %d\n", config->raw_packet_size);
  if (send_head_tcp_syn (config, sockfd, &sin, &head_sockaddr_in))
    {
      perror ("Failed to send_head_tcp_syn packet");
      free (args);
      free (recv_times);
      return 1;
    }
  char buf[config->udp_payload_size];
  bzero (buf, config->udp_payload_size);
  int send_low = send_udp_train (udp_client, config, low, buf);
  if (send_low)
    {
      perror ("Client failed to send low entropy data");
      return 1;
    }
  if (send_head_tcp_syn (config, sockfd, &sin, &tail_sockaddr_in))
    {
      perror ("Failed to send_head_tcp_syn packet");
      free (args);
      free (recv_times);
      return 1;
    }
  sleep (config->inter_measure_time);
  if (send_head_tcp_syn (config, sockfd, &sin, &head_sockaddr_in))
    {
      perror ("Failed to send_head_tcp_syn packet");
      free (args);
      free (recv_times);
      return 1;
    }
  char high_data[config->udp_payload_size];
  if (get_high_entropy_data(config, high_data))
    {
      perror ("Unable to open high entropy data");
      return 1;
    }
  if (send_udp_train (udp_client, config, high, high_data))
    {
      perror ("Client failed to send low entropy data");
      return 1;
    }
  if (send_head_tcp_syn (config, sockfd, &sin, &tail_sockaddr_in))
    {
      perror ("Failed to send_head_tcp_syn packet");
      free (args);
      free (recv_times);
      return 1;
    }
  if (udp_destroy_client (udp_client))
    {
      perror ("Failed to destroy upd client");
      return 1;
    }

  printf ("Trying to join thread\n");
  pthread_join(rst_listener_thread, NULL);
  printf ("joined thread\n");
  free (args);
  free (recv_times);
  return 0;
}


void
recv_rst (void *inputs)
{
  struct rst_listener_args *args = (struct rst_listener_args*) inputs;
  char buf[args->config->raw_packet_size];
  ssize_t received;
  uint16_t tcp_dest_head_syn_port = htons (args->config->tcp_dest_head_syn_port);
  uint16_t tcp_dest_tail_syn_port = htons (args->config->tcp_dest_tail_syn_port);
  while (*args->count < RST_PACKET_TOTAL)
    {
      received = recvfrom (*args->sockfd, buf, args->config->raw_packet_size, 0, NULL, NULL);
      if (received == 0)
        break;
      else if (received < 0)
        continue;
      struct iphdr *ip = (struct iphdr *) buf;
      struct tcphdr *tcp = (struct tcphdr *) (buf + (ip->ihl * 4));
      if (tcp->dest == args->sin->sin_port && tcp->source == tcp_dest_head_syn_port && tcp->rst)
        {
          printf ("Head found!!\n");
          char addr0[INET_ADDRSTRLEN];
          inet_ntop (AF_INET, &ip->saddr, addr0, INET_ADDRSTRLEN);
          printf ("Source addr for port found is %s\n", addr0);
          struct timeb recv_time;
          ftime(&recv_time);
          *(args->recv_times + *args->count) = recv_time;
          *args->count += 1;
        }
      else if (tcp->dest == args->sin->sin_port && tcp->source == tcp_dest_tail_syn_port && tcp->rst)
        {
          printf ("received tail\n");
          char addr0[INET_ADDRSTRLEN];
          inet_ntop (AF_INET, &ip->saddr, addr0, INET_ADDRSTRLEN);
          printf ("Source addr for port found is %s\n", addr0);
          struct timeb recv_time;
          ftime(&recv_time);
          *(args->recv_times + *args->count) = recv_time;
          *args->count += 1;
        }
    }
  for (int i=0; i<4; i++)
    {
      struct timeb current_time = *(args->recv_times + i);
      if (current_time.millitm != 0)
        printf ("Index %d is mili at %d\n", i, current_time.millitm);
      else
        printf ("Index %d has no data\n", i);
    }
  printf("\n");
  char addr[INET_ADDRSTRLEN];
  inet_ntop (AF_INET, &args->head_sockaddr_in->sin_addr.s_addr, addr, INET_ADDRSTRLEN);
  printf ("Server ip from head_sockaddr_in %s\n", addr);
}

int
start_rst_listener (pthread_t *rst_listener_thread, struct rst_listener_args *args)
{
  printf ("Starting listener\n");

  if (pthread_create (rst_listener_thread, NULL, (void *) &recv_rst, (void *) args))
    {
      perror ("Error creating RST recv thread");
      return 1;
    }

  return 0;
}

int
send_head_tcp_syn (CONFIG config, int sockfd, struct sockaddr_in *sin, struct sockaddr_in *sout)
{
  char *packet = malloc (sizeof (char) * config->raw_packet_size);
  if (packet == NULL)
    {
      perror ("Error allocating packet with malloc");
      return 1;
    }

  printf ("head_sockaddr_in addr %d\n", sout->sin_addr.s_addr);

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
  printf ("Sent SYN packet to server at %s on port %d\n", config->server_ip, config->tcp_dest_head_syn_port);
  return 0;
}

// pseudo header form TCP rfc and from https://github.com/MaxXor/raw-sockets-example/blob/6bf7f8bb550ccbe9e3b29d2cc632c9b91197fdd6/rawsockets.c#L12
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
  ip->tos = 0;
  ip->ttl = 32;
  ip->tot_len = sizeof (struct tcphdr) + sizeof (struct iphdr);
  ip->id = htonl (id);
  ip->frag_off = 0;
  ip->check = 0;
  ip->protocol = IPPROTO_TCP;

  ip->daddr = sout->sin_addr.s_addr;
  ip->saddr = sin->sin_addr.s_addr;

  tcp->source = sin->sin_port;
  tcp->dest = sout->sin_port;
  tcp->seq = htonl(55);
  tcp->ack_seq = htonl (0);
  tcp->syn = 1;
  tcp->cwr = 0;
  tcp->urg = 0;
  tcp->ack = 0;
  tcp->psh = 0;
  tcp->rst = 0;
  tcp->fin = 0;
  tcp->window = htons (64240);
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
  tcp->check = checksum2 ((const char *) pseudo_packet, pseudo_size);
  ip->check = checksum2 (packet, ip->tot_len);
  printf("checksum2 %d\n", ip->check);
  printf("tcp checksum %d\n", tcp->check);
  free (pseudo_packet);
  return 0;
}

// This function is from https://github.com/MaxXor/raw-sockets-example/blob/6bf7f8bb550ccbe9e3b29d2cc632c9b91197fdd6/rawsockets.c#L24
unsigned short
checksum2(const char *buf, unsigned size)
{
  unsigned sum = 0, i;

  /* Accumulate checksum2 */
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
  printf ("Sent raw socket %d \n", sent);
  return 0;
}

int
create_raw_socket (int *sockfd, char *interface)
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
      perror ("Failed to set socket opt for raw socket");
      return 1;
    }

  struct ifreq ifr;
  memset (&ifr, 0, sizeof (struct ifreq));

  // TODO check if need error handling around strcpy
  strcpy (ifr.ifr_ifrn.ifrn_name, interface);
  if (ioctl (*sockfd, SIOCGIFFLAGS, &ifr) == -1)
    {
      perror ("Unable to get flags for network interface");
      return 1;
    }
  ifr.ifr_ifru.ifru_flags |= IFF_PROMISC;
  if (ioctl (*sockfd, SIOCGIFFLAGS, &ifr) == -1)
    {
      perror ("Unable to set network interface to promiscuous mode");
      return 1;
    }
  return 0;
}