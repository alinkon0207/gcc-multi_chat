#include "define.h"
#include <stdio.h>
#include <stdint.h>

// Function to calculate CRC32 for a given data stream
uint32_t calculateCRC32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF; // Initialize register with all bits set to 1
    const uint32_t polynomial = 0xEDB88320; // Pre-defined constant

    for (size_t i = 0; i < length; i++)
    {
        uint8_t byte = data[i];
        crc ^= byte; // XOR byte with least significant byte of register

        for (size_t j = 0; j < 8; j++)
        {
            // Process each bit of byte
            if (crc & 1)    // Check if least significant bit of register is 1
                crc = (crc >> 1) ^ polynomial; // Shift register one bit to the right and XOR with polynomial
            else
                crc = crc >> 1; // Shift register one bit to the right
        }
    }

    return ~crc; // Final value of register is CRC32 value (with all bits inverted)
}
