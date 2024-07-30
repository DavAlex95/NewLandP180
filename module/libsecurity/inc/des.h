#ifndef DES_H_
#define DES_H_

#include <stdint.h>

/* Single-key DES key schedule */
int deskey(uint8_t key[8], int decrypt);
int des(uint8_t block[8], uint8_t output[8]);

/* Triple-DES key schedule */
int des3key2(uint8_t key[16], int decrypt);
int des3key3(uint8_t key[24], int decrypt);
int des3(uint8_t block[8], uint8_t output[8]);


#endif
