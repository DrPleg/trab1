#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
// the port I'll listen to
#define RCVR_PORT 4950
 
// input buffer size
#define MAXBUFLEN 100
 
int main(void)
{
    int sockfd;
    struct sockaddr_in rcvr_addr;
    struct sockaddr_in sndr_addr;
    int numbytes;
    socklen_t addr_len;
    char buf[MAXBUFLEN];
 
    // creates the UDP socket
    if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }
 
    // fills struct with receiver's info
    rcvr_addr.sin_family = AF_INET;
    rcvr_addr.sin_port = htons(RCVR_PORT);   // short, network byte order
    rcvr_addr.sin_addr.s_addr = INADDR_ANY;  // automatically fill with my IP
    memset(&(rcvr_addr.sin_zero), '\0', 8);  // zero the rest of the struct
 
    // associates the socket to receiver's data (addr & port)
    if (bind (sockfd, (struct sockaddr *) &rcvr_addr,
              sizeof (struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }
 
    // waits for a packet arriving to the socket
    addr_len = sizeof(struct sockaddr);
    printf ("Listening for packets at port %d\n", RCVR_PORT) ;
    numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
                        (struct sockaddr *) &sndr_addr, &addr_len) ;
    if (numbytes == -1)
    {
        perror ("recvfrom");
        exit (1);
    }
 
    // prints packet info and contents
    printf ("Got packet from host %s:%d\n",
            inet_ntoa (sndr_addr.sin_addr), ntohs(sndr_addr.sin_port));
    buf[numbytes] = '\0';
    printf ("Packet is %d bytes long and contains \"%s\"\n", numbytes, buf);
 
    // closes the socket
    close (sockfd);
 
    return 0;
}