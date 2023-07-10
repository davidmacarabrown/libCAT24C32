#ifndef CAT24C32_H
#define CAT24C32_H

/* C/C++ Includes */
//#include <memory.h>
//#include <stdio.h>

/* Pico SDK Includes */
#include "pico/stdlib.h"
#include "hardware/i2c.h"

const uint8_t CAT24C32_PAGE_SIZE    = 32;

//TODO: Implement hardware watchdog to handle outboard read/write failures causing i2c_read/write to block or switch to "block_until" variants
class CAT24C32
{
    private:
        i2c_inst_t* i2c_instance = NULL;
        uint8_t i2c_address = 0;
        uint16_t page_count = 0;

        /* Private Utility Functions */
        void serialize_address(const uint16_t address, uint8_t* buf);
        
    public:
        CAT24C32();
        CAT24C32(i2c_inst_t *i2c_instance, uint8_t i2c_address, uint16_t page_count);

        int8_t write_multiple_bytes_checked(const uint8_t *source, uint16_t address, uint16_t num_bytes);
        void write_multiple_bytes(const uint8_t *source, uint16_t address, uint16_t num_bytes);

        void read_multiple_bytes(uint16_t address, uint16_t num_bytes, uint8_t *dest);

        int8_t write_byte(const uint8_t byte, uint16_t address);
        uint8_t read_byte(uint16_t address);

        void erase(void);

        void test(void);
};

#endif

//TODO: For later... page counter to increment pages?
