#include <stdio.h>

// Function to add Hamming code for error detection and correction
void addHammingCode(int data[])
{
    // Calculate number of parity bits needed
    int m = 4; // Number of data bits
    int r = 3; // Number of parity bits

    // Insert parity bits at their respective positions
    int hammingCode[m + r];
    int j = 0;
    for (int i = 0; i < m + r; i++)
    {
        if (i == 0 || i == 1 || i == 3 || i == 7) {
            hammingCode[i] = 0; // Initialize parity bits to 0
        } else {
            hammingCode[i] = data[j];
            j++;
        }
    }

    // Calculate parity for each parity bit
    for (int i = 0; i < r; i++)
    {
        int count = 0;
        for (int j = 0; j < m + r; j++) {
            if ((j & (1 << i)) && hammingCode[j])
                count++;
        }
        if (count % 2 == 0) {
            hammingCode[(1 << i) - 1] = 0; // Set parity bit to make total even
        } else {
            hammingCode[(1 << i) - 1] = 1; // Set parity bit to make total odd
        }
    }

    // // Transmit data with added parity bits
    // printf("Transmitted Hamming code: ");
    // for (int i = 0; i < m + r; i++) {
    //     printf("%d ", hammingCode[i]);
    // }
    // printf("\n");
}

// Function to detect errors and correct 1 bit error using Hamming code
void detectAndCorrectError(int hammingCode[])
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
        for (int j = 0; j < m + r; j++) {
            if (((j+1) & (1 << i)) != 0) { // Check if bit is included in parity calculation
                if (hammingCode[j] == 1) {
                    count++;
                }
            }
        }
        if (count % 2 != hammingCode[(1 << i)-1]) { // Check if calculated parity matches received parity
            errorPos += (1 << i); // Set corresponding bit in error position
            errorFound = 1;
        }
    }

    // // Correct error if found
    // if (errorFound) {
    //     hammingCode[errorPos-1] = !hammingCode[errorPos-1]; // Flip erroneous bit
    //     printf("Error detected and corrected at bit position %d\n", errorPos);
    // } else {
    //     printf("No errors detected\n");
    // }
}
