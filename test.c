#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "encDec.h"
#include "crc.h"

int test_encode();
int test_decode();

int main()
{
    test_encode();
    test_decode();
}

int test_encode()
{
    FILE *fpIn;
    uint8_t* pIn;
    uint32_t dwSize;
    uint8_t* pOut;
    FILE *fpOut;
    uint32_t crc32;
    int i;

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

    pOut = malloc(BUFFER_SIZE * 2 + 4);
    if (pOut == NULL)
    {
        printf("Failed to allocate memory!");
        return -2;
    }

    hamming_encode(pIn, pOut, dwSize);
    free(pIn);

    crc32 = calculateCRC32(pOut, dwSize * 2);
    B(pOut, dwSize * 2) = (crc32) & 0xFF;
    B(pOut, dwSize * 2 + 1) = (crc32 >> 8) & 0xFF;
    B(pOut, dwSize * 2 + 2) = (crc32 >> 16) & 0xFF;
    B(pOut, dwSize * 2 + 3) = (crc32 >> 24) & 0xFF;

    fpOut = fopen("temp.txt", "wb");
    if (fpOut == NULL)
    {
        printf("Failed to open temp file!\n");
        return -1;
    }

    for (i = 0; i < dwSize * 2 + 4; i++)
        fprintf(fpOut, "%02X", pOut[i]);
    
    fclose(fpOut);
    free(pOut);

    return 0;
}

int test_decode()
{
    FILE *fpIn;
    uint8_t* pIn;
    uint32_t dwSize;
    uint8_t* pOut;
    FILE *fpOut;
    uint32_t crc32, file_crc;
    int i;

    fpIn = fopen("temp.txt", "rb");
    if (fpIn == NULL)
    {
        printf("Failed to open temp file!\n");
        return -1;
    }

    fseek(fpIn, 0, SEEK_END);
    dwSize = ftell(fpIn);
    fseek(fpIn, 0, SEEK_SET);

    if (dwSize <= 8)
    {
        printf("Invalid file length!");
        fclose(fpIn);
        return -4;
    }
    // printf("Size = %d\n", dwSize);
    
    pIn = malloc(dwSize / 2);
    if (pIn == NULL)
    {
        printf("Failed to allocate memory!");
        fclose(fpIn);
        return -2;
    }

    for (i = 0; i < dwSize / 2; i++)
    {
        fscanf(fpIn, "%02hhX", &pIn[i]);
        // printf("%02X", pIn[i]);
    }
    printf("\n");

    fclose(fpIn);

    dwSize = dwSize / 2 - 4;

    crc32 = calculateCRC32(pIn, dwSize);
    file_crc = B(pIn, dwSize) | (B(pIn, dwSize + 1) << 8) | (B(pIn, dwSize + 2) << 16) | (B(pIn, dwSize + 3) << 24);
    if (crc32 != file_crc)
    {
        printf("CRC32 mismatch!\n");
        free(pIn);
        return -3;
    }

    pOut = malloc(BUFFER_SIZE);
    if (pOut == NULL)
    {
        printf("Failed to allocate memory!\n");
        free(pIn);
        return -2;
    }

    hamming_decode(pIn, pOut, dwSize);
    free(pIn);

    fpOut = fopen("result.txt", "wb");
    if (fpOut == NULL)
    {
        printf("Failed to open result file!\n");
        return -1;
    }

    fwrite(pOut, 1, dwSize / 2, fpOut);
    fclose(fpOut);
    
    free(pOut);

    return 0;
}
