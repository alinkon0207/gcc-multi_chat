#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <sys/socket.h>

#include "define.h"
#include "encDec.h"
#include "crc.h"
#include "hamming.h"


#define B(ptr, offset)  *((uint8_t *)(ptr) + (offset))


int recv_with_crc(int sock, void *buffer, unsigned int size, int flag)
{
    int ret;
    unsigned int crc32;

    ret = recv(sock, buffer, size, flag);
    if (ret <= 4)
    {
        // printf("Invalid length!");
        return INVALID_LEN;
    }

    crc32 = calculateCRC32(buffer, ret - 4);
    // printf("%s: ret = %d, crc32 = 0x%08X\n", __FUNCTION__, ret, crc32);
    // printf("attached crc32: 0x%02X%02X%02X%02X\n", 
    //     B(buffer, ret - 4), B(buffer, ret - 3), B(buffer, ret - 2), B(buffer, ret - 1));
    if (crc32 != ((B(buffer, ret - 4) << 24) | (B(buffer, ret- 3) << 16) | (B(buffer, ret - 2) << 8) | B(buffer, ret- 1)))
    {
        printf("Invalid CRC!");
        return INVALID_CRC;
    }
    
    B(buffer, ret - 4) = '\0';
    return ret - 4;
}

int send_with_crc(int sock, void *buffer, unsigned int size, int flag)
{
    void    *data = NULL;
    unsigned int crc32;
    int     ret;

    data = malloc(size + 4);
    if (data == NULL)
    {
        perror("Memory allocation failure!");
        return MEM_ALLOC_FAIL;
    }

    memcpy(data, buffer, size);
    crc32 = calculateCRC32(buffer, size);
    B(data, size) = crc32 >> 24;
    B(data, size + 1) = (crc32 >> 16) & 0xFF;
    B(data, size + 2) = (crc32 >> 8) & 0xFF;
    B(data, size + 3) = (crc32) & 0xFF;

    // printf("%s: len = %d, crc32 = 0x%08X\n", __FUNCTION__, size + 4, crc32);
    ret = send(sock, data, size + 4, flag);

    free(data);
    return ret;
}


int hamming_encode(void *in, void *enc, unsigned int size)
{
    int i, j;
    unsigned char nibble1, nibble2;

    for (i = j = 0; i < size; i++, j += 2)
    {
        nibble1 = (B(in, i) & 0xF0) >> 4;
        nibble2 = B(in, i) & 0x0F;

        B(enc, j) = addHammingCode(nibble1);
        B(enc, j + 1) = addHammingCode(nibble2);
    }

    return j;
}

int hamming_decode(void *in, void *dec, unsigned int size)
{
    int i, j;
    unsigned char nibble1, nibble2;
    uint8_t *src = (uint8_t *)in;
    uint8_t *dst = (uint8_t *)dec;

    for (i = j = 0; i < size; i += 2, j++)
    {
        nibble1 = detectAndCorrectError(src[i]);
        if (nibble1 == HAMMING_ERROR)
            break;
        nibble2 = detectAndCorrectError(src[i + 1]);
        if (nibble2 == HAMMING_ERROR)
            break;
        
        dst[j] = (nibble1 << 4) | nibble2;
    }
    dst[j] = 0;

    return j;
}
