#ifndef __HAMMING_H__
#define __HAMMING_H__

unsigned char addHammingCode(unsigned char nibble);
unsigned char detectAndCorrectError(unsigned char hammingCode);

#endif  // __HAMMING_H__
