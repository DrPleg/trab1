#ifndef __CRC__
#define __CRC__
#include<string.h>
#include<math.h>
#include<stdlib.h>
//#include <inttypes.h> // uint32_t, uint8_t

char* crcRemainder(char* in, char* polyn, char fill); //int input, int polyn, int filler
int crcCheck(char* in, char* polyn, char* check); //int input, int polyn, int check

//uint32_t CRC32(const uint8_t data[], size_t data_length); // wikipedia
unsigned int CRC32Wiki(unsigned int data[], size_t dataLen); //wikipedia
unsigned int crc32(const char* s, size_t n);
unsigned int crc8(unsigned int* s, size_t n);

int nPad(int n, int x);
char* nPadString(int n, char* msg);

#endif