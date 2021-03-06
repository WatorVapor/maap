/*! ----------------------------------------------------------------------------
 * @file    port.h
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2015-2020 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */


#ifndef PORT_H_
#define PORT_H_

#include <Arduino.h>


#include <stdint.h>
#include <string.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <SPI.h>

/// spi 2
#define DW_PIN_SCK  PB13
#define DW_PIN_MISO PB14
#define DW_PIN_MOSI PB15
#define DW_PIN_NSS  PB12

#define DW_IRQn_Pin PC14
#define DW_WAKEUP_Pin PC15
#define DW_RESET_Pin PB2

#define SPI_CHANNEL 0
#define SPI_CLOCK_SPEED_FAST 18* 1000 * 1000
//#define SPI_CLOCK_SPEED_FAST    4500 * 1000
#define SPI_CLOCK_SPEED_SLOW    4500 *  1000

#define UNUSED(x) (void)(x)

//extern PlatformMutex  dwt_lock;
extern SPISettings dwt_spi_setting;
extern SPIClass dwt_spi;


#ifdef __cplusplus
extern "C" {
#endif

/* DW IC IRQ (EXTI15_10_IRQ) handler type. */
typedef void (*port_dwic_isr_t)(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn port_set_DWIC_isr()
 *
 * @brief This function is used to install the handling function for DW1000 IRQ.
 *
 * NOTE:
 *   - As EXTI9_5_IRQHandler does not check that port_deca_isr is not null, the user application must ensure that a
 *     proper handler is set by calling this function before any DW1000 IRQ occurs!
 *   - This function makes sure the DW1000 IRQ line is deactivated while the handler is installed.
 *
 * @param deca_isr function pointer to DW1000 interrupt handler to install
 *
 * @return none
 */
void port_set_dwic_isr(port_dwic_isr_t isr);

/*****************************************************************************************************************//*
**/

 /****************************************************************************//**
  *
  *                                 Types definitions
  *
  *******************************************************************************/

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

/****************************************************************************//**
 *
 *                              MACRO
 *
 *******************************************************************************/

/****************************************************************************//**
 *
 *                              MACRO function
 *
 *******************************************************************************/

/****************************************************************************//**
 *
 *                              port function prototypes
 *
 *******************************************************************************/

void Sleep(uint32_t Delay);

void port_set_dw_ic_spi_slowrate(void);
void port_set_dw_ic_spi_fastrate(void);

void process_dwRSTn_irq(void);
void process_deca_irq(void);

int  peripherals_init(void);
void spi_peripheral_init(void);

void setup_DWICRSTnIRQ(int enable);

void reset_DWIC(void);

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
void wakeup_device_with_io(void);

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
void make_very_short_wakeup_io(void);

#ifdef __cplusplus
}
#endif

//This set the IO for waking up the chip
#define SET_WAKEUP_PIN_IO_LOW     digitalWrite(DW_WAKEUP_Pin, LOW);
#define SET_WAKEUP_PIN_IO_HIGH    digitalWrite(DW_WAKEUP_Pin, HIGH);

//#define WAIT_500uSEC    Sleep(1)/*This is should be a delay of 500uSec at least. In our example it is more than that*/

#define WAIT_500uSEC    {usleep(500);}



#endif /* PORT_H_ */
