/**
 ****************************************************************************************
 *
 * @file spi_hci.c
 *
 * @brief functions for exchange  of hci messages over spi. V2_0
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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "app.h"
#include "global_io.h"
#include "gpio.h"
#include "spi.h"
#include "spi_hci_msg.h"
#include "queue.h"
#include <core_cm0.h>
#include "periph_setup.h"


/**
****************************************************************************************
* @brief Enables interrupt for DREADY GPIO
****************************************************************************************
*/
void dready_irq_enable(void)
{
	SetWord16(GPIO_IRQ0_IN_SEL_REG,SPI_DREADY_IRQ);    // Assign SPI_DREADY_IRQ pin to GPIO0 interrupt
	SetWord16(GPIO_INT_LEVEL_CTRL_REG, 0x0022);        // Config interrupt as Edge IRQ, high pulse
 	NVIC_EnableIRQ(GPIO0_IRQn);                        // Enable spi interrupt, if MINT is '1'	
}
/**
****************************************************************************************
* @brief Receive an HCI message over the SPI
* @param[in] *msg_ptr: 	pointer to the position the received data will be stored
* @return Message type
****************************************************************************************
*/
uint16_t spi_receive_hci_msg(uint8_t *msg_ptr)
{
	uint16_t i, size, msg_type;
	
	uint8_t * rd_ptr;
	unsigned char *msg;
	
	rd_ptr = msg_ptr+2;    // Discard 0x0500;

	spi_cs_high();			// Close CS	
	spi_cs_low();				// Open CS
	
	spi_access(DREADY_ACK);    // Write DREADY acknowledge
	
	*msg_ptr++ = spi_access(0x00);
	
	if (*(msg_ptr-1) == 0x05)				// HCI Message
  {
		msg_ptr++;										// Align 16-bit
		for (i=0;i<6;i++)
		{
      *msg_ptr++ = spi_access(0x00);
		}
		*msg_ptr = spi_access(0x00);
		size = *msg_ptr++;
		*msg_ptr = spi_access(0x00);
		size += *msg_ptr++<<8;
		for (i=0; i<size; i++)
    {
			*msg_ptr++ = spi_access(0x00);
		}
	  
		msg = malloc(size+8);
		memcpy(msg,rd_ptr,size+8);
		EnQueue(&SPIRxQueue, msg, size+8);
		app_env.size_rx_queue++;
		msg_type = 1;											// GTL Message
	}
	else if (*(msg_ptr-1) == FLOW_ON_BYTE)		// Flow ON
  {
		app_env.slave_on_sleep = SLAVE_ACTIVE;
		msg_type = 2;										// Flow Message
	}
	else if (*(msg_ptr-1) == FLOW_OFF_BYTE)			// Flow OFF
  {
		app_env.slave_on_sleep = SLAVE_NOT_ACTIVE;
		msg_type = 3;										// Flow Message
  }
	else
  {
		msg_type = 0;										// Error	
	}
		
	spi_cs_high();                    // Close CS

	return msg_type;
}


/**
****************************************************************************************
* @brief Send an HCI message over the SPI
* @param[in] size: 			size of data to send in bytes
* @param[in] *msg_ptr: 	pointer to the first byte to be sent

****************************************************************************************
*/
void spi_send_hci_msg(uint16_t size, uint8_t *msg_ptr)
{
	uint16_t i;	

	NVIC_DisableIRQ(GPIO0_IRQn);
	
	while(GetBits16(SPI_DATA_REG,1<<SPI_DREADY_PIN)==1);  // Polling DREADY to detect if data is being received
	
	spi_cs_high();  // Close CS	
	spi_cs_low();   // Open CS
	
	spi_access(0x05);

	for (i=0; i<size; i++)
  {
		spi_access(*msg_ptr++);
	}
	
	spi_cs_high();  // Close CS
	
	NVIC_EnableIRQ(GPIO0_IRQn);	
}


