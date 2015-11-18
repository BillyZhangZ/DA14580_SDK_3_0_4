/**
****************************************************************************************
*
* @file rwip.c
*
* @brief RW IP SW main module
*
* Copyright (C) RivieraWaves 2009-2013
*
*
****************************************************************************************
*/

/**
 ****************************************************************************************
 * @addtogroup RW IP SW main module
 * @ingroup ROOT
 * @brief The RW IP SW main module.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"    // RW SW configuration

#include <string.h>         // for mem* functions
#include "co_version.h"
#include "rwip.h"           // RW definitions
#include "arch.h"           // Platform architecture definition
#include "arch_sleep.h"

#if (NVDS_SUPPORT)
#include "nvds.h"           // NVDS definitions
#endif // NVDS_SUPPORT

#if (BT_EMB_PRESENT)
#include "rwbt.h"           // rwbt definitions
#endif //BT_EMB_PRESENT

#if (BLE_EMB_PRESENT)
#include "rwble.h"          // rwble definitions
#endif //BLE_EMB_PRESENT

#if (BLE_HOST_PRESENT)
#include "rwble_hl.h"       // BLE HL definitions
#include "gapc.h"
#include "smpc.h"
#include "gattc.h"
#include "attc.h"
#include "atts.h"
#include "l2cc.h"
#endif //BLE_HOST_PRESENT

#if (BLE_APP_PRESENT)
#include "app.h"            // Application definitions
#endif //BLE_APP_PRESENT

#if (DEEP_SLEEP)
#if (BT_EMB_PRESENT)
#include "ld_sleep.h"       // definitions for sleep mode
#endif //BT_EMB_PRESENT
#if (BLE_EMB_PRESENT)
#include "lld_sleep.h"      // definitions for sleep mode
#endif //BLE_EMB_PRESENT
#include "led.h"            // led definitions
#endif //DEEP_SLEEP

#if (BLE_EMB_PRESENT)
#include "llc.h"
#endif //BLE_EMB_PRESENT

#if (BT_EMB_PRESENT || BLE_EMB_PRESENT)
#include "plf.h"            // platform definition
#include "rf.h"             // RF definitions
#endif //BT_EMB_PRESENT || BLE_EMB_PRESENT

#if (GTL_ITF)
#include "gtl.h"
#endif //GTL_ITF

#if (HCIC_ITF)
#if (BT_EMB_PRESENT)
#include "hci.h"            // HCI definition
#elif (BLE_EMB_PRESENT)
#include "hcic.h"           // HCI definition
#endif //BT_EMB_PRESENT / BLE_EMB_PRESENT
#endif //HCIC_ITF

#if (KE_SUPPORT)
#include "ke.h"             // kernel definition
#include "ke_event.h"       // kernel event
#include "ke_timer.h"       // definitions for timer
#include "ke_mem.h"         // kernel memory manager
#endif //KE_SUPPORT

#include "dbg.h"            // debug definition

#if (BT_EMB_PRESENT)
#include "reg_btcore.h"     // bt core registers
#endif //BT_EMB_PRESENT


#if ((BLE_APP_PRESENT) || ((BLE_HOST_PRESENT && (!GTL_ITF))))
#include "app.h"
#endif //BLE_APP_PRESENT

#include "em_map_ble_user.h"
#include "em_map_ble.h"
#include "reg_ble_em_rx.h"

extern uint32_t rcx_freq;
extern uint32_t lp_clk_sel;

extern bool sys_startup_flag      __attribute__((section("retention_mem_area0"),zero_init));
extern uint8_t func_check_mem_flag      __attribute__((section("retention_mem_area0"),zero_init));

void ble_regs_push(void);
void smpc_regs_push(void);

#if (DEEP_SLEEP)
static uint32_t rwip_slot_2_lpcycles(uint32_t slot_cnt)
{
    uint32_t lpcycles;

    // Sanity check: The number of slots should not be too high to avoid overflow
    ASSERT_ERR(slot_cnt < 1000000);

    #if HZ32000
    // Compute the low power clock cycles - case of a 32kHz clock
    lpcycles = slot_cnt * 20;
    #else //HZ32000
    // Compute the low power clock cycles - case of a 32.768kHz clock
    lpcycles = (slot_cnt << 11)/100;
    #endif //HZ32000
    
    return(lpcycles);
}

extern float rcx_slot_duration;

static uint32_t rwip_slot_2_lpcycles_rcx(uint32_t slot_cnt)
{
    volatile uint32_t lpcycles;

    // Sanity check: The number of slots should not be too high to avoid overflow
    ASSERT_ERR(slot_cnt < 1000000);

    lpcycles = (uint32_t)(slot_cnt * rcx_slot_duration);
    
    return(lpcycles);
}

#endif //DEEP_SLEEP

sleep_mode_t rwip_sleep(void)
{
	sleep_mode_t proc_sleep = mode_active;
    uint32_t twirq_set_value;
    uint32_t twext_value;
    #if (DEEP_SLEEP)
    uint32_t sleep_duration = jump_table_struct[max_sleep_duration_external_wakeup_pos];//MAX_SLEEP_DURATION_EXTERNAL_WAKEUP;
    #endif //DEEP_SLEEP

    DBG_SWDIAG(SLEEP, ALGO, 0);

#if (BLE_APP_PRESENT)
    if ( app_ble_ext_wakeup_get() || (rwip_env.ext_wakeup_enable == 2) )  // sleep forever!
        sleep_duration = 0;
#else
#if (!EXTERNAL_WAKEUP) // sleep_duration will remain as it was set above....
    if (rwip_env.ext_wakeup_enable == 2)
        sleep_duration = 0;
#endif		
#endif    
    
    do
    {
        /************************************************************************
         **************            CHECK STARTUP FLAG             **************
         ************************************************************************/
        // Check if system is in startup porcedure. Give some time to XTAL32 to settle before first sleep.
        if (lp_clk_sel == LP_CLK_XTAL32)
        {
            if (sys_startup_flag)
            {
                uint32_t current_time;
                current_time = lld_evt_time_get();
                
                if (current_time < 3200) // 2 seconds after startup to allow system to sleep
                    break;
                else // After 2 seconds system can sleep
                    sys_startup_flag = false;
            }
        }
        else
        {
            sys_startup_flag = false;
        }
        
        
        /************************************************************************
         **************            CHECK KERNEL EVENTS             **************
         ************************************************************************/
        // Check if some kernel processing is ongoing
        if (!ke_sleep_check())
            break;
        // Processor sleep can be enabled
        proc_sleep = mode_idle;

        DBG_SWDIAG(SLEEP, ALGO, 1);

#if (DEEP_SLEEP)
        /************************************************************************
         **************             CHECK ENABLE FLAG              **************
         ************************************************************************/
        // Check sleep enable flag
        if(!rwip_env.sleep_enable)
            break;

		/************************************************************************
		 **************           CHECK RADIO POWER DOWN           **************
		 ************************************************************************/
		// Check if BLE + Radio are still sleeping
		if(GetBits16(SYS_STAT_REG, RAD_IS_DOWN)) {//If BLE + Radio are in sleep return the appropriate mode for ARM
			proc_sleep = mode_sleeping;
			break;
		}
		
        /************************************************************************
         **************              CHECK RW FLAGS                **************
         ************************************************************************/
        // First check if no pending procedure prevent from going to sleep
        if (rwip_prevent_sleep_get() != 0)
            break;

        DBG_SWDIAG(SLEEP, ALGO, 2);

        /************************************************************************
         **************           CHECK EXT WAKEUP FLAG            **************
         ************************************************************************/
        /* If external wakeup enable, sleep duration can be set to maximum, otherwise
         *  system should be woken-up periodically to poll incoming packets from HCI */
        if(jump_table_struct[0] == TASK_GTL || (BLE_INTEGRATED_HOST_GTL == 1 )) // No need for periodic wakeup if we have full-hosted system
        {
            if(!rwip_env.ext_wakeup_enable)
                sleep_duration = jump_table_struct[max_sleep_duration_periodic_wakeup_pos];//MAX_SLEEP_DURATION_PERIODIC_WAKEUP;
        }

        /************************************************************************
         **************            CHECK KERNEL TIMERS             **************
         ************************************************************************/
        // Compute the duration up to the next software timer expires
        if (!ke_timer_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;

        DBG_SWDIAG(SLEEP, ALGO, 3);

        #if (BLE_EMB_PRESENT)
        /************************************************************************
         **************                 CHECK BLE                  **************
         ************************************************************************/
        // Compute the duration up to the next BLE event
        if (!lld_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;
        #endif // BLE_EMB_PRESENT
        
        DBG_SWDIAG(SLEEP, ALGO, 4);

        #if (BT_EMB_PRESENT)
        /************************************************************************
         **************                 CHECK BT                   **************
         ************************************************************************/
        // Compute the duration up to the next BT active slot
        if (!ld_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;
        #endif // BT_EMB_PRESENT

        DBG_SWDIAG(SLEEP, ALGO, 5);

        #if (HCIC_ITF)
        /************************************************************************
         **************                 CHECK HCI                  **************
         ************************************************************************/
		if((jump_table_struct[0] == TASK_GTL) || (BLE_INTEGRATED_HOST_GTL == 1 ))
		{       
            // Try to switch off HCI
            if (!hci_enter_sleep())
                break;
		}
        #endif // HCIC_ITF

        #if (GTL_ITF)
        /************************************************************************
         **************                 CHECK TL                   **************
         ************************************************************************/
        // Try to switch off Transport Layer
        if (!gtl_enter_sleep())
            break;
        #endif // GTL_ITF

        DBG_SWDIAG(SLEEP, ALGO, 6);

        /************************************************************************
         **************          PROGRAM CORE DEEP SLEEP           **************
         ************************************************************************/
          if (lp_clk_sel == LP_CLK_RCX20)
        {
            twirq_set_value = lld_sleep_us_2_lpcycles_sel_func(XTAL_TRIMMING_TIME_USEC);
            // TWEXT setting is ok for RXC freq up to 15,789KHz
            twext_value = TWEXT_VALUE_RCX;
            sleep_duration--;
            if (sleep_duration < rwip_env.wakeup_delay)
                break;
        }
        else // if (lp_clk_sel == LP_CLK_XTAL32)
        {
            twirq_set_value = XTAL_TRIMMING_TIME;
            twext_value = TWEXT_VALUE_XTAL32;
        }

       
        //Prepare BLE_ENBPRESET_REG for next sleep cycle
        SetBits32(BLE_ENBPRESET_REG, TWIRQ_RESET, TWIRQ_RESET_VALUE);   // TWIRQ_RESET
        SetBits32(BLE_ENBPRESET_REG, TWIRQ_SET, twirq_set_value);       // TWIRQ_SET
        SetBits32(BLE_ENBPRESET_REG, TWEXT, twext_value);               // TWEXT

        //Everything ready for sleep!
        
		proc_sleep = mode_sleeping;

        #if (BT_EMB_PRESENT)
        // Put BT core into deep sleep
        ld_sleep_enter(rwip_slot_2_lpcycles(sleep_duration), rwip_env.ext_wakeup_enable);
        #elif (BLE_EMB_PRESENT)
        // Put BT core into deep sleep
        if (lp_clk_sel == LP_CLK_XTAL32)
            lld_sleep_enter(rwip_slot_2_lpcycles(sleep_duration), rwip_env.ext_wakeup_enable);
        else if (lp_clk_sel == LP_CLK_RCX20)
            lld_sleep_enter(rwip_slot_2_lpcycles_rcx(sleep_duration), rwip_env.ext_wakeup_enable);
        #endif //BT_EMB_PRESENT / BT_EMB_PRESENT
        

        DBG_SWDIAG(SLEEP, SLEEP, 1);

        /************************************************************************
         **************               SWITCH OFF RF                **************
         ************************************************************************/        
        rwip_rf.sleep();
        
        while(!ble_deep_sleep_stat_getf());

        ble_regs_push();    // push the ble ret.vars to retention memory
        smpc_regs_push();   // push smpc ble ret.vars to retention memory
        
		//check and wait till you may disable the radio. 32.768KHz XTAL must be running!
        //(debug note: use BLE_CNTL2_REG:MON_LP_CLK bit to check (write 0, should be set to 1 by the BLE))
		while ( !(GetWord32(BLE_CNTL2_REG) & RADIO_PWRDN_ALLOW) );
		//BLE CLK must be off when BLE_DEEPSLCNTL_REG:DEEP_SLEEP_ON is set
		SetBits16(CLK_RADIO_REG, BLE_ENABLE, 0);   												

        #endif // DEEP_SLEEP
    } while(0);

    return proc_sleep;
}

