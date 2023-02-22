//Formato da mensagem de comunicação
//https://wiki.inf.ufpr.br/todt/doku.php?id=especificacao_trabalho_i

//Cuidados
//https://wiki.inf.ufpr.br/maziero/doku.php?id=prog2:operacoes_com_bits
//https://stackoverflow.com/questions/4306186/structure-padding-and-packing

//padding

//little-endian
//Ao se codificar inteiros em vários bytes, utilize a codificação little-endian, isto é, o byte menos significativo vem primeiro
//isto é diferente do padrão da rede. O padrão da rede é utilizar a codificação big-endian
//e a função htons é utilizada para normalizar para big-endian), onde os bytes mais significativos vem primeiro (por exemplo, um IP 127.0.0.1 seria codificado como 0x7f 0x00 0x00 0x01
//https://en.wikipedia.org/wiki/Endianness

//tam max frame: 256 bytes?
//tam janela 16?

typedef struct message{
    unsigned char preamble : 8; // Desnec //8 bits // (01111110, 126 ou 0x7e)
    unsigned char type : 6; // 6 bits
    /*
    text: 0x01
    media: 0x10
    ack: 0x0A
    nack: 0x00
    error: 0x1E
    Start of Transmission: 0x1D
    End of Transmission: 0x0F
    data: 0x0D
    */
    unsigned char sequence : 4; // 4 bits //varies from 0 to 15, circular form (must be incremented by the machine when sending)
    unsigned char size : 6; // 6 bits // 0 up to 63 // size of a complete message: 63(data)+3(header)+1(verification) = 67 bytes
    //char* data; // ? n bytes ? (63?)
    char data[63];
    /*
    text: text of message in UTF-8
    media: start of file sent, must contain the file size in bytes, utilizing 4 bytes
    ack: 
    nack:
    error:
    */
    unsigned char crc8; // 8 bits // verified data, if size is 0, this field will be zerod
} message;