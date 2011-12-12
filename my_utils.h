#ifndef MY_UTILS_GUARD
#define MY_UTILS_GUARD
#include "stdint.h"
#include <string>

int getFilesize(std::string filename);
short convertTwoBytesToShort(char a, char b);
uint32_t convertFourBytesToInt(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
void convertCharArrayToShort(char* arr, short* arr2, int arraySize);
int16_t getIntFromChars(uint8_t a, uint8_t b);
int32_t getIntFromChars(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

#endif
