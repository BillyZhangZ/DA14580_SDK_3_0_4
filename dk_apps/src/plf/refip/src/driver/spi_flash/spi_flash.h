#ifndef _SPI_FLASH_
#define _SPI_FLASH_

/**
 ****************************************************************************************
 *
 * @file spi_flash.h
 *
 * @brief flash memory driver over spi interface header file.
 *
 * Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */
#define SPI_FLASH_DRIVER_VERSION (2)
#define SPI_FLASH_DRIVER_SUBVERSION (1)

 #include "spi.h"

 /*
	Tested SPI Flashes
		- W25x10/windbond
*/

// Definitions for the various SPI Flash Devices

// 1. W25X10CL
#define W25X10_MAN_DEV_ID 0xEF10
#define W25x10_MEM_PROT_NONE 0
#define W25x10_MEM_PROT_UPPER_HALF 1
#define W25x10_MEM_PROT_LOWER_HALF 9

// 1. W25X20CL
#define W25X20_MAN_DEV_ID 0xEF11
#define W25x20_MEM_PROT_NONE 0
#define W25x20_MEM_PROT_UPPER_QUARTER 1
#define W25x20_MEM_PROT_UPPER_HALF 2
#define W25x20_MEM_PROT_LOWER_QUARTER 9
#define W25x20_MEM_PROT_LOWER_HALF 10
#define W25x20_MEM_PROT_ALL 3

#define W25x_MEM_PROT_BITMASK 0x2C


typedef enum SPI_ERASE_MODULE
{
    BLOCK_ERASE_64  = 0xd8,
    BLOCK_ERASE_32  = 0x52,
    SECTOR_ERASE    = 0x20,
} SPI_erase_module_t;

#define	MAX_READY_WAIT_COUNT	200000
#define	MAX_COMMAND_SEND_COUNT 50

/* Status Register Bits */
#define STATUS_BUSY		0x01
#define	STATUS_WEL		0x02
#define	STATUS_BP0		0x04
#define	STATUS_BP1		0x08
#define	STATUS_TB   	0x20
#define	STATUS_SRP		0x80

#define ERR_OK				    0
#define ERR_TIMEOUT			    -1
#define ERR_NOT_ERASED  	    -2
#define ERR_PROTECTED	        -3
#define ERR_INVAL			    -4
#define ERR_ALIGN			    -5
#define ERR_UNKNOWN_FLASH_VENDOR -6
#define ERR_UNKNOWN_FLASH_TYPE	 -7
#define ERR_PROG_ERROR			 -8

/* commands */
#define WRITE_ENABLE      0x06 
#define WRITE_ENABLE_VOL  0x50 
#define WRITE_DISABLE     0x04 

#define READ_STATUS_REG   0x05
#define WRITE_STATUS_REG  0x01
#define PAGE_PROGRAM      0x02
#define QUAD_PAGE_PROGRAM 0x32
#define CHIP_ERASE        0xC7  
//                        ^^^// or 0x60
#define ERASE_SUSPEND     0x75
#define ERASE_RESUME      0x7a
#define POWER_DOWN        0xb9
#define HIGH_PERF_MODE    0xa3
#define MODE_BIT_RESET    0xff
#define REL_POWER_DOWN    0xab
#define MAN_DEV_ID        0x90
#define READ_UNIQUE_ID    0x4b
#define JEDEC_ID          0x9f
#define READ_DATA         0x03
#define FAST_READ         0x0b

/**
 ****************************************************************************************
 * @brief Initialize SPI Flash
 * @param[in ]spi_flash_size_param:         Flash Size
 * @param[in] spi_flash_page_size_param:    Flash Page Size
 ****************************************************************************************
 */
void spi_flash_init(uint32_t spi_flash_size_param, uint32_t spi_flash_page_size_param);
/**
 ****************************************************************************************
 * @brief Wait till flash is ready for next action 
 * @return  Success : ERR_OK
 *          Failure : ERR_TIMEOUT 
 ****************************************************************************************
 */
int8_t spi_flash_wait_till_ready (void);
/**
 ****************************************************************************************
 * @brief Read Status Register
 * @return  Status Register value
 ****************************************************************************************
 */
uint8_t spi_flash_read_status_reg(void);

 /**
 ****************************************************************************************
 * @brief Issue a Write Enable Command  
 * @return error code or success (ERR_OK)  
 ****************************************************************************************
 */  
int8_t spi_flash_set_write_enable(void);
/**
 ****************************************************************************************
 * @brief Issue a Write Enable Volatile Command  
 * @return error code or success (ERR_OK)  
 ****************************************************************************************
 */  
int8_t spi_flash_write_enable_volatile(void);

/**
 ****************************************************************************************
 * @brief Issue a Write Disable Command  
 * @return error code or success (ERR_OK)  
 ****************************************************************************************
 */  
int8_t spi_flash_set_write_disable(void);

/**
 ****************************************************************************************
 * @brief Write Status Register
 * @param[in] dataToWrite:   Value to be written to Status Register
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int32_t spi_flash_write_status_reg(uint8_t dataToWrite);

/**
 ****************************************************************************************
 * @brief Read data from a given starting address (up to the end of the flash)
 *
 * @param[in] *rd_data_ptr:  Points to the position the read data will be stored
 * @param[in] address:       Starting address of data to be read
 * @param[in] size:          Size of the data to be read
 * 
 * @return  Number of read bytes or error code
 ****************************************************************************************
 */
uint32_t spi_flash_read_data (uint8_t *rd_data_ptr, uint32_t address, uint32_t size);

/**
 ****************************************************************************************
 * @brief Program page (up to <SPI Flash page size> bytes) starting at given address
 *
 * @param[in] *wr_data_ptr:  Pointer to the data to be written
 * @param[in] address:       Starting address of data to be written
 * @param[in] size:          Size of the data to be written (should not be larger than SPI Flash page size)
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int32_t spi_flash_page_program(uint8_t *wr_data_ptr, uint32_t address, uint16_t size);

 /**
 ****************************************************************************************
 * @brief Issue a comamnd to Erase a given address
 *
 * @param[in] address:  Address that belongs to the block64/block32/sector range
 * @param[in] spiEraseModule: BLOCK_ERASE_64, BLOCK_ERASE_32, SECTOR_ERASE
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int8_t spi_flash_block_erase(uint32_t address, SPI_erase_module_t spiEraseModule);

/**
 ****************************************************************************************
 * @brief Erase chip
 * @note In order for the erasure to succeed, all locking options must be disabled.
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int8_t spi_flash_chip_erase(void);
 
/**
 ****************************************************************************************
 * @brief verify erasure
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int8_t TO_BE_IMPLEMENTED_spi_flash_check_erase(unsigned long dest_addr, unsigned long len);
/**
 ****************************************************************************************
 * @brief Get Manufacturer / Device ID
 * @return  Manufacturer/Device ID
 ****************************************************************************************
 */
int16_t spi_read_flash_memory_man_and_dev_id(void);

/**
 ****************************************************************************************
 * @brief Get Unique ID Number
 * @return  Unique ID Number
 ****************************************************************************************
 */
uint64_t spi_read_flash_unique_id(void);

/**
 ****************************************************************************************
 * @brief Get JEDEC ID
 * @return  JEDEC ID
 ****************************************************************************************
 */
int32_t spi_read_flash_jedec_id(void);

/**
 ****************************************************************************************
 * @brief Write data to flash across page boundaries and at any starting address
 *
 * @param[in] *wr_data_ptr:  Pointer to the data to be written
 * @param[in] address:       Starting address of page to be written (must be a multiple of SPI Flash page size)
 * @param[in] size:          Size of the data to be written (can be larger than SPI Flash page size)
 * 
 * @return  Number of bytes actually written
 ****************************************************************************************
 */
int32_t spi_flash_write_data (uint8_t * wr_data_ptr, uint32_t address, uint32_t size);
 
 
 /**
 ****************************************************************************************
 * @brief Sends the Power-Down instruction
 * Remark: The function spi_flash_release_from_powerdown() is used to enable the IC again
 ****************************************************************************************
 */
int32_t spi_flash_power_down(void);

/**
 ****************************************************************************************
 * @brief Sends the Release from Power-Down instruction
 * Remark: This function is used to restore the IC from power-down mode
 ****************************************************************************************
 */
int32_t spi_flash_release_from_power_down(void);
 
/**
 ****************************************************************************************
 * @brief Selects the the memory protection configuration
 * @param[in] SPI_flash_memory_protection_setting
 ****************************************************************************************
 */
int32_t spi_flash_configure_memory_protection(uint8_t spi_flash_memory_protection_setting);

/**
 ****************************************************************************************
 * @brief Erase chip even if locked
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int8_t spi_flash_chip_erase_forced(void);

/**
 ****************************************************************************************
 * @brief Fill memory page (up to <SPI Flash page size> bytes) with a given 1-byte value
 *        starting at given address
 *
 * @param[in] value:         Value used to fill memory
 * @param[in] address:       Starting address
 * @param[in] size:          Size of the area to be filled (should not be larger than SPI Flash page size)
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int8_t spi_flash_page_fill(uint8_t value, uint32_t address, uint16_t size);

/**
 ****************************************************************************************
 * @brief Fill memory with a 1-byte value, across page boundaries and at any starting address
 *
 * @param[in] value:    The value with which memory will be filled
 * @param[in] address:  Starting address of page to be written (must be a multiple of SPI Flash page size)
 * @param[in] size:     Size of the area to be filled (can be larger than SPI Flash page size)
 * 
 * @return  Number of bytes actually written
 ****************************************************************************************
 */
int32_t spi_flash_fill (uint8_t value, uint32_t address, uint32_t size);

#endif //_SPI_FLASH_
