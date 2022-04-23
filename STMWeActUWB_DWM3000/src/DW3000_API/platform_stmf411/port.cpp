/*! ----------------------------------------------------------------------------
 * @file    port.c
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2016-2020 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include "port.h"
#include <SPI.h>
#include "debug.hpp"

/****************************************************************************//**
 *
 *                              APP global variables
 *
 *******************************************************************************/
//PlatformMutex  dwt_lock;
SPISettings dwt_spi_setting;
//SPIClass spi5(PIN_MOSI,PIN_MISO,PIN_SCK,PIN_CS);

/****************************************************************************//**
 *
 *                  Port private variables and function prototypes
 *
 *******************************************************************************/


/****************************************************************************//**
 *
 *                              Time section
 *
 *******************************************************************************/

/* @fn    usleep
 * @brief precise usleep() delay
 * */
#pragma GCC optimize ("O0")
int usleep(unsigned long usec)
{
    delayMicroseconds(usec);
    return 0;
}


/* @fn    Sleep
 * @brief Sleep delay in ms using SysTick timer
 * */
void
Sleep(uint32_t x)
{
    delay(x);
}

/****************************************************************************//**
 *
 *                              END OF Time section
 *
 *******************************************************************************/

/****************************************************************************//**
 *
 *                              Configuration section
 *
 *******************************************************************************/

/* @fn    peripherals_init
 * */
int peripherals_init (void)
{
    DUMP_VAR_I(PIN_SPI_SS);
    DUMP_VAR_I(DW_RESET_Pin);
    DUMP_VAR_I(DW_WAKEUP_Pin);
    DUMP_VAR_I(DW_IRQn_Pin);

    /* All has been initialized in the CubeMx code, see main.c */

    pinMode(PIN_SPI_SS, OUTPUT);    
    pinMode(DW_RESET_Pin, INPUT);
    pinMode(DW_WAKEUP_Pin, OUTPUT);
    digitalWrite(DW_WAKEUP_Pin,LOW);
    pinMode(DW_IRQn_Pin, INPUT);
    return 0;
}


/* @fn    spi_peripheral_init
 * */
void spi_peripheral_init()
{
    dwt_spi_setting = SPISettings(SPI_CLOCK_SPEED_FAST, MSBFIRST, SPI_MODE0);
    SPI.begin(PIN_SPI_SS);
    SPI.setDataMode(SPI_MODE0);
}


/****************************************************************************//**
 *
 *                          End of configuration section
 *
 *******************************************************************************/

/****************************************************************************//**
 *
 *                          DW IC port section
 *
 *******************************************************************************/

/* @fn      reset_DW IC
 * @brief   DW_RESET pin on DW IC has 2 functions
 *          In general it is output, but it also can be used to reset the digital
 *          part of DW IC by driving this pin low.
 *          Note, the DW_RESET pin should not be driven high externally.
 * */
void reset_DWIC(void)
{
    /* Configure Reset GPIO as output */
    pinMode(DW_RESET_Pin, OUTPUT);
    digitalWrite(DW_RESET_Pin, LOW);

    delay(2);

    /* Configure Reset GPIO as input */
    pinMode(DW_RESET_Pin, INPUT_FLOATING);

    delay(10);
    Sleep(2);

    DUMP_VAR_I(DW_RESET_Pin);
    DUMP_VAR_I(DW_WAKEUP_Pin);
    DUMP_VAR_I(DW_IRQn_Pin);

}

/* @fn      setup_DWICRSTnIRQ
 * @brief   setup the DW_RESET pin mode
 *          0 - output Open collector mode
 *          !0 - input mode with connected EXTI0 IRQ
 * */
void setup_DWICRSTnIRQ(int enable)
{
    UNUSED(enable);
}

/*! ------------------------------------------------------------------------------------------------------------------
* @fn wakeup_device_with_io()
*
* @brief This function wakes up the device by toggling io with a delay.
*
* input None
*
* output -None
*
*/
void wakeup_device_with_io(void)
{
    DUMP_VAR_I(DW_RESET_Pin);
    SET_WAKEUP_PIN_IO_HIGH;
    WAIT_500uSEC;
    SET_WAKEUP_PIN_IO_LOW;
    DUMP_VAR_I(DW_RESET_Pin);
}

/*! ------------------------------------------------------------------------------------------------------------------
* @fn make_very_short_wakeup_io()
*
* @brief This will toggle the wakeup pin for a very short time. The device should not wakeup
*
* input None
*
* output -None
*
*/
void make_very_short_wakeup_io(void)
{
    uint8_t   cnt;

    DUMP_VAR_I(DW_RESET_Pin);
    SET_WAKEUP_PIN_IO_HIGH;
    for (cnt=0;cnt<10;cnt++)
    ;

    SET_WAKEUP_PIN_IO_LOW;
    DUMP_VAR_I(DW_RESET_Pin);
}


/* @fn      port_set_dw_ic_spi_slowrate
 * @brief   set 4.5MHz
 *          note: hspi1 is clocked from 72MHz
 * */
void port_set_dw_ic_spi_slowrate(void)
{
    dwt_spi_setting = SPISettings(SPI_CLOCK_SPEED_SLOW, MSBFIRST, SPI_MODE0);    
    DUMP_VAR_I(SPI_CLOCK_SPEED_SLOW);
    Serial.print("SPI communication successfully setup.\n");
}

/* @fn      port_set_dw_ic_spi_fastrate
 * @brief   set 18MHz
 *          note: hspi1 is clocked from 72MHz
 * */
void port_set_dw_ic_spi_fastrate(void)
{
    /*
     * Fast rates are not available at present.
     */
    dwt_spi_setting = SPISettings(SPI_CLOCK_SPEED_FAST, MSBFIRST, SPI_MODE0);    
    DUMP_VAR_I(SPI_CLOCK_SPEED_FAST);
    Serial.print("SPI communication successfully setup.\n");
}

/****************************************************************************//**
 *
 *                          End APP port section
 *
 *******************************************************************************/



/****************************************************************************//**
 *
 *                              IRQ section
 *
 *******************************************************************************/
/* @fn      process_deca_irq
 * @brief   main call-back for processinfg of DW3000 IRQ
 *          it re-enters the IRQ routing and processes all events.
 *          After processing of all events, DW3000 will clear the IRQ line.
 * */
__INLINE void process_deca_irq(void)
{
}

/****************************************************************************//**
 *
 *                              END OF IRQ section
 *
 *******************************************************************************/



/****************************************************************************//**
 *
 *                              USB report section
 *
 *******************************************************************************/

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn port_set_dwic_isr()
 *
 * @brief This function is used to install the handling function for DW IC IRQ.
 *
 * NOTE:
 *   - The user application shall ensure that a proper handler is set by calling this function before any DW IC IRQ occurs.
 *   - This function deactivates the DW IC IRQ line while the handler is installed.
 *
 * @param deca_isr function pointer to DW IC interrupt handler to install
 *
 * @return none
 */
void port_set_dwic_isr(port_dwic_isr_t dwic_isr)
{
    //dwt_lock.lock();
    attachInterrupt(digitalPinToInterrupt(DW_IRQn_Pin), dwic_isr,RISING );
    if ( 0 )
    {
        fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
    }
    //dwt_lock.unlock();
}


/****************************************************************************//**
 *
 *                              END OF Report section
 *
 *******************************************************************************/


extern "C" void __dw_drivers_start(void ){
    DUMP_VAR_I(1);
}
extern "C" void __dw_drivers_end(void){
    DUMP_VAR_I(2);    
}
