/**
 ****************************************************************************************
 *
 * @file bootloader.c
 *
 * @brief bootloader application
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

#include <stdio.h>
#include "global_io.h"
#include "core_cm0.h"
#include "periph_setup.h"
#include "uart_booter.h"
#include "i2c_eeprom.h"
#include "bootloader.h"
#include "spi_commands.h"
#include "spi_flash.h"
#include "gpio.h"
#include "uart.h"

/**
****************************************************************************************
* @brief Read a block of data from a nonvolatile memory
* @param[in] destination_buffer: buffer to put the data
* @param[in] source_addr: starting position to read the data from spi
* @param[in] len: size of data to read  
****************************************************************************************
*/ 
static int FlashRead(unsigned long destination_buffer,unsigned long source_addr,unsigned long len)
{
#ifdef SPI_FLASH_SUPPORTED
    spi_flash_read_data((uint8_t *)destination_buffer, source_addr,len);
    return 0;
#else
    return i2c_eeprom_read_data((uint8_t *)destination_buffer, (unsigned long) source_addr, (unsigned long ) len);
#endif
}

/**
 ****************************************************************************************
 * @brief Return the bank index of the active (latest and valid) image 
 * @param[in] destination_buffer: buffer to put the data
 * @param[in] source_addr: starting position to read the data from spi
 * @param[in] len: size of data to read  
 ****************************************************************************************
 */
static uint8 findlatest(uint8 id1, uint8 id2)
{
    if (id1==0xFF && id2==0) return 2;
    if (id2==0xFF && id1==0) return 1;
    if (id1>id2) return 1;
    else return 2;
}  
extern uint32_t crc32(uint32_t crc, const void *buf, size_t size);

/**
 ****************************************************************************************
 * @brief Load the active (latest and valid) image from a non-volatile memory
 * @return Success (0) or Error Code.
 * 
 ****************************************************************************************
 */
static int loadActiveImage(void)
{
    uint32 codesize1=0;
    uint32 codesize2=0;
    uint8 spi_data_buffer[64];
    s_productHeader *pProductHeader;
    s_imageHeader *pImageHeader;
    uint32 imageposition1;
    uint32 imageposition2;
    uint8 activeImage=0;
    uint8 images_status=0;
    uint8 imageid1=0;
    uint8 imageid2=0;
    uint32 crc_image1=0;
    uint32 crc_image2=0;

    // read product header
    pProductHeader = (s_productHeader*)spi_data_buffer;	
    FlashRead((unsigned long )pProductHeader,(unsigned long)PRODUCT_HEADER_POSITION, (unsigned long)sizeof(s_productHeader));
    // verify product header
    if (pProductHeader->signature[0]!=PRODUCT_HEADER_SIGNATURE1 || pProductHeader->signature[1]!=PRODUCT_HEADER_SIGNATURE2) 
        return -1;
  
    imageposition1 = pProductHeader->offset1; 
    imageposition2 = pProductHeader->offset2; 
     
    pImageHeader = (s_imageHeader*)spi_data_buffer;	
    // read image header 1, image id must be stored for the bank selection
    FlashRead((unsigned long )pImageHeader,(unsigned long)imageposition1, (unsigned long)sizeof(s_imageHeader));
     
    if ((pImageHeader->validflag==STATUS_VALID_IMAGE || pImageHeader->validflag==STATUS_NEW_IMAGE) && pImageHeader->signature[0]==IMAGE_HEADER_SIGNATURE1 && pImageHeader->signature[1]==IMAGE_HEADER_SIGNATURE2) 
    {
        codesize1 = pImageHeader->code_size;
        imageid1 = pImageHeader->imageid;
        images_status = 1;
        crc_image1 = (pImageHeader->CRC);
    } 
    pImageHeader = (s_imageHeader*)spi_data_buffer;
    // read image header 2, image id must be stored for the bank selection
    FlashRead((unsigned long )pImageHeader,(unsigned long)imageposition2, (unsigned long)sizeof(s_imageHeader));
    if ((pImageHeader->validflag==STATUS_VALID_IMAGE || pImageHeader->validflag==STATUS_NEW_IMAGE) && pImageHeader->signature[0]==IMAGE_HEADER_SIGNATURE1 && pImageHeader->signature[1]==IMAGE_HEADER_SIGNATURE2) 
    {
        
        imageid2=pImageHeader->imageid;
        codesize2=pImageHeader->code_size;
        crc_image2 = (pImageHeader->CRC);
        images_status+=2;
    }
    if (images_status==3) 
        activeImage=findlatest(imageid1,imageid2);
    else
        activeImage = images_status;

    if (activeImage==1) {
        FlashRead(SYSRAM_BASE_ADDRESS, (unsigned long) imageposition1+CODE_OFFSET, (unsigned long)  codesize1);	
        if (crc_image1!=crc32(0, (uint8_t*)SYSRAM_BASE_ADDRESS, codesize1))
        {
            if (images_status==3)
            {
                FlashRead(SYSRAM_BASE_ADDRESS, (unsigned long) imageposition2+CODE_OFFSET, (unsigned long)  codesize2);
                 if (crc_image2 != crc32(0, (uint8_t*)SYSRAM_BASE_ADDRESS, codesize2))
                        return -1;
            }
        }
    } else if (activeImage==2) {
        FlashRead(SYSRAM_BASE_ADDRESS, (unsigned long) imageposition2+CODE_OFFSET, (unsigned long)  codesize2);	
        if (crc_image2 !=crc32(0,(uint8_t*)SYSRAM_BASE_ADDRESS, codesize2))
        {
            if (images_status==3)
            {
                FlashRead(SYSRAM_BASE_ADDRESS, (unsigned long) imageposition1+CODE_OFFSET, (unsigned long)  codesize1);	
                if ( crc_image1 !=crc32(0,(uint8_t*)SYSRAM_BASE_ADDRESS, codesize1))
                        return -1;
            }
        }
    }
  
    return 0;
}

/**
****************************************************************************************
* @brief set gpio port function mode for EEPROM interface
* 
****************************************************************************************
**/
 void set_pad_eeprom(void)       
{
    GPIO_ConfigurePin(I2C_GPIO_PORT, I2C_SCL_PIN, INPUT, PID_I2C_SCL, false);
    GPIO_ConfigurePin(I2C_GPIO_PORT, I2C_SDA_PIN, INPUT, PID_I2C_SDA, false);
} 

/**
****************************************************************************************
* @brief Load the active from a SPI flash
* @return Success (0) or Error Code.
* 
****************************************************************************************
**/
int spi_loadActiveImage(void)
{
    //Initiate the SPI interface
    spi_flash_peripheral_init();
#ifdef SUPPORT_AN_B_001
    uint8 AN001Header[AN001_SPI_HEADER_SIZE];
    // check for AN-B-001 header
    SpiFlashRead((unsigned long)&AN001Header, (unsigned long) 0, (unsigned long)AN001_SPI_HEADER_SIZE);
    if (AN001Header[0]==0x70 && AN001Header[1]==0x50) { //it must be disabled if the bootloader runs from the SPI flash
        SpiFlashRead(SYSRAM_BASE_ADDRESS, (unsigned long)AN001_SPI_STARTCODE_POSITION, (unsigned long)  (AN001Header[6]<<8 | AN001Header[7]));
        return 0;
    } else return -1;
#else 
    spi_flash_size_init();
    return loadActiveImage();
#endif
} 

/**
****************************************************************************************
* @brief Load the active from an EEPROM flash
* @return Success (0) or Error Code.
* 
****************************************************************************************
**/
int eeprom_loadActiveImage(void)
{
    //Initiate the i2C/EEPROM flash & interface
    set_pad_eeprom();
    i2c_eeprom_init(I2C_SLAVE_ADDRESS, I2C_SPEED_MODE, I2C_ADDRESS_MODE, I2C_2BYTES_ADDR);
#ifdef SUPPORT_AN_B_001
    int i;
    uint8 AN001Header[AN001_EEPROM_HEADER_SIZE];
    char *code;
    int current;
    char crc_code;
    
    // check for AN-B-001 header
    FlashRead((unsigned long)&AN001Header, (unsigned long) 0, (unsigned long)AN001_EEPROM_HEADER_SIZE);
    if (AN001Header[0]==0x70 && AN001Header[1]==0x50) { //it must be disabled if the bootloader runs from the SPI flash
        codesize= (AN001Header[2]<<8 | AN001Header[3]);
        i2c_eeprom_read_data((uint8_t *)SYSRAM_BASE_ADDRESS, (unsigned long) AN001_EEPROM_STARTCODE_POSITION, (unsigned long ) codesize);
        crc_code=0;
        current=0;
        
        code= (char*)SYSRAM_BASE_ADDRESS;
        do{
            current+=32;               // set to next page (32bytes per page)
            for(i=0;i<32;i++) 
                    crc_code^=code[i]; // update CRC
            code+=32;
        }while(current<codesize);      // check if read all code
        if (crc_code==AN001Header[4]) 
            return 0;
    }
    return -1;
#else
    return loadActiveImage();
#endif
}

