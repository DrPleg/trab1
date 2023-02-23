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
#include <curses.h>
#include <string.h>
#include <sys/stat.h>


#define ESC 27

#include"msgFormat.h"
// Para rodar, instale a biblioteca ncurses, e compile com a flag: -lcurses
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


typedef struct packet {
    int num_seq;
    char data[MAXBUFLEN];
} packet;

int ULTIMO_PACKET_RECEBIDO = -1;

void concatenar_letra(char texto[], char letra){
    size_t tamanho = strlen(texto);
    texto[tamanho] = letra;
    texto[tamanho + 1] = '\0'; //recolocar terminador
}

void remover_letra(char texto[]) {
    size_t tamanho = strlen(texto);
    texto[tamanho - 2] = '\0';
}

void remover_letra_send(char texto[]) {
    size_t tamanho = strlen(texto);
    texto[tamanho - 1] = '\0';
}

int enviar_arquivo(char *string) {
    struct stat file_info;
    if (stat(string, &file_info) != 0) {
        printw("ERRO 205\n");
        return 205;
    }

    if ((!file_info.st_mode & S_IRUSR)) {
        printw("ERRO 206\n");
        return 1;
    }
    
    printw("podemos enviar\n");
    //TODO FUNÇÃO DE ENVIAR ARQUIVO
    return 1;
}


void modo_edicao(char *str) {
    printw("Digite sua mensagem, e ao terminar digite <Enter>, ou, para cancelar pressione <ESC>\n\n");
    echo();

    str = NULL;
    int ch;
    size_t size=0,len=0;
    while ((ch=getch()) != EOF && ch != '\n' && ch != ESC) {
        
        if (ch != 127) {
            ++len;
            str=realloc(str,sizeof(char)*len);
            str[len-1]=ch;
        }
        else {
            //backspace
            clear();
            printw("%s", str); 
        }
    }
    str[len] = '\0';

    if (ch == '\n') {
        printw("\n\nenviamos a seguinte mensagem = %s\n", str);
    }
    refresh();

    return;
}


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

    char he[100];

    char c, ultimo_char;
    char arquivo[100];
    int seguindo_sequencia = 0; //1 se estiver seguindo a sequência :q<enter>
    int enviar_mensagem = 0; //1 se ele estiver no modo talker
    int num_seq_esperado = 0;

    // creates the UDP socket
    if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    initscr();     // inicializa a tela do curses
    cbreak();      // habilita o modo de leitura não-bloqueante de entrada de teclado
    echo();        // habilita o modo de eco de entrada de teclado
    printw("Digite i para entrar no modo de envio de mensagens, :q<enter> para sair do programa e :send <arquivo> <enter> para enviar um arquivo\n");
    char *str = NULL;


    do {
        enviar_mensagem = 0;
        c = getch();

        if (c == 105) {
            str = NULL;
            modo_edicao(str);
            enviar_mensagem = 1;
        }

        if (c == ':') {
            seguindo_sequencia = 1;
        }else if (c == 's') {
            if (ultimo_char == ':' && seguindo_sequencia == 1) {
                //:s
                seguindo_sequencia = 1;
            } else {
                seguindo_sequencia = 0;
            }
        } else if (c == 'e') {
            if (ultimo_char == 's' && seguindo_sequencia == 1) {
                //:se
                seguindo_sequencia = 1;
            } else {
                seguindo_sequencia = 0;
            }
        } else if (c == 'n') {
            if (ultimo_char == 'e' && seguindo_sequencia == 1) {
                //:sen
                seguindo_sequencia = 1;
            } else {
                seguindo_sequencia = 0;
            }
        } else if (c == 'd' && seguindo_sequencia == 1) {
            if (ultimo_char == 'n' && seguindo_sequencia == 1) {
            //:send a proxima string depois do espaco sera o nome do arquivo
            while (c != '\n') {
                c = getch();
                if (c == 127) {
                    remover_letra_send(arquivo);
                    clear();
                    printw(":send %s", arquivo);       
                } else if (c != ' ' && c != '\n') {
                    concatenar_letra(arquivo, c);
                }
            }
            //printw("Arquivo a ser enviado:%s\n", arquivo);
            enviar_arquivo(arquivo);
            *arquivo = 0;
                seguindo_sequencia = 1;
            } else {
                seguindo_sequencia = 0;
            }
        } else if (c == 'q') {
            if (ultimo_char == ':' && seguindo_sequencia == 1) {
                //printw("VocÊ digitou :q\n");
                seguindo_sequencia = 1;
            } else {
                seguindo_sequencia = 0;
            }
        } else if (c == '\n') {
            if (ultimo_char == 'q' && seguindo_sequencia == 1) {
                //saindo do programa
                break;
            } else {
                seguindo_sequencia = 0;
            }
        } else {
            seguindo_sequencia = 0;
        }

        ultimo_char = c;


        //1 = Mandar/Talker
        if (enviar_mensagem){

            printw("Digite o host\n");
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
            //printw("Digite a mensagem\n");
            //funcao pra pegar mensagem "infinita", fazer funcao
            //precisa de referencia como input
            //precisamos cortar mmensagem em pedacos dps
            /*
            char* str = NULL;
            int ch;
            size_t size=0,len=0;
            while ((ch=getchar()) != EOF && ch != '\n') {
            /    if (len + 1 >= size){
                    //printf("heah\n");
                    size = size * 2 + 1;
                    str = realloc(str, sizeof(char)*size);
                }
            str[len++] = ch;
            /
                ++len;
                str=realloc(str,sizeof(char)*len);
                str[len-1]=ch;
            }
            */
        //if (str != NULL) {
            //str[len] = '\0';
        //    printf("%s\n", str);
            //free(str);
        //}
        //vai ate aqui com possivel free (precisamos de free, porem tb temos q passar pra msg.name)
        printw("Success\n");
            strcpy(msg.name,str);
            printw("%s\n",str);
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

            int num_packets = 1; //TODO calcular o número de pacotes a serem enviados
            int num_packets_send = 0;

            while (num_packets_send < num_packets) {
                // Espera um pacote de dados do remetente
                recv(sockfd, buf, MAXBUFLEN, 0);
                struct packet *incoming_pkt = (struct packet *) buf;

                // Verifica o número de sequência
                if (incoming_pkt->num_seq == num_seq_esperado) {
                    // Pacote correto, envia um ACK
                    sendto(sockfd, &num_seq_esperado, sizeof(int), 0, (struct sockaddr *) &rcvr_addr, sizeof(rcvr_addr));
                    num_seq_esperado++;
                    num_packets_send++;
                } else {
                    // Pacote incorreto, envia um NACK com o número de sequência esperado
                    int nack = num_seq_esperado;
                    sendto(sockfd, &nack, sizeof(int), 0, (struct sockaddr *) &rcvr_addr, sizeof(rcvr_addr));
                }

                
                printw ("sent %d bytes to host %s port %d\n", numbytes,
                inet_ntoa (rcvr_addr.sin_addr), RCVR_PORT);
                }
        } else{ //Else ou 2 = Receber
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
            printw ("Listening for packets at port %d\n", RCVR_PORT) ;
            /*numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
                                (struct sockaddr *) &sndr_addr, &addr_len) ;*/
            numbytes = recvfrom(sockfd, buf, MAXBUFLEN, 0 , (struct sockaddr *) &sndr_addr, &addr_len);
            packet *packet_rcvd = (struct packet *) buf;

            printw("lol\n");
            //printf("%s",msgrcv.data);
            if (numbytes == -1){
                perror ("recvfrom");
                exit (1);
            }
            printw("lol\n");
            // prints packet info and contents
            printw ("Got packet from host %s:%d\n",
                    inet_ntoa (sndr_addr.sin_addr), ntohs(sndr_addr.sin_port));
            //buf[numbytes] = '\0';
            //printf ("Packet is %d bytes long and contains \"%s\"\n", numbytes, buf);
            //printw ("Packet is %d bytes long and contains \"%s\"\n", numbytes, msgrcv.name);
            if (packet_rcvd->num_seq == ULTIMO_PACKET_RECEBIDO+1) {
                ULTIMO_PACKET_RECEBIDO++;
                // Pacote correto, envia um ACK com o número de sequência correspondente
                sendto(sockfd, &ULTIMO_PACKET_RECEBIDO, sizeof(int), 0, (struct sockaddr *) &sndr_addr, sizeof(sndr_addr));

                // Imprime o conteúdo do pacote
                printf("Received: %s\n", packet_rcvd->data);
            } else {
                // Pacote incorreto, envia um NACK com o número de sequência esperado
                sendto(sockfd, &ULTIMO_PACKET_RECEBIDO+1, sizeof(int), 0, (struct sockaddr *) &sndr_addr, sizeof(sndr_addr));
            }
        
        }
        
    
    } while(1);
    
    //fecha socket
    close (sockfd);
 
    return 0;
}