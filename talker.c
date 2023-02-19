#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
 
// receiver listening port
#define RCVR_PORT 4950

//https://superuser.com/questions/1586386/how-to-find-wsl2-machines-ip-address-from-windows
//http://www.steves-internet-guide.com/tcpip-ports-sockets/
//https://tutorialspoint.dev/computer-science/computer-network-tutorials/c-program-display-hostname-ip-address

//check wsl hostname -I
//sudo ./talker ip message
int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in rcvr_addr;
    struct hostent *he;
    int numbytes;
 
    if (argc != 3)
    {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }
 
    // translates receiver name to address
    he = gethostbyname(argv[1]) ;
    if (he == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
    
    ///localhost:8081/

    // fills the receiver struct info
    rcvr_addr.sin_family = AF_INET;
    rcvr_addr.sin_port   = htons(RCVR_PORT);
    rcvr_addr.sin_addr   = *((struct in_addr *)he->h_addr);
 
    // zeroes the rest of the struct
    memset(&(rcvr_addr.sin_zero), '\0', 8);
 
    // creates the UDP socket
    sockfd = socket (AF_INET, SOCK_DGRAM, 0) ;
    if (sockfd == -1)
    {
        perror("socket");
        exit(1);
    }
 
    // sends the message to the receiver using the socket
    numbytes = sendto (sockfd, argv[2], strlen(argv[2]), 0,
                       (struct sockaddr *) &rcvr_addr,
                       sizeof(struct sockaddr)) ;
    if (numbytes == -1)
    {
        perror("sendto");
        exit(1);
    }
 
    printf ("sent %d bytes to host %s port %d\n", numbytes,
            inet_ntoa (rcvr_addr.sin_addr), RCVR_PORT);
 
    // closes the socket
    close (sockfd);
 
    return 0;
}
