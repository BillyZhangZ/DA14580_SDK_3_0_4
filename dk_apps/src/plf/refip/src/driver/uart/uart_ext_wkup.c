/**
 ****************************************************************************************
 *
 * @file uart.c
 *
 * @brief UART driver
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: 6880 $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup UART
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stddef.h>     // standard definition
#include "timer.h"      // timer definition
#include "uart.h"       // uart definition
#include "reg_uart.h"   // uart register
#include "hcic_eif.h"   // hci uart definition
#include "global_io.h"

#include "rwble_config.h"
#include "periph_setup.h"

#include "gpio.h"

///*
// * DEFINES
// *****************************************************************************************
// */

///*
// * ENUMERATION DEFINITIONS
// *****************************************************************************************
// */
///// INT_PRIO values
//#if 0 //@WIK, commented because this is not in line with our IIR bitfields
//enum UART_INTPRIO
//{
//    UART_INTPRIO_MODEMSTAT     = 0,
//    UART_INTPRIO_ALLDATASENT   = 1,
//    UART_INTPRIO_DATAAVAILABLE = 2,
//    UART_INTPRIO_LINESTAT      = 3,
//    UART_INTPRIO_TIMEOUT       = 6
//};
//#endif

////@WIK, added next enum
//enum UART_ID
//{
//    MODEM_STAT         = 0,
//    NO_INT_PEND        = 1,
//    THR_EMPTY          = 2,
//    RECEIVED_AVAILABLE = 4,    
//    UART_TIMEOUT       = 12
//};

///// RX_LVL values
//enum UART_RXLVL
//{
//    UART_RXLVL_1,
//    UART_RXLVL_4,
//    UART_RXLVL_8,
//    UART_RXLVL_14
//};

///// WORD_LEN values
//enum UART_WORDLEN
//{
//    UART_WORDLEN_5,
//    UART_WORDLEN_6,
//    UART_WORDLEN_7,
//    UART_WORDLEN_8
//};

///// FIFO_SZ values
//enum UART_FIFOSIZE
//{
//    UART_FIFOSIZE_16,
//    UART_FIFOSIZE_32,
//    UART_FIFOSIZE_64,
//    UART_FIFOSIZE_128
//};

#ifdef CFG_WKUP_EXT_PROCESSOR
/*
 * STRUCT DEFINITIONS
 *****************************************************************************************
 */
/* TX and RX channel class holding data used for asynchronous read and write data
 * transactions
 */
/// UART TX RX Channel
struct uart_txrxchannel
{
    /// size
    uint32_t  size;
    /// buffer pointer
    uint8_t  *bufptr;
    /// call back function pointer
    void (*callback) (uint8_t);
};

/// UART environment structure
struct uart_env_tag
{
    /// tx channel
    struct uart_txrxchannel tx;
    /// rx channel
    struct uart_txrxchannel rx;
    /// error detect
    uint8_t errordetect;
};


extern struct uart_env_tag uart_env;

void uart_finish_transfers_ext_wkup_func(void)
{
    // Configure modem (HW flow control disable, 'RTS flow off')
    uart_mcr_pack(UART_ENABLE,     // extfunc
                  UART_DISABLE,    // autorts
                  UART_ENABLE,     // autocts
                  UART_DISABLE);   // rts

    // Wait TX FIFO empty
    while(!uart_thre_getf() || !uart_temt_getf());
	
	GPIO_SetInactive( EXT_WAKEUP_PORT, EXT_WAKEUP_PIN);
	
}

static void (*wrapped_callback) (uint8_t);

static void callback_proxy (uint8_t status)
{
    if (wrapped_callback)
		{
			wrapped_callback (status);
		}
	  GPIO_SetInactive( EXT_WAKEUP_PORT, EXT_WAKEUP_PIN);
}

void uart_write_ext_wkup_func(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t))
{
    // Sanity check
    ASSERT_ERR(bufptr != NULL);
    ASSERT_ERR(size != 0);
    ASSERT_ERR(uart_env.tx.bufptr == NULL);


	GPIO_SetActive( EXT_WAKEUP_PORT, EXT_WAKEUP_PIN);
	
    wrapped_callback = callback;
		
    // Prepare TX parameters
    uart_env.tx.size = size;
    uart_env.tx.bufptr = bufptr;
	uart_env.tx.callback = callback_proxy; // callback; 


    /* start data transaction
     * first isr execution is done without interrupt generation to reduce
     * interrupt load
     */
    uart_thr_empty_isr();
    if (uart_env.tx.bufptr != NULL)
    {
        uart_thr_empty_setf(1);
    }
}

#endif //CFG_WKUP_EXT_PROCESSOR

/// @} UART
