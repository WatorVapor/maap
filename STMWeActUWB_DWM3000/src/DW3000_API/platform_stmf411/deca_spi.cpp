/*! ----------------------------------------------------------------------------
 * @file    deca_spi.c
 * @brief   SPI access functions
 *
 * @attention
 *
 * Copyright 2015-2020 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */


#include "deca_spi.h"
#include "deca_device_api.h"
#include "port.h"
#include "debug.hpp"

#include <stm32f4xx_hal_def.h>

#define SPI_CHANNEL 0
#define SPI_CLOCK_SPEED 32000000
#define CRC_SIZE 1

#define JUNK 0x0


int wiringPiSPIDataRW (int channel, uint8_t *data, int len);


int writeSpi (int channel, uint8_t *data, int len) ;
int readSpi (int channel, uint8_t *data, int len) ;


/****************************************************************************//**
 *
 *                              DW1000 SPI section
 *
 *******************************************************************************/
/*! ------------------------------------------------------------------------------------------------------------------
 * Function: openspi()
 *
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
int openspi(/*SPI_TypeDef* SPIx*/)
{
    return 0;
} // end openspi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
int closespi(void)
{
    return 0;
} // end closespi()




/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospiwithcrc()
 *
 * Low level abstract function to write to the SPI when SPI CRC mode is used
 * Takes two separate byte buffers for write header and write data, and a CRC8 byte which is written last
 * returns 0 for success, or -1 for error
 */
int writetospiwithcrc(
                uint16_t      headerLength,
                const uint8_t *headerBuffer,
                uint16_t      bodyLength,
                const uint8_t *bodyBuffer,
                uint8_t       crc8)
{
    TRACE_VAR_I(headerLength);
    TRACE_VAR_I(bodyLength);
    decaIrqStatus_t  stat ;
    stat = decamutexon() ;

    uint8_t buf[headerLength + bodyLength + CRC_SIZE];
    memcpy(buf, headerBuffer, headerLength);

    if(bodyLength != 0)
    {
        memcpy(&buf[headerLength], bodyBuffer, bodyLength);
        headerLength += bodyLength;
    }
    buf[headerLength] = crc8;
    writeSpi(SPI_CHANNEL, buf, (headerLength + CRC_SIZE));

    decamutexoff(stat);
    return 0;
} // end writetospiwithcrc()


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospi()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success, or -1 for error
 */
int writetospi(uint16_t       headerLength,
               const uint8_t  *headerBuffer,
               uint16_t       bodyLength,
               const uint8_t  *bodyBuffer)
{
    TRACE_VAR_I(headerLength);
    TRACE_VAR_I(bodyLength);

    decaIrqStatus_t  stat ;
    stat = decamutexon() ;

    uint8_t buf[headerLength + bodyLength];
    memcpy(buf, headerBuffer, headerLength);
    if(bodyLength != 0)
    {
        memcpy(&buf[headerLength], bodyBuffer, bodyLength);
        headerLength += bodyLength;
    }
    writeSpi(SPI_CHANNEL, buf, headerLength);

    decamutexoff(stat);
    return 0;
} // end writetospi()


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: readfromspi()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns -1 if there was an error
 */
//#pragma GCC optimize ("O3")
int readfromspi(uint16_t headerLength,
                uint8_t  *headerBuffer,
                uint16_t readlength,
                uint8_t  *readBuffer)
{
    TRACE_VAR_I(headerLength);
    TRACE_VAR_I(readlength);


#if 1
    decaIrqStatus_t  stat ;
    stat = decamutexon() ;
    TRACE_VAR_I(stat);
#endif
    for(int i = 0 ;i < headerLength; i++) {
        TRACE_VAR_I(headerBuffer[i]);
    }

#if 1
    dwt_spi.beginTransaction(dwt_spi_setting);
    digitalWrite(DW_PIN_NSS, LOW);
    //delayMicroseconds(5);
/*
    for(int i = 0 ;i < headerLength; i++) {
        //TRACE_VAR_I(headerBuffer[i]);
        dwt_spi.transfer(headerBuffer[i]);
    }
*/
    dwt_spi.transfer(headerBuffer,headerLength);

    delayMicroseconds(5);
    for(int i = 0 ;i < readlength; i++) {
        readBuffer[i] = dwt_spi.transfer(JUNK);
        //TRACE_VAR_I(readBuffer[i]);
    }
    //delayMicroseconds(5);
    digitalWrite(DW_PIN_NSS, HIGH);
    dwt_spi.endTransaction();
#endif

    for(int i = 0 ;i < readlength; i++) {
        TRACE_VAR_I(readBuffer[i]);
    }


#if 1
    decamutexoff(stat);
    TRACE_VAR_I(stat);
#endif

    return 0;
} // end readfromspi()


int writeSpi (int channel, uint8_t *data, int len) {
    digitalWrite(DW_PIN_NSS, LOW);
    dwt_spi.beginTransaction(dwt_spi_setting);
    for(int i = 0 ;i < len; i++) {
        dwt_spi.transfer(data[i]);
    }
    dwt_spi.endTransaction();
    digitalWrite(DW_PIN_NSS, HIGH);
    return 0;
}
/*
int readSpi (int channel, uint8_t *data, int len) {

}
*/


/****************************************************************************//**
 *
 *                              END OF DW1000 SPI section
 *
 *******************************************************************************/



#if 0

/*! ----------------------------------------------------------------------------
 * @file    deca_spi.c
 * @brief   SPI access functions
 *
 * @attention
 *
 * Copyright 2015 - 2021 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include <deca_device_api.h>
#include <deca_spi.h>
#include <port.h>
#include <stm32f4xx_hal_def.h>


extern SPI_HandleTypeDef    *hcurrent_active_spi;/*clocked from 72MHz*/
extern uint16_t             pin_io_active_spi;
extern GPIO_PinState        SPI_CS_state;

/****************************************************************************
 *
 *
 *                              DWxxx SPI section
 *
 *******************************************************************************/
/*! ------------------------------------------------------------------------------------------------------------------
 * Function: openspi()
 *
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
int openspi2(/*SPI_TypeDef* SPIx*/)
{
    return 0;
} // end openspi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
int closespi2(void)
{
    return 0;
} // end closespi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospiwithcrc()
 *
 * Low level abstract function to write to the SPI when SPI CRC mode is used
 * Takes two separate byte buffers for write header and write data, and a CRC8 byte which is written last
 * returns 0 for success, or -1 for error
 */
int writetospiwithcrc2(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer, uint8_t crc8)
{
#ifdef DWT_ENABLE_CRC
    decaIrqStatus_t stat;
    stat = decamutexon();
    while (HAL_SPI_GetState(hcurrent_active_spi) != HAL_SPI_STATE_READY);

    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, pin_io_active_spi, SPI_CS_state); /**< Put chip select line low */


    HAL_SPI_Transmit(hcurrent_active_spi, (uint8_t *)headerBuffer, headerLength, 10);    /* Send header in polling mode */
    HAL_SPI_Transmit(hcurrent_active_spi, (uint8_t *)bodyBuffer, bodyLength, 10);        /* Send data in polling mode */
    HAL_SPI_Transmit(hcurrent_active_spi, (uint8_t *)&crc8, 1, 10);      /* Send data in polling mode */

    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, pin_io_active_spi,(GPIO_PinState)(!((uint8_t)SPI_CS_state))); /**< Put chip select line high */

    decamutexoff(stat);
#endif //DWT_ENABLE_CRC
    return 0;
} // end writetospiwithcrc()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospi()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success, or -1 for error
 */
int writetospi2(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer)
{
    decaIrqStatus_t stat;
    stat = decamutexon();

    while (HAL_SPI_GetState(hcurrent_active_spi) != HAL_SPI_STATE_READY);

    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, pin_io_active_spi, SPI_CS_state); /**< Put chip select line low */

    HAL_SPI_Transmit(hcurrent_active_spi, (uint8_t *)headerBuffer, headerLength, HAL_MAX_DELAY); /* Send header in polling mode */

    if(bodyLength != 0)
    {
        HAL_SPI_Transmit(hcurrent_active_spi, (uint8_t *)bodyBuffer,   bodyLength, HAL_MAX_DELAY);     /* Send data in polling mode */
    }

    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, pin_io_active_spi, (GPIO_PinState)(!((uint8_t)SPI_CS_state))); /**< Put chip select line high */

    decamutexoff(stat);

    return 0;
} // end writetospi()

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn spi_cs_low_delay()
 *
 * @brief This function sets the CS to '0' for ms delay and than raises it up
 *
 * input parameters:
 * @param ms_delay - The delay for CS to be in '0' state
 *
 * no return value
 */
uint16_t spi_cs_low_delay(uint16_t delay_ms)
{
    /* Blocking: Check whether previous transfer has been finished */
    while (HAL_SPI_GetState(hcurrent_active_spi) != HAL_SPI_STATE_READY);
    /* Process Locked */
    __HAL_LOCK(hcurrent_active_spi);
    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, pin_io_active_spi, SPI_CS_state); /**< Put chip select line low */
    Sleep(delay_ms);
    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, pin_io_active_spi, (GPIO_PinState)(!((uint8_t)SPI_CS_state))); /**< Put chip select line high */
    /* Process Unlocked */
    __HAL_UNLOCK(hcurrent_active_spi);

    return 0;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: readfromspi()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns -1 if there was an error
 */
//#pragma GCC optimize ("O3")
int readfromspi2(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readlength, uint8_t *readBuffer)
{

    decaIrqStatus_t stat;
    stat = decamutexon();

    /* Blocking: Check whether previous transfer has been finished */
    while (HAL_SPI_GetState(hcurrent_active_spi) != HAL_SPI_STATE_READY);

    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, pin_io_active_spi, SPI_CS_state); /**< Put chip select line low */

    /* Send header */
    HAL_SPI_Transmit(hcurrent_active_spi, (uint8_t*)headerBuffer, headerLength, HAL_MAX_DELAY); //No timeout

    while(__HAL_SPI_GET_FLAG(hcurrent_active_spi, SPI_FLAG_TXE) == RESET)//Verify that the transmit was ended
    {
    }

    /* for the data buffer use LL functions directly as the HAL SPI read function
     * has issue reading single bytes */
    while (readlength-- > 0)
    {
        /* Wait until TXE flag is set to send data */
        while(__HAL_SPI_GET_FLAG(hcurrent_active_spi, SPI_FLAG_TXE) == RESET)
        {
        }

        hcurrent_active_spi->Instance->DR=0;

        /* set output to 0 (MOSI), this is necessary for
        e.g. when waking up DW3000 from DEEPSLEEP via dwt_spicswakeup() function.
        */

        /* Wait until RXNE flag is set to read data */
        while(__HAL_SPI_GET_FLAG(hcurrent_active_spi, SPI_FLAG_RXNE) == RESET)
        {
        }

        (*readBuffer++) = hcurrent_active_spi->Instance->DR;  //copy data read form (MISO)
    }


    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, pin_io_active_spi, (GPIO_PinState)(!((uint8_t)SPI_CS_state))); /**< Put chip select line high */

    decamutexoff(stat);

    return 0;
} // end readfromspi()

/****************************************************************************
 *
 *                              END OF DW3xxx SPI section
 *
 *******************************************************************************/
#endif
