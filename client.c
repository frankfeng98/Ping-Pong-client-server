#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

/* simple client, takes two parameters, the server domain name,
   and the server port number */

int main(int argc, char** argv) {
  /* our client socket */
  int sock;

  /* variables for identifying the server */
  unsigned int server_addr;
  struct sockaddr_in sin;
  struct addrinfo *getaddrinfo_result, hints;
  struct timeval begin, now;

  if (argc != 5) {
		fprintf(stderr, "usage: %s <host> <port> <size> <count>\n", argv[0]);
		exit(1);
	}

  /* convert server domain name to IP address */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; /* indicates we want IPv4 */

  if (getaddrinfo(argv[1], NULL, &hints, &getaddrinfo_result) == 0) {
    server_addr = (unsigned int) ((struct sockaddr_in *) (getaddrinfo_result->ai_addr))->sin_addr.s_addr;
    freeaddrinfo(getaddrinfo_result);
  }

  /* server port number */
  unsigned short server_port = atoi (argv[2]);

  char *buffer, *sendbuffer;
  int size = atoi(argv[3]);
  int count = atoi(argv[4]);
  int total_received = 0; 
  int bytes_received = 0;
  int total_sent = 0;
  int bytes_sent = 0;
  int current_round = 0;
  double total_elapsed_time = 0.0;
  double average_elapsed_time = 0.0;

  /* Error checking for argument */
  if (server_port < 18000 || server_port > 18200) {
    perror("Server port needs to between 18000 and 18200");
    abort();
  } 

  if (size < 18 || size > 65535) {
    perror("Size needs to stay between 18 and 65535");
    abort();
  }

  if (count < 1 || count > 10000) {
    perror("Count needs to stay between 1 and 10000");
    abort();
  }

  /* allocate a memory buffer in the heap */
  /* putting a buffer on the stack like:
         char buffer[500];
     leaves the potential for
     buffer overflow vulnerability */
  buffer = (char *) malloc(size);
  if (!buffer)
    {
      perror("failed to allocated buffer");
      abort();
    }

  sendbuffer = (char *) malloc(size);
  if (!sendbuffer)
    {
      perror("failed to allocated sendbuffer");
      abort();
    }


  /* create a socket */
  if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
      perror ("opening TCP socket");
      abort ();
    }

  /* fill in the server's address */
  memset (&sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = server_addr;
  sin.sin_port = htons(server_port);

  /* connect to the server */
  if (connect(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror("connect to server failed");
      abort();
    }

  /* Use a while loop to keep track of the count time*/
  while (current_round < count) {
    memset(sendbuffer, 0, size);
    /* Format the packet to be sent (make sure to include the time) */
    *(unsigned short *) (sendbuffer) = (unsigned short) htons(size);
    gettimeofday(&begin, NULL);
    *(unsigned int *) (sendbuffer+2) = (unsigned int) htonl(begin.tv_sec);
    *(unsigned int *) (sendbuffer+10) = (unsigned int) htonl(begin.tv_usec);

    /* Send the packet, make sure that the entire packet is sent */
    while (total_sent < size) {
      bytes_sent = send(sock, sendbuffer + total_sent, size - total_sent, 0);
      if (bytes_sent < 0) {
        perror("send failure");
        abort();
      }
      total_sent += bytes_sent;
    }

    /* Wait to receive the response (make sure that entire packet is received)*/
    while (total_received < size) {
      bytes_received = recv(sock, buffer + total_received, size - total_received, 0);
      if (bytes_received < 0) {
        perror("receive failure");
        abort(); 
      }
      total_received += bytes_received;
    }

    /* Parse the receive packet to view time */
    unsigned int tv_sec = (unsigned int) ntohl(* (unsigned int *)(buffer+2));
    unsigned int tv_usec = (unsigned int) ntohl(* (unsigned int *)(buffer+10));
    /*printf("Buffer size is %d\n", (unsigned short) ntohs(* (unsigned short *)(buffer)));
    printf("tv_sec %u\n", tv_sec);
    printf("tv_usec %u\n", tv_usec);*/

    /* Calculate the time used and add it to overall time */
    gettimeofday(&now, NULL);
    /*printf("tv_sec now %d\n", now.tv_sec);
    printf("tv_usec now %d\n", now.tv_usec);*/
    total_elapsed_time += 1e3 * (now.tv_sec - tv_sec) + 1e-3 * (now.tv_usec - tv_usec);

    /* Increment the count and clear the variable */
    current_round += 1;
    total_sent = 0;
    total_received = 0;
  }

  /* Once the loop is exitted, print the average time and close the connection*/
  average_elapsed_time = total_elapsed_time / count;
  printf("%.3f\n", average_elapsed_time);

  close(sock);
  free(buffer);
  free(sendbuffer);
  return 0;
}