#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <sys/stat.h>


#define ESC 27

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

void remove_espacos(char texto[]) {
    if (texto[strlen(texto) - 1] == ' ') {
        texto[strlen(texto) - 1] = '\0';
    }
}

int enviar_arquivo(char *string) {
    struct stat file_info;
    if (stat(string, &file_info) != 0) {
        printw("ERRO 205\n");
        return 1;
    }

    if ((!file_info.st_mode & S_IRUSR)) {
        printw("ERRO 206\n");
        return 1;
    }
    
    printw("podemos enviar\n");
    //TODO FUNÇÃO DE ENVIAR ARQUIVO
}


void modo_edicao() {
    printw("Digite sua mensagem, e ao terminar digite <Enter>, ou, para cancelar pressione <ESC>\n\n");
    char c,ultimo_char;
    char buffer[100] = "";
    int sair_modo_edicao = 0, seguindo_sequencia = 0;
    echo();

    c = 0;
    ultimo_char = 0;
    *buffer = 0;
    sair_modo_edicao = 0;
    seguindo_sequencia = 0;

    do {
        c = getch();
        if (c != ESC && c != '\n'){
            concatenar_letra(buffer, c);
        }

        if (c == 127) {
            remover_letra(buffer);
            clear();
            printw("%s", buffer);       
        }
    } while (c != ESC && c != '\n');
    clear();

    if (c == '\n') {
        printw("\n\nenviamos a seguinte mensagem = %s\n", buffer);
        //enviar_mensagem(buffer);
    }
    refresh();

    return;
}

int main() {
    char c, ultimo_char;
    int i;
    char arquivo[100];
    int seguindo_sequencia = 0; //1 se estiver seguindo a sequência :q<enter>


    initscr();     // inicializa a tela do curses
    cbreak();      // habilita o modo de leitura não-bloqueante de entrada de teclado
    echo();        // habilita o modo de eco de entrada de teclado
    printw("Digite i para entrar no modo de envio de mensagens, :q<enter> para sair do programa e :send <arquivo> <enter> para enviar um arquivo\n");

    do {
        c = getch();

        if (c == 105) {
            modo_edicao();
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
    } while(1);

    endwin();      // restaura o terminal para o seu estado original
    return 0;
}
