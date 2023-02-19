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

#define RCVR_PORT 4950

// input buffer size
#define MAXBUFLEN 100

int main(int argc, char *argv[]){
    int sockfd;
    struct sockaddr_in rcvr_addr;
    struct sockaddr_in sndr_addr;
    struct hostent *he;
    int numbytes;
    socklen_t addr_len;
    char buf[MAXBUFLEN];

    if (argc != 3)
    {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }

    he = gethostbyname(argv[1]) ;
    if (he == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
    
    rcvr_addr.sin_family = AF_INET;
    rcvr_addr.sin_port   = htons(RCVR_PORT);
    rcvr_addr.sin_addr   = *((struct in_addr *)he->h_addr);
    // zeroes the rest of the struct
    memset(&(rcvr_addr.sin_zero), '\0', 8);

    // creates the UDP socket
    if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    int x;
    scanf("%d\n",&x);

    //1 = Mandar/Talker
    if (x==1){
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

    
    }
    else{ //Else ou 2 = Receber
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
 

    }
    
    //fecha socket
    close (sockfd);
 
    return 0;
}