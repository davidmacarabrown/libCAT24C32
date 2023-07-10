#include "CAT24C32.h"
#include <cstring>

const uint8_t I2C_TIMING            = 5;

CAT24C32::CAT24C32(i2c_inst_t *i2c_instance, uint8_t i2c_address, uint16_t page_count) : i2c_instance(i2c_instance), i2c_address(i2c_address), page_count(page_count)
{
#ifdef DEBUG
    printf("init EEPROM\n");
#endif
}

/********************************************************************************************************************* 
Uncapped sequential write functions. Pass pointer to the source array, destination address and number of bytes to be 
written, returns diff between bytes written and read back. Assumes source buffer is not modified by any other entity 
during operation. Writes do not wrap around page boundaries.
*********************************************************************************************************************/
int8_t CAT24C32::write_multiple_bytes_checked(const uint8_t *source, uint16_t address, uint16_t num_bytes)
{
    write_multiple_bytes(source, address, num_bytes);

    uint8_t* read_buffer = new uint8_t[num_bytes];
    i2c_read_blocking(i2c_instance, i2c_address, read_buffer, num_bytes, false);
    return memcmp(source, read_buffer, num_bytes);

    delete[] read_buffer;
}

void CAT24C32::write_multiple_bytes(const uint8_t *source, uint16_t address, uint16_t num_bytes)
{
    uint16_t bytes_written = 0;
    uint8_t write_buffer[CAT24C32_PAGE_SIZE + 2];

    /* Write any initial non page-aligned bytes first so subsequent whole pages can be written in page_size blocks 
    to allow writes which would normally "wrap" around to the start of page boundary */
    uint16_t unaligned_bytes = CAT24C32_PAGE_SIZE - (address % CAT24C32_PAGE_SIZE);
    if(unaligned_bytes)
    {
        serialize_address(address, write_buffer);
        memcpy(&write_buffer[2], source, unaligned_bytes);
        i2c_write_blocking(i2c_instance, i2c_address, write_buffer, unaligned_bytes, false);
        bytes_written += unaligned_bytes;
    }

    while(bytes_written < num_bytes)
    {
        uint16_t bytes_to_write = (num_bytes - bytes_written);

        if(bytes_to_write >= CAT24C32_PAGE_SIZE)
        {
            bytes_to_write = CAT24C32_PAGE_SIZE;
        }

        serialize_address((address + bytes_written), write_buffer);
        memcpy(&write_buffer[2], &source[bytes_written], CAT24C32_PAGE_SIZE);
        i2c_write_blocking(i2c_instance, i2c_address, write_buffer, unaligned_bytes, false);
        bytes_written += bytes_to_write;
    }

}


/*********************************************************************************************************************
Uncapped read function, must pass a pointer to an array big enough to hold the requested number of bytes as these are 
not buffered internally.
*********************************************************************************************************************/
void CAT24C32::read_multiple_bytes(uint16_t address, uint16_t num_bytes, uint8_t *destination)
{
    uint8_t write_buffer[2]; //small buffer to hold address bytes

    serialize_address(address, write_buffer);
    i2c_write_blocking(i2c_instance, i2c_address, write_buffer, 2, false);
    sleep_ms(I2C_TIMING);
    i2c_read_blocking(i2c_instance, i2c_address, destination, num_bytes, false);
    sleep_ms(I2C_TIMING);
}

/* Write a given single byte to the given address */
int8_t CAT24C32::write_byte(uint8_t byte, uint16_t address)
{
    uint8_t write_buffer[4];

    /* Prepare address bytes */
    write_buffer[0] = (uint8_t) (address >> 8);
    write_buffer[1] = (uint8_t) address;

    write_buffer[2] = byte;

    i2c_write_blocking(i2c_instance, i2c_address, write_buffer, 3, false);
    sleep_ms(I2C_TIMING);

    /* Readback and compare */
    write_buffer[3] = read_byte(address);

    return memcmp(&write_buffer[2], &write_buffer[3], 1);
}

/* Reads a single byte at the given address and returns it */
uint8_t CAT24C32::read_byte(uint16_t address)
{
    uint8_t write_buffer[2];
    uint8_t read_byte;

    /* Prepare address bytes */
    write_buffer[0] = (uint8_t) (address >> 8);
    write_buffer[1] = (uint8_t) address;

    /* Set the EEPROM internal address register by writing the 2 address bytes */
    i2c_write_blocking(i2c_instance, i2c_address, write_buffer, 2, false);
    sleep_ms(I2C_TIMING);

    i2c_read_blocking(i2c_instance, i2c_address, &read_byte, 1, false);

    return read_byte;
}

void CAT24C32::erase(void)
{
    uint16_t address = 0;
    uint8_t erase_buffer[CAT24C32_PAGE_SIZE] = {0};

    for(int i = 0; i < page_count; i++)
    {
        i2c_write_blocking(i2c_instance, i2c_address, erase_buffer, CAT24C32_PAGE_SIZE, false);
        address+= CAT24C32_PAGE_SIZE;
    }
}

void CAT24C32::serialize_address(const uint16_t addr, uint8_t* buf)
{
    buf[0] = (uint8_t) (addr >> 8);
    buf[1] = (uint8_t) addr;
}