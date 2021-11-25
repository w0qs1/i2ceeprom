/**
 * @file
 * @code #include "i2ceeprom.h"
 * @endcode
 * 
 * @brief A Library for Interfacing AVR with 24CXX EEPROMs.
 * 
 * Basic functions for use with 24CXX EEPROMs.
 *  
 * Requires: I2C Master library by Peter Fleury pfleury@gmx.ch
 * 
 * @version 1.0
 * @author Sandeep Kumar http://www.github.com/w0qs1
 * @copyright (C) 2021 Sandeep Kumar, GNU General Public License Version 3
 * 
 * @par Usage Example:
 * 
 * @code
 * #include <avr/io.h>
 * #include <util/delay.h>
 * #include <i2cmaster.h>
 * #include <i2ceeprom.h>
 * 
 * #define EEPROM_ADDRESS 0xA0
 * 
 * int main(void) {
 *     eeprom eep1;
 *     eeprom_init(&eep1, EEPROM_ADDRESS, 8);      // AT24C08 -> 8Kbit EEPROM
 * 
 *     uint8_t data_write[] = {0xCA, 0xFE, 0xBA, 0xBE};
 *     eeprom_write(&eep1, 0x00, data_write, 4);   // Write 4 bytes
 * 
 *     uint8_t data_read[4];
 *     eeprom_read(&eep1, 0x00, data_read, 4);     // Read 4 bytes
 * }
 * @endcode
 */

// 1K, 2K       - 1 0 1 0  E2 E1 E0 RW
// 4K           - 1 0 1 0  E2 E1 A8 RW
// 8K           - 1 0 1 0  E2 A9 A8 RW
// 16K          - 1 0 1 0 A10 A9 A8 RW
// 32K, 64K     - 1 0 1 0  E2 E1 E0 RW
// 128K, 256K   - 1 0 1 0   0 E1 E0 RW
// 512K         - 1 0 1 0   0 E1 E0 RW
// 1M           - 1 0 1 0   0 E1 E0 RW

// Ex - Device Address selection bits
// Ax - Memory Address selection bits

/**
 * @brief  This creates a new EEPROM instance
 * @note   The address of the instance must be passed in a function call
 */
typedef struct eeprom {
    uint8_t eeprom_address;                 // Device Address
    uint16_t eeprom_size;                   // Size of EEPROM in Kbits
}eeprom;

/**
 * @brief  To set the EEPROM properties
 * @note   This is used to set the I2C address and the size of the EEPROM.
 * @param  *a: Address of the EEPROM instance
 * @param  dev_address: I2C address of the EEPROM
 * @param  size: Size of the EEPROM in Kbits
 * @return None
 */
void eeprom_init(eeprom *a, uint8_t dev_address, uint16_t size) {
    i2c_init();
    a->eeprom_address = dev_address;         // Set the device address
    a->eeprom_size = size;                   // Set the eeprom size (size in kbits)
}

/**
 * @brief  To write a byte array of data
 * @param  *a: Address of the EEPROM instance
 * @param  mem_address: Starting address
 * @param  *data: Data Array
 * @param  datasize: Size of the data array
 * @return None
 */
void eeprom_write(eeprom *a, uint16_t mem_address, uint8_t *data, uint16_t datasize) {
    uint8_t actual_address;
    uint16_t pos_in_page;
    uint16_t bytes_written = 0;
    uint16_t pages_written = 0;
    if (a->eeprom_size == 1 || a->eeprom_size == 2) {
        pos_in_page = mem_address % 8;                                              // 8 byte page write
        while (bytes_written < datasize) {
            // fixed device address
            i2c_start_wait(a->eeprom_address + I2C_WRITE);                           // set the device address
            i2c_write((uint8_t) mem_address + (8 * pos_in_page));                   // set the write address (one byte address)

            // write one page of data
            for (uint8_t i = pos_in_page; (i < 8) && (bytes_written < datasize); ++i) {
                i2c_write(*(data + bytes_written));
                ++bytes_written;
            }

            // go to the next page and set the position in the page to 0
            ++pages_written;
            pos_in_page = 0;
        }
    } else if (a->eeprom_size == 4) {
        pos_in_page = mem_address % 16;                                             // 16 byte page write
        while (bytes_written < datasize) {
            actual_address = (a->eeprom_address | ((mem_address >> 8) & 1));        // set the device address from the MSB bits
            i2c_start_wait(actual_address + I2C_WRITE);                             // set the device address
            i2c_write(((uint8_t) mem_address) + (16 * pos_in_page));                // set the write address (one byte address)

            // write one page of data
            for (uint8_t i = pos_in_page; (i < 16) && (bytes_written < datasize); ++i) {
                i2c_write(*(data + bytes_written));
                ++bytes_written;
            }

            // go to the next page and set the position in the page to 0
            ++pages_written;
            pos_in_page = 0;
        }       
    } else if (a->eeprom_size == 8) {
        pos_in_page = mem_address % 16;                                             // 16 byte page write
        while (bytes_written < datasize) {
            actual_address = (a->eeprom_address | ((mem_address >> 8) & 3));        // set the device address from the MSB bits
            i2c_start_wait(actual_address + I2C_WRITE);                             // set the device address
            i2c_write(((uint8_t) mem_address) + (16 * pos_in_page));                // set the write address (one byte address)

            // write one page of data
            for (uint8_t i = pos_in_page; (i < 16) && (bytes_written < datasize); ++i) {
                i2c_write(*(data + bytes_written));
                ++bytes_written;
            }

            // go to the next page and set the position in the page to 0
            ++pages_written;
            pos_in_page = 0;
        } 
    } else if (a->eeprom_size == 16) {
        pos_in_page = mem_address % 16;                                             // 16 byte page write
        while (bytes_written < datasize) {
            actual_address = (a->eeprom_address | ((mem_address >> 8) & 7));        // set the device address from the MSB bits
            i2c_start_wait(actual_address + I2C_WRITE);                             // set the device address
            i2c_write(((uint8_t) mem_address) + (16 * pos_in_page));                // set the write address (one byte address)

            // write one page of data
            for (uint8_t i = pos_in_page; (i < 16) && (bytes_written < datasize); ++i) {
                i2c_write(*(data + bytes_written));
                ++bytes_written;
            }

            // go to the next page and set the position in the page to 0
            ++pages_written;
            pos_in_page = 0;
        } 
    } else if (a->eeprom_size == 32 || a->eeprom_size == 64) {
        // fixed device address
        pos_in_page = mem_address % 32;                                             // 32 byte page write
        while (bytes_written < datasize) {
            // fixed device address
            i2c_start_wait(a->eeprom_address + I2C_WRITE);                           // set the device address
            i2c_write((mem_address & 0xFF00) >> 8);                                 // write the MSB address first
            i2c_write((mem_address & 0x00FF) + (32 * pos_in_page));                 // write the LSB address next

            // write one page of data
            for (uint8_t i = pos_in_page; (i < 32) && (bytes_written < datasize); ++i) {
                i2c_write(*(data + bytes_written));
                ++bytes_written;
            }

            // go to the next page and set the position in the page to 0
            ++pages_written;
            pos_in_page = 0;
        }
    } else if (a->eeprom_size == 128 || a->eeprom_size == 256) {
        // fixed device address
        pos_in_page = mem_address % 64;                                             // 64 byte page write
        while (bytes_written < datasize) {
            // fixed device address
            i2c_start_wait(a->eeprom_address + I2C_WRITE);                          // set the device address
            i2c_write((mem_address & 0xFF00) >> 8);                                 // write the MSB address first
            i2c_write((mem_address & 0x00FF) + (64 * pos_in_page));                 // write the LSB address next

            // write one page of data
            for (uint8_t i = pos_in_page; (i < 64) && (bytes_written < datasize); ++i) {
                i2c_write(*(data + bytes_written));
                ++bytes_written;
            }

            // go to the next page and set the position in the page to 0
            ++pages_written;
            pos_in_page = 0;
        }
    } else if (a->eeprom_size == 512) {
        // fixed device address
        pos_in_page = mem_address % 128;                                            // 128 byte page write
        while (bytes_written < datasize) {
            // fixed device address
            i2c_start_wait(a->eeprom_address + I2C_WRITE);                           // set the device address
            i2c_write((mem_address & 0xFF00) >> 8);                                 // write the MSB address first
            i2c_write((mem_address & 0x00FF) + (128 * pos_in_page));                // write the LSB address next

            // write one page of data
            for (uint8_t i = pos_in_page; (i < 128) && (bytes_written < datasize); ++i) {
                i2c_write(*(data + bytes_written));
                ++bytes_written;
            }

            // go to the next page and set the position in the page to 0
            ++pages_written;
            pos_in_page = 0;
        }
    } else if (a->eeprom_size == 1024) {
        // fixed device address
        pos_in_page = mem_address % 256;                                            // 256 byte page write
        while (bytes_written < datasize) {
            // fixed device address
            i2c_start_wait(a->eeprom_address + I2C_WRITE);                           // set the device address
            i2c_write((mem_address & 0xFF00) >> 8);                                 // write the MSB address first
            i2c_write((mem_address & 0x00FF) + (256 * pos_in_page));                // write the LSB address next

            // write one page of data
            for (uint16_t i = pos_in_page; (i < 256) && (bytes_written < datasize); ++i) {
                i2c_write(*(data + bytes_written));
                ++bytes_written;
            }

            // go to the next page and set the position in the page to 0
            ++pages_written;
            pos_in_page = 0;
        }
    }
    i2c_stop();
}

/**
 * @brief  To read a byte array
 * @param  *a: Address of the EEPROM instance
 * @param  mem_address: Starting address
 * @param  *data: Data Array
 * @param  datasize: Size of the data array
 * @return None
 */
void eeprom_read(eeprom *a, uint16_t mem_address, uint8_t *data, uint16_t datasize) {
    uint8_t actual_address;
    uint16_t bytes_read = 0;

    if (a->eeprom_size == 1 || a->eeprom_size == 2) {
        i2c_start_wait(a->eeprom_address + I2C_WRITE);
        i2c_write((uint8_t) mem_address);
        i2c_stop();
        i2c_start_wait(a->eeprom_address + I2C_READ);

        while(bytes_read < datasize - 1) {
            *(data + bytes_read) = i2c_readAck();                                   // Read datasize - 1 bytes
            ++bytes_read;
        }
        *(data + bytes_read) = i2c_readNak();                                       // Read the last byte
    } else if (a->eeprom_size == 4) {
        actual_address = (a->eeprom_address | ((mem_address >> 8) & 1));
        i2c_start_wait(actual_address + I2C_WRITE);
        i2c_write((uint8_t) mem_address);
        i2c_stop();
        i2c_start_wait(actual_address + I2C_READ);

        while(bytes_read < datasize - 1) {
            *(data + bytes_read) = i2c_readAck();                                   // Read datasize - 1 bytes
            ++bytes_read;
        }
        *(data + bytes_read) = i2c_readNak();                                       // Read the last byte
    } else if (a->eeprom_size == 8) {
        actual_address = (a->eeprom_address | ((mem_address >> 8) & 3));
        i2c_start_wait(actual_address + I2C_WRITE);
        i2c_write((uint8_t) mem_address + bytes_read);
        i2c_stop();
        i2c_start_wait(actual_address + I2C_READ);

        while(bytes_read < datasize - 1) {
            *(data + bytes_read) = i2c_readAck();                                   // Read datasize - 1 bytes
            ++bytes_read;
        }
        *(data + bytes_read) = i2c_readNak();                                       // Read the last byte
    } else if (a->eeprom_size == 16) {
        actual_address = (a->eeprom_address | ((mem_address >> 8) & 7));
        i2c_start_wait(actual_address + I2C_WRITE);
        i2c_write((uint8_t) mem_address);
        i2c_stop();
        i2c_start_wait(actual_address + I2C_READ);

        while(bytes_read < datasize - 1) {
            *(data + bytes_read) = i2c_readAck();                                   // Read datasize - 1 bytes
            ++bytes_read;
        }
        *(data + bytes_read) = i2c_readNak();                                       // Read the last byte
    } else if (a->eeprom_size == 32 || a->eeprom_size == 64 || a->eeprom_size == 128 || a->eeprom_size == 256 || a->eeprom_size == 512 || a->eeprom_size == 1024) {
        i2c_start_wait(a->eeprom_address + I2C_WRITE);
        i2c_write((uint8_t) ((mem_address & 0xFF00) >> 8));                         // Write MSB address first
        i2c_write((uint8_t) (mem_address & 0x00FF));                                // Write LSB address
        i2c_stop();
        i2c_start_wait(a->eeprom_address + I2C_READ);

        while(bytes_read < datasize - 1) {
            *(data + bytes_read) = i2c_readAck();                                   // Read datasize - 1 bytes
            ++bytes_read;
        }
        *(data + bytes_read) = i2c_readNak();                                       // Read the last byte
    }
    i2c_stop();
}