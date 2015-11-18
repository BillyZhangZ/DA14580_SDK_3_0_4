 /**
 ****************************************************************************************
 *
 * @file periph_setup.h
 *
 * @brief Peripheral Setup file. GPIO assignment, SPI, SPI Flash, I2C, EEPROM configuration settings, Example selection
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
#include "global_io.h"
#include "gpio.h"
#include "i2c_eeprom.h"
#include "spi.h"

#undef DEVELOPMENT_MODE
 
// Select EEPROM characteristics
#define I2C_EEPROM_SIZE   0x20000         // EEPROM size in bytes
#define I2C_EEPROM_PAGE   256             // EEPROM's page size in bytes
#define I2C_SLAVE_ADDRESS 0x50            // Set slave device address
#define I2C_SPEED_MODE    I2C_FAST        // 1: standard mode (100 kbits/s), 2: fast mode (400 kbits/s)
#define I2C_ADDRESS_MODE  I2C_7BIT_ADDR   // 0: 7-bit addressing, 1: 10-bit addressing
#define I2C_ADDRESS_SIZE  I2C_2BYTES_ADDR // 0: 8-bit memory address, 1: 16-bit memory address, 3: 24-bit memory address

// SPI Flash Manufacturer and ID
#define W25X10CL_MANF_DEV_ID (0xEF10)     // W25X10CL Manufacturer and ID
#define W25X20CL_MANF_DEV_ID (0xEF11)     // W25X10CL Manufacturer and ID

// SPI Flash options
#define W25X10CL_SIZE 131072              // SPI Flash memory size in bytes
#define W25X20CL_SIZE 262144              // SPI Flash memory size in bytes
#define W25X10CL_PAGE 256                 // SPI Flash memory page size in bytes
#define W25X20CL_PAGE 256                 // SPI Flash memory page size in bytes

#define SPI_FLASH_DEFAULT_SIZE 131072     // SPI Flash memory size in bytes
#define SPI_FLASH_DEFAULT_PAGE 256        // SPI Flash memory page size in bytes

//SPI initialization parameters
#define SPI_WORD_MODE  SPI_8BIT_MODE      // 0: 8-bit, 1: 16-bit, 2: 32-bit, 3: 9-bit
#define SPI_SMN_MODE   SPI_MASTER_MODE    // 0: Master mode, 1: Slave mode
#define SPI_POL_MODE   SPI_CLK_INIT_HIGH  // 0: SPI clk initially low, 1: SPI clk initially high
#define SPI_PHA_MODE   SPI_PHASE_1        // If same with SPI_POL_MODE, data are valid on clk high edge, else on low
#define SPI_MINT_EN    SPI_NO_MINT        // (SPI interrupt to the ICU) 0: Disabled, 1: Enabled
#define SPI_CLK_DIV    SPI_XTAL_DIV_2     // (SPI clock divider) 0: 8, 1: 4, 2: 2, 3: 14
// UART
#define UART_PIN_PAIR   4 //(0:(P0_0 , P0_1),2:(P0_2 , P0_3),4:(P0_4 , P0_5),6:(P0_6 , P0_7))  
#define UART_GPIO_PORT  GPIO_PORT_0
#define UART_TX_PIN     GPIO_PIN_4
#define UART_RX_PIN     GPIO_PIN_5
// SPI
#define SPI_GPIO_PORT  	GPIO_PORT_0
#define SPI_CS_PIN     	GPIO_PIN_3                           
#define SPI_CLK_PIN    	GPIO_PIN_0                
#define SPI_DO_PIN     	GPIO_PIN_6                 
#define SPI_DI_PIN     	GPIO_PIN_5  
// EEPROM 
#define I2C_GPIO_PORT   GPIO_PORT_0
#define I2C_SCL_PIN     GPIO_PIN_2
#define I2C_SDA_PIN     GPIO_PIN_3

