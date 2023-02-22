#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include"msgFormat.h"

//https://www.google.com/search?q=C+sending+pointer+over+socket&sxsrf=AJOqlzXV-W4zsd6bZpoC4FHcg6X6E4mDjA%3A1676939748169&ei=5BH0Y-mGCpuL5OUPopq06Ag&ved=0ahUKEwjpiP-vr6X9AhWbBbkGHSINDY0Q4dUDCA8&uact=5&oq=C+sending+pointer+over+socket&gs_lcp=Cgxnd3Mtd2l6LXNlcnAQAzIGCAAQFhAeMgUIABCGAzIFCAAQhgM6CggAEEcQ1gQQsAM6BAgjECc6BAgAEEM6BQgAEJECOgUIABCABDoLCC4QgAQQxwEQ0QM6BwgAEIAEEAo6CggAEIAEEBQQhwI6CQgAEBYQHhDxBDoNCAAQFhAeEA8Q8QQQCjoFCCEQoAE6CwghEBYQHhDxBBAdOggIIRAWEB4QHToECCEQFToKCCEQFhAeEA8QHUoECEEYAFCMBliDI2CzI2gDcAF4AIABsgGIAZIbkgEEMC4yOJgBAKABAcgBCMABAQ&sclient=gws-wiz-serp

#define RCVR_PORT 4950

// input buffer size
#define MAXBUFLEN 100

char* readinput()
{
#define CHUNK 200
   char* input = NULL;
   char tempbuf[CHUNK];
   size_t inputlen = 0, templen = 0;
   do {
       fgets(tempbuf, CHUNK, stdin);
       templen = strlen(tempbuf);
       input = realloc(input, inputlen+templen+1);
       strcpy(input+inputlen, tempbuf);
       inputlen += templen;
    } while (templen==CHUNK-1 && tempbuf[CHUNK-2]!='\n');
    return input;
}

typedef struct animal{
    int age;
    //char* name; //n faz sentido mandar um ponteiro/endereco para socket, eh do servidor, pq eu mandaria?
    char name[45];
} animal;

int main(){ // int argc, char *argv[] // get host at least?
    //setlocale(LC_ALL,""); //Os dois usuários devem ser capazes de fazer envios de mensagem de texto com todos os caracteres disponíveis na codificação UTF-8. Isso inclui por exemplo letras acentuadas e emojis.
    int sockfd;
    struct sockaddr_in rcvr_addr;
    struct sockaddr_in sndr_addr;
    struct hostent *hostname;
    animal msg;
    animal msgrcv;
    int numbytes;
    socklen_t addr_len;
    char buf[MAXBUFLEN];

    /*if (argc != 3)
    {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }
    */

    int x;
    printf("Falar(1) ou Receber(Any)?\n");
    scanf("%d",&x);

    char he[100];

    // creates the UDP socket
    if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    //1 = Mandar/Talker
    if (x==1){

        printf("Digite o host\n");
        scanf("%s\n",he);

        hostname = gethostbyname(he) ;
        if (hostname == NULL){
            perror("gethostbyname");
            exit(1);
        }
    
        rcvr_addr.sin_family = AF_INET;
        rcvr_addr.sin_port   = htons(RCVR_PORT);
        rcvr_addr.sin_addr   = *((struct in_addr *)hostname->h_addr);
        // zeroes the rest of the struct
        memset(&(rcvr_addr.sin_zero), '\0', 8);

        //msg=malloc(sizeof(message));

        msg.age = 15;
        //msg.name=malloc(6);
        //msg.name="Lucas";
        //"Lucas";
         //strcpy(msg.name,"Lucas suca");//{"L","u","c","a","s","\0"};
         //printf("%s\n",msg.name);

        /*
        msg.preamble = 126;
        msg.type = 1;
        msg.sequence = 0;
        msg.size=15;
        msg.data="mensagem";
        msg.crc8=10;
        */

        //printf("%s",msg.data);
        printf("Digite a mensagem\n");
        //funcao pra pegar mensagem "infinita", fazer funcao
        //precisa de referencia como input
        //precisamos cortar mmensagem em pedacos dps
        char* str = NULL;
        int ch;
        size_t size=0,len=0;
        while ((ch=getchar()) != EOF && ch != '\n') {
        /*    if (len + 1 >= size){
                //printf("heah\n");
                size = size * 2 + 1;
                str = realloc(str, sizeof(char)*size);
            }
        str[len++] = ch;
        */
            ++len;
            str=realloc(str,sizeof(char)*len);
            str[len-1]=ch;
        }
    //if (str != NULL) {
        str[len] = '\0';
    //    printf("%s\n", str);
        //free(str);
    //}
    //vai ate aqui com possivel free (precisamos de free, porem tb temos q passar pra msg.name)
    printf("Success\n");
        strcpy(msg.name,str);
        printf("%s\n",str);
        //strcpy(msg.name,inputString(stdin,10));
        //scanf("%s",he);
            // sends the message to the receiver using the socket
        /*numbytes = sendto (sockfd, argv[2], strlen(argv[2]), 0,
                       (struct sockaddr *) &rcvr_addr,
                       sizeof(struct sockaddr)) ;
                       */               //sizeof(animal)-sizeof(char*)
        numbytes = sendto (sockfd, &msg, sizeof(animal),0,(struct sockaddr *) &rcvr_addr,
                       sizeof(struct sockaddr));
        if (numbytes == -1){
            perror("sendto");
            exit(1);
        }
 
        printf ("sent %d bytes to host %s port %d\n", numbytes,
                inet_ntoa (rcvr_addr.sin_addr), RCVR_PORT);
    }
    else{ //Else ou 2 = Receber
        rcvr_addr.sin_family = AF_INET;
        rcvr_addr.sin_port = htons(RCVR_PORT);   // short, network byte order
        rcvr_addr.sin_addr.s_addr = INADDR_ANY;  // automatically fill with my IP
        memset(&(rcvr_addr.sin_zero), '\0', 8);  // zero the rest of the struct

        /*msg.preamble = 126;
        msg.type = 1;
        msg.sequence = 0;
        msg.size=15;
        msg.data="mensagem";
        msg.crc8=10;*/
    
        // associates the socket to receiver's data (addr & port)
        if (bind (sockfd, (struct sockaddr *) &rcvr_addr,
                sizeof (struct sockaddr)) == -1){
            perror("bind");
            exit(1);
        }
 
        // waits for a packet arriving to the socket
        addr_len = sizeof(struct sockaddr);
        printf ("Listening for packets at port %d\n", RCVR_PORT) ;
        /*numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
                            (struct sockaddr *) &sndr_addr, &addr_len) ;*/
        numbytes = recvfrom(sockfd, &msgrcv, sizeof(animal), 0 , (struct sockaddr *) &sndr_addr, &addr_len);
        printf("lol\n");
        //printf("%s",msgrcv.data);
        if (numbytes == -1){
            perror ("recvfrom");
            exit (1);
        }
        printf("lol\n");
        // prints packet info and contents
        printf ("Got packet from host %s:%d\n",
                inet_ntoa (sndr_addr.sin_addr), ntohs(sndr_addr.sin_port));
        //buf[numbytes] = '\0';
        //printf ("Packet is %d bytes long and contains \"%s\"\n", numbytes, buf);
        printf ("Packet is %d bytes long and contains \"%s\"\n", numbytes, msgrcv.name);
    }
    
    //fecha socket
    close (sockfd);
 
    return 0;
}