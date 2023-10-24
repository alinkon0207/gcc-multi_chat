#ifndef __DEFINE_H__
#define __DEFINE_H__

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;


#define MEM_ALLOC_FAIL  -1
#define INVALID_LEN     -2
#define INVALID_CRC     -3
#define HAMMING_ERROR   -4

#define PORT            5000
#define MAX_CLIENTS     10
#define BUFFER_SIZE     50001

#define B(ptr, offset)  *((uint8_t *)(ptr) + (offset))

#endif  /// __DEFINE_H__
