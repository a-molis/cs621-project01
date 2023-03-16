#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <linux/tcp.h>

// pseudo header needed for tcp header checksum calculation
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

#define DATAGRAM_LEN 4096
#define OPT_SIZE 20

unsigned short checksum(const char *buf, unsigned size)
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

void create_syn_packet(struct sockaddr_in* src, struct sockaddr_in* dst, char** out_packet, int* out_packet_len)
{
  // datagram to represent the packet
  char *datagram = calloc(DATAGRAM_LEN, sizeof(char));

  // required structs for IP and TCP header
  struct iphdr *iph = (struct iphdr*)datagram;
  struct tcphdr *tcph = (struct tcphdr*)(datagram + sizeof(struct iphdr));
  struct pseudo_header psh;

  // IP header configuration
  iph->ihl = 5;
  iph->version = 4;
  iph->tos = 0;
  iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
  iph->id = htonl(rand() % 65535); // id of this packet
  iph->frag_off = 0;
  iph->ttl = 64;
  iph->protocol = IPPROTO_TCP;
  iph->check = 0; // correct calculation follows later
  iph->saddr = src->sin_addr.s_addr;
  iph->daddr = dst->sin_addr.s_addr;

  // TCP header configuration
  tcph->source = src->sin_port;
  tcph->dest = dst->sin_port;
  tcph->seq = htonl(rand() % 4294967295);
  tcph->ack_seq = htonl(0);
  tcph->doff = 5; // tcp header size
  tcph->fin = 0;
  tcph->syn = 1;
  tcph->rst = 0;
  tcph->psh = 0;
  tcph->ack = 0;
  tcph->urg = 0;
  tcph->check = 0; // correct calculation follows later
  tcph->window = htons(5840); // window size
  tcph->urg_ptr = 0;

  // TCP pseudo header for checksum calculation
  psh.source_address = src->sin_addr.s_addr;
  psh.dest_address = dst->sin_addr.s_addr;
  psh.placeholder = 0;
  psh.protocol = IPPROTO_TCP;
  psh.tcp_length = htons(sizeof(struct tcphdr));
  int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
  // fill pseudo packet
  char* pseudogram = malloc(psize);
  memcpy(pseudogram, (char*)&psh, sizeof(struct pseudo_header));
  memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr));

//  // TCP options are only set in the SYN packet
//  // ---- set mss ----
//  datagram[40] = 0x02;
//  datagram[41] = 0x04;
//  int16_t mss = htons(48); // mss value
//  memcpy(datagram + 42, &mss, sizeof(int16_t));
//  // ---- enable SACK ----
//  datagram[44] = 0x04;
//  datagram[45] = 0x02;
//  // do the same for the pseudo header
//  pseudogram[32] = 0x02;
//  pseudogram[33] = 0x04;
//  memcpy(pseudogram + 34, &mss, sizeof(int16_t));
//  pseudogram[36] = 0x04;
//  pseudogram[37] = 0x02;

  tcph->check = checksum ((const char *) pseudogram, psize);
  iph->check = checksum ((const char *) datagram, iph->tot_len);

  *out_packet = datagram;
  *out_packet_len = iph->tot_len;
  free(pseudogram);
}

int receive_from(int sock, char* buffer, size_t buffer_length, struct sockaddr_in *dst)
{
  unsigned short dst_port;
  int received;
  do
    {
      received = recvfrom(sock, buffer, buffer_length, 0, NULL, NULL);
      if (received < 0)
        break;
      memcpy(&dst_port, buffer + 22, sizeof(dst_port));
    }
  while (dst_port != dst->sin_port);
  printf("received bytes: %d\n", received);
  printf("destination port: %d\n", ntohs(dst->sin_port));
  return received;
}

int main(int argc, char** argv)
{
  if (argc != 4)
    {
      printf("invalid parameters.\n");
      printf("USAGE %s <source-ip> <target-ip> <port>\n", argv[0]);
      return 1;
    }

  srand(time(NULL));

  int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (sock == -1)
    {
      printf("socket creation failed\n");
      return 1;
    }

  // destination IP address configuration
  struct sockaddr_in daddr;
  daddr.sin_family = AF_INET;
  daddr.sin_port = htons(atoi(argv[3]));
  if (inet_pton(AF_INET, argv[2], &daddr.sin_addr) != 1)
    {
      printf("destination IP configuration failed\n");
      return 1;
    }

  // source IP address configuration
  struct sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(rand() % 65535); // random client port
  if (inet_pton(AF_INET, argv[1], &saddr.sin_addr) != 1)
    {
      printf("source IP configuration failed\n");
      return 1;
    }


  printf("selected source port number: %d\n", ntohs(saddr.sin_port));

  // tell the kernel that headers are included in the packet
  int one = 1;
  const int *val = &one;
  if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) == -1)
    {
      printf("setsockopt(IP_HDRINCL, 1) failed\n");
      return 1;
    }

  // send SYN
  char* packet;
  int packet_len;
  create_syn_packet(&saddr, &daddr, &packet, &packet_len);

  int sent;
  if ((sent = sendto(sock, packet, packet_len, 0, (struct sockaddr*)&daddr, sizeof(struct sockaddr))) == -1)
    {
      printf("sendto() failed\n");
    }
  else
    {
      printf("successfully sent %d bytes SYN!\n", sent);
    }

  // receive SYN-ACK
  char recvbuf[DATAGRAM_LEN];
  int received = receive_from(sock, recvbuf, sizeof(recvbuf), &saddr);
  if (received <= 0)
    {
      printf("receive_from() failed\n");
    }
  else
    {
      printf("successfully received %d bytes RST!\n", received);
    }
  close(sock);
  return 0;
}
