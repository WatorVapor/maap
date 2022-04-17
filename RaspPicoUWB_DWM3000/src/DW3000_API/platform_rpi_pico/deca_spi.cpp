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

#define SPI_CHANNEL 0
#define SPI_CLOCK_SPEED 32000000
#define CRC_SIZE 1

#define JUNK 0x00


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
    decaIrqStatus_t  stat ;
    stat = decamutexon() ;

    digitalWrite(PIN_CS, LOW);
    SPI.beginTransaction(dwt_spi_setting);
    for(int i = 0 ;i < headerLength; i++) {
        SPI.transfer(headerBuffer[i]);
    }
    for(int i = 0 ;i < readlength; i++) {
        readBuffer[i] = SPI.transfer(JUNK);
    }
    SPI.endTransaction();
    digitalWrite(PIN_CS, HIGH);

    decamutexoff(stat);

    return 0;
} // end readfromspi()


int writeSpi (int channel, uint8_t *data, int len) {
    digitalWrite(PIN_CS, LOW);
    SPI.beginTransaction(dwt_spi_setting);
    for(int i = 0 ;i < len; i++) {
        SPI.transfer(data[i]);
    }
    SPI.endTransaction();
    digitalWrite(PIN_CS, HIGH);
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


