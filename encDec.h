#ifndef __ENCDEC_H__
#define __ENCDEC_H__

int recv_with_crc(int sock, void *buffer, unsigned int size, int flag);
int send_with_crc(int sock, void *buffer, unsigned int size, int flag);

int hamming_encode(void *in, void *enc, unsigned int size);
int hamming_decode(void *in, void *dec, unsigned int size);

#endif  // __ENCDEC_H__
