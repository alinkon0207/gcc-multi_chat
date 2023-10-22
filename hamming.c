#include "define.h"
#include "hamming.h"
#include <stdio.h>

// Function to add Hamming code for error detection and correction
unsigned char addHammingCode(unsigned char nibble)
{
    // Calculate number of parity bits needed
    int m = 4; // Number of data bits
    int r = 3; // Number of parity bits

    // Insert parity bits at their respective positions
    unsigned char hammingCode = 0;
    int j = 0;
    for (int i = 0; i < m + r; i++)
    {
        if (i == 0 || i == 1 || i == 3 || i == 7)
        {
            // hammingCode[i] = 0; // Initialize parity bits to 0
        }
        else
        {
            hammingCode |= ((nibble >> j) & 1) << i;
            j++;
        }
    }

    // Calculate parity for each parity bit
    for (int i = 0; i < r; i++)
    {
        int count = 0;
        for (int j = 0; j < m + r; j++)
        {
            if ((j & (1 << i)) && ((hammingCode >> j) & 1))
                count++;
        }
        
        // printf(">>> hammingCode = 0x%02X, i = %d, count = %d\n", hammingCode, i, count);
        if (count % 2 == 0)
            hammingCode &= ~(1 << ((1 << i) - 1)); // Set parity bit to make total even
        else
            hammingCode |= 1 << ((1 << i) - 1); // Set parity bit to make total odd
    }

    // printf("Transmitted Hamming code: 0x%X\n", hammingCode);
    return hammingCode;
}

// Function to detect errors and correct 1 bit error using Hamming code
unsigned char detectAndCorrectError(unsigned char hammingCode)
{
    // Calculate number of parity bits needed
    int m = 4; // Number of data bits
    int r = 3; // Number of parity bits

    // Calculate parity for each received parity bit
    int errorPos = 0;
    int errorFound = 0;

    for (int i = 0; i < r; i++)
    {
        int count = 0;
        for (int j = 0; j < m + r; j++)
        {
            if (((j + 1) & j) != 0 && (j & (1 << i)) != 0)    // Check if bit is included in parity calculation
            {
                if ((hammingCode >> j) & 1)
                    count++;
            }
        }
        
        // printf("<<< hammingCode = 0x%02X, i = %d, count = %d\n", hammingCode, i, count);
        if (count % 2 != ((hammingCode >> ((1 << i)-1)) & 1))   // Check if calculated parity matches received parity
        {
            errorPos += (1 << i); // Set corresponding bit in error position
            errorFound = 1;
        }
    }

    // Correct error if found
    if (errorFound)
    {
        hammingCode ^= 1 << (errorPos-1); // Flip erroneous bit
        printf("Error detected and corrected at bit position %d\n", errorPos);
    }

    // recover nibble
    unsigned char nibble = 0;
    int j = 0;
    for (int i = 0; i < m + r; i++)
    {
        if (i == 0 || i == 1 || i == 3 || i == 7)
        {
            // hammingCode[i] = 0; // Initialize parity bits to 0
        }
        else
        {
            nibble |= ((hammingCode >> i) & 1) << j;
            j++;
        }
    }

    // printf("nibble = 0x%X\n", nibble);
    return nibble;
}
