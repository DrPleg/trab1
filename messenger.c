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

#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<net/ethernet.h>
#include<linux/if_packet.h>
#include<net/if.h>

#include <errno.h>

#include"crc.h"

#include<pthread.h>

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

void verificaConexao(int sockfd){
    if (errno == EPIPE || errno == ENOTCONN){
        clear();
        printw("Erro de conexao - 202");
        refresh();
        close(sockfd);
        endwin();
        exit(202);//(202);
    }
    if (errno == ECONNRESET){
        clear();
        printw("Conexao Reestabelecida - 208");
        refresh();
    }
}


void modo_edicao(char **str) {
    printw("Digite sua mensagem, e ao terminar digite <Enter>, ou, para cancelar pressione <ESC>\n\n");
    echo();

    *str = NULL;
    int ch;
    size_t size=0,len=0;
    while ((ch=getch()) != EOF && ch != '\n' && ch != ESC) {
        
        if (ch != 127) {
            ++len;
            *str=realloc(*str,sizeof(char)*len);
            (*str)[len-1]=ch;
        }
        else {
            //backspace
            clear();
            printw("%s", *str); 
        }
    }
    if (*str==NULL){
        *str="";
    }
    else{
        (*str)[len] = '\0';
    }
    if (ch == '\n') {
        printw("\n\nenviamos a seguinte mensagem = %s\n", *str);
    }
    //printw("Succes\n");
    //refresh();
    

    return;
}


int main(){ // int argc, char *argv[] // get host at least?
    //setlocale(LC_ALL,""); //Os dois usuários devem ser capazes de fazer envios de mensagem de texto com todos os caracteres disponíveis na codificação UTF-8. Isso inclui por exemplo letras acentuadas e emojis.
    pthread_t thr[2];
    
    int sockfd;
    struct ifreq ir;
    //struct sockaddr_in rcvr_addr;
    //struct sockaddr_in sndr_addr;
    struct hostent *hostname;
    struct sockaddr_ll eth_snd;
    struct sockaddr_ll eth_rcvr;
    message msg;
    message msgrcv;
    int numbytes;
    int connfd;
    socklen_t addr_len;
    char buf[MAXBUFLEN];

    char he[100];

    char c, ultimo_char;
    char arquivo[100];
    int seguindo_sequencia = 0; //1 se estiver seguindo a sequência :q<enter>
    int enviar_mensagem = 0; //1 se ele estiver no modo talker
    int num_seq_esperado = 0;

    // creates the UDP socket
    if ((sockfd = socket (AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL))) == -1) { //SOCK_STREAM
        perror("socket");
        printw("Erro de conexao - 202");
        refresh();
        //break;//(1);
        close(sockfd);
        endwin();
        exit(202);
    }

    memset(&ir, 0, sizeof(struct ifreq)); /*eth0*/
    memcpy(ir.ifr_name, "eth0", sizeof("eth0"));
    if (ioctl(sockfd, SIOCGIFINDEX, &ir)==-1){
        /*perror("ioctl");
        break;//(1);*/
        clear();
        printw("Erro de conexao - 202");
        refresh();
        close(sockfd);
        endwin();
        exit(202);
    }

    eth_snd.sll_family = AF_PACKET;
    eth_snd.sll_protocol = htons(ETH_P_ALL);
    eth_snd.sll_ifindex = ir.ifr_ifindex;

    eth_rcvr.sll_family = AF_PACKET;
    eth_rcvr.sll_protocol = htons(ETH_P_ALL);
    eth_rcvr.sll_ifindex = ir.ifr_ifindex;

    // associates the socket to receiver's data (addr & port)
    if (bind (sockfd, &eth_snd,
            sizeof (eth_snd)) == -1){
        perror("bind");
        //break;//(1);
        close(sockfd);
        endwin();
        exit(202);
        ///exit(1);
    }

    initscr();     // inicializa a tela do curses
    cbreak();      // habilita o modo de leitura não-bloqueante de entrada de teclado
    echo();        // habilita o modo de eco de entrada de teclado
    char *str = NULL;


            /*rcvr_addr.sin_family = AF_INET;
            rcvr_addr.sin_port = htons(RCVR_PORT);   // short, network byte order
            rcvr_addr.sin_addr.s_addr = INADDR_ANY;  // automatically fill with my IP
            memset(&(rcvr_addr.sin_zero), '\0', 8);  // zero the rest of the struct*/

            /*msg.preamble = 126;
            msg.type = 1;
            msg.sequence = 0;
            msg.size=15;
            msg.data="mensagem";
            msg.crc8=10;*/
        
            

    do {
        clear();
        if (ioctl(sockfd, SIOCGIFINDEX, &ir)==-1){
            clear();
            printw("Erro de conexao - 202");
            refresh();
            close(sockfd);
            endwin();
            exit(202);//(202);;//(1);
        }
        verificaConexao(sockfd);

        printw("Digite i para entrar no modo de envio de mensagens, :q<enter> para sair do programa e :send <arquivo> <enter> para enviar um arquivo\n");
        refresh();
        ///enviar_mensagem = 0;
        c = getch();

        if (c == 105) {
            str = NULL;
            modo_edicao(&str);
            enviar_mensagem = 1;
        }

        if (c == ':') {
            seguindo_sequencia = 1;
        } else if (c == 's') {
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


        //1 = Mandar/Talker/Client
        if (enviar_mensagem){ // reverse, put the receive before //test these without ncurses, this is a pain
            
            
            
            msg.preamble = 126;
            msg.type = 1;
            msg.size=63;
            
            int msgLen = strlen(str);
            int num_packets = msgLen/63;
            int num_packets_send = 0;

            int in=0; int i=0; int ackrcvd=1;
            while (num_packets_send < num_packets || num_packets_send==0){
                if (ackrcvd==1){
                    strncpy(msg.data,str+in,in+62); //63?
                    msg.sequence=i%16;
                    msg.type=1;
                    msg.crc8=crc8(msg.data,strlen(msg.data));
                    numbytes = sendto (sockfd, &msg, sizeof(message),0,&eth_rcvr,
                                sizeof(eth_rcvr));
                    verificaConexao(sockfd);
                    if (numbytes == -1){
                        printw("Erro de sendto");
                        refresh();
                        perror("sendto");
                        close(sockfd);
                        endwin();
                        exit(202);//(202);
                    }
                    printw("Sent\n");
                    refresh();
                }
            
                addr_len = sizeof(struct sockaddr_ll);
                numbytes = recvfrom(sockfd, &msgrcv, sizeof(message), 0 , &eth_rcvr, &addr_len);
                //printw("Received");
                //refresh();
                if (numbytes>0 && msgrcv.type==10){ //&& msgrcv.data[0] == i
                    ackrcvd=1;
                    printw("Client: Ack");
                    refresh();
                    ++num_packets_send;
                    in+=63;
                    ++i;
                    //i=msgrcv.data[0];
                    //perror("recv");
                    ///break;//(1);
                } else if(msgrcv.type==0){
                    //i=msgrcv.data[0];
                    ackrcvd=1;
                    //i=msgrcv.data[0];
                    clear();
                    printw("Client: Nack");
                    refresh();
                } 
                else{
                    ackrcvd=0;
                }
            }
        

        /*while (num_packets_send < num_packets) {
                // Espera um pacote de dados do remetente
                recv(sockfd, buf, MAXBUFLEN, 0);
                struct packet *incoming_pkt = (struct packet *) buf;
                printw("duh\n");
                    refresh();
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
                    printw("duh\n");
                    refresh();
                }

                
                printw ("sent %d bytes to host %s port %d\n", numbytes,
                inet_ntoa (rcvr_addr.sin_addr), RCVR_PORT);
                refresh();
                }*/
        /*if (strlen(str)>63){

        }*/
            

            /*int num_packets = 1; //TODO calcular o número de pacotes a serem enviados
            int num_packets_send = 0;
            printw("dwqew\n",str);
            refresh();
           */
        } else{ //Else ou 2 = Receber //Server
            // waits for a packet arriving to the socket
            
            /*if ((listen(sockfd,5))!=0){
                printw("Listen failed");
                refresh();
                perror("listen");
                exit(1);
            }*/

            addr_len = sizeof(eth_snd);
            
            /*connfd = accept(sockfd, &eth_snd, addr_len);
            if (connfd==-1){
                printw("Accept failed");
                refresh();
                perror("accept");
                exit(1);
            }*/

            int i=0;
            //while (1){
            printw ("Listening for packets at port %d\n", RCVR_PORT) ;
            refresh();

            numbytes = recvfrom(sockfd, &msgrcv, sizeof(message), 0 , &eth_snd, &addr_len); //buf
            //packet *packet_rcvd = (struct packet *) buf;
            if (numbytes>0 && msgrcv.type==1 ){ //&& msgrcv.sequence==i
                printw("Packet is %d bytes long, is %d and contains \"%s\"\n", numbytes, msgrcv.sequence, msgrcv.data);
                refresh();
                msgrcv.type=10;
                msgrcv.data[0]=msgrcv.sequence+1;
                numbytes = sendto (sockfd, &msgrcv, sizeof(message),0,&eth_snd,
                        sizeof(eth_snd));
                verificaConexao(sockfd);
                if (numbytes == -1){
                    perror("sendto");
                    close(sockfd);
                    endwin();
                    exit(204);//(202);;//(1);
                }
                //clear();
                printw("AckSent\n");
                refresh();
            } else{ //Nack
                msgrcv.type=0;
                msgrcv.data[0]=msgrcv.sequence;
                numbytes = sendto (sockfd, &msgrcv, sizeof(message),0,&eth_snd,
                        sizeof(eth_snd));
                verificaConexao(sockfd);
                if (numbytes == -1){
                    perror("sendto");
                    close(sockfd);
                    endwin();
                    exit(204);//(202);;//(1);
                }
                printw("Nack\n");
                refresh();
            }
            ++i;
            //}
            
            // prints packet info and contents
            //clear();
            //printw ("Got packet from host %s:%d\n",
            //        inet_ntoa (sndr_addr.sin_addr), ntohs(sndr_addr.sin_port));
            //refresh();
            

            //buf[numbytes] = '\0';
            //printf ("Packet is %d bytes long and contains \"%s\"\n", numbytes, buf);
            //printw ("Packet is %d bytes long and contains \"%s\"\n", numbytes, msgrcv.name);
            /*if (packet_rcvd->num_seq == ULTIMO_PACKET_RECEBIDO+1) {
                ULTIMO_PACKET_RECEBIDO++;
                // Pacote correto, envia um ACK com o número de sequência correspondente
                sendto(sockfd, &ULTIMO_PACKET_RECEBIDO, sizeof(int), 0, (struct sockaddr *) &sndr_addr, sizeof(sndr_addr));

                // Imprime o conteúdo do pacote
                printw("Received: %s\n", packet_rcvd->data);
                refresh();
            } else {
                // Pacote incorreto, envia um NACK com o número de sequência esperado
                sendto(sockfd, &ULTIMO_PACKET_RECEBIDO+1, sizeof(int), 0, (struct sockaddr *) &sndr_addr, sizeof(sndr_addr));
            }*/
            //}
        }
        
    refresh();
    getch();
    //sleep(5);
    } while(1);
    
    //fecha socket
    close (sockfd);

    endwin();
 
    return 0;
}