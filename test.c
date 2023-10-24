#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "encDec.h"
#include "crc.h"

int main()
{
    FILE *fpIn;
    uint8_t* pIn;
    uint32_t dwSize;
    uint8_t* pOut;
    FILE *fpOut;
    uint32_t crc32;

    fpIn = fopen("intext.txt", "rb");
    if (fpIn == NULL)
    {
        printf("Failed to open input file!\n");
        return -1;
    }

    pIn = malloc(BUFFER_SIZE);
    if (pIn == NULL)
    {
        printf("Failed to allocate memory!");
        fclose(fpIn);
        return -2;
    }

    dwSize = fread(pIn, 1, BUFFER_SIZE, fpIn);    
    fclose(fpIn);

    pOut = malloc(BUFFER_SIZE * 2);
    if (pOut == NULL)
    {
        printf("Failed to allocate memory!");
        return -2;
    }

    hamming_encode(pIn, pOut, dwSize);
    crc32 = calculateCRC32(pOut, dwSize * 2);
    B(pOut, dwSize * 2) = (crc32) & 0xFF;
    B(pOut, dwSize * 2 + 1) = (crc32 >> 8) & 0xFF;
    B(pOut, dwSize * 2 + 2) = (crc32 >> 16) & 0xFF;
    B(pOut, dwSize * 2 + 3) = (crc32 >> 24) & 0xFF;

    fpOut = fopen("result.txt", "wb");
    if (fpIn == NULL)
    {
        printf("Failed to open output file!\n");
        return -1;
    }

    fwrite(pOut, 1, dwSize * 2 + 4, fpOut);
    fclose(fpOut);

    return 0;
}
