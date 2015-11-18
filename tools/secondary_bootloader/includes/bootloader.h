 /**
 ****************************************************************************************
 *
 * @file downloader.h
 *
 * @brief  firmware download protocol settings
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
#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

/************** UART Booter section**************/
#undef UART_SUPPORTED

/************** Flash memory section  **************/
#define SPI_FLASH_SUPPORTED
#undef EEPROM_FLASH_SUPPORTED

#if  defined (SPI_FLASH_SUPPORTED ) && defined ( EEPROM_FLASH_SUPPORTED )
    #undef  EEPROM_FLASH_SUPPORTED
    #warning "Both SPI and EEPROM flashes are defined"
#endif 

/************** AN-B-001 based image /fast bootloader section **************/
// Define it if
// if AN-B-001 based image is burned in SPI flash and the bootloader is running from the OTP
#undef SUPPORT_AN_B_001
#define AN001_SPI_HEADER_SIZE               8
#define AN001_SPI_STARTCODE_POSITION        8
#define AN001_EEPROM_HEADER_SIZE            8
#define AN001_EEPROM_STARTCODE_POSITION     32

/************** Dual Image bootloader section**************/
 
// product header flags 
#define PRODUCT_HEADER_POSITION     0x1F000
#define PRODUCT_HEADER_SIGNATURE1   0x70
#define PRODUCT_HEADER_SIGNATURE2   0x52
// image header flags 
#define IMAGE_HEADER_SIGNATURE1     0x70
#define IMAGE_HEADER_SIGNATURE2     0x51
#define CODE_OFFSET                 64
#define STATUS_INVALID_IMAGE        0x0
#define STATUS_VALID_IMAGE          0xAA
#define STATUS_NEW_IMAGE            0xFF
//product header structure
typedef struct __productHeader {
    uint8 signature[2];
    uint8 version[2];
    uint32_t offset1;
    uint32_t offset2;
}s_productHeader;
//image header structure
typedef struct __imageHeader {
    uint8 signature[2];
    uint8 validflag;
    uint8 imageid;
    uint32_t code_size;
    uint32_t CRC;
    uint8 version[16];
    uint32_t timestamp;
    uint8 reserved[32];
}s_imageHeader;

// Functions Prototype
/**
****************************************************************************************
* @brief Load the active from a SPI flash
* @return Success (0) or Error Code.
* 
****************************************************************************************
**/
int spi_loadActiveImage(void);

/**
****************************************************************************************
* @brief Load the active from an EEPROM flash
* @return Success (0) or Error Code.
* 
****************************************************************************************
**/
int eeprom_loadActiveImage(void);

#ifdef CRC16_USED 
uint16_t gen_crc16(const uint8_t *data, uint16_t size);
#endif

#endif
