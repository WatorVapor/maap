#include <Arduino.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port.h"
#include "shared_defines.h"

#include "debug.hpp"

//#define RUNTX 1
#define RUNRX 1


int read_dev_id(void);
bool config_tx(void);
bool config_rx(void);

void uwb_setup(void) {
    Serial.printf("UWB Go!!!!\r\n");
    DUMP_VAR_S(__DATE__);
    DUMP_VAR_S(__TIME__);
    peripherals_init();
    spi_peripheral_init();
    port_set_dw_ic_spi_fastrate();
    /* Reset DW IC */
    reset_DWIC(); /* Target specific drive of RSTn line into DW IC low for a period. */

    Sleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)
    delay(2000);

    wakeup_device_with_io();

    int32_t api = dwt_apiversion();
    DUMP_VAR_I(api);

    //char *apis = dwt_version_string();
    //DUMP_VAR_S(apis);
    read_dev_id();
#if RUNTX
    config_tx();
#endif
#if RUNRX
    config_rx();
#endif

}

int read_dev_id(void)
{
    int err;
    uint32_t dev_id;

    DUMP_VAR_I(dev_id);
    dev_id = dwt_readdevid();
    DUMP_VAR_I(dev_id);

    /* Reads and validate device ID returns DWT_ERROR if it does not match expected else DWT_SUCCESS */
    if ((err = dwt_check_dev_id()) == DWT_SUCCESS)
    {
        Serial.printf("DEV ID OK\r\n");
    }
    else
    {
        Serial.printf("DEV ID FAILED\r\n");
    }
    return err;
}



static dwt_config_t config = {
    5,               /* Channel number. */
    DWT_PLEN_128,    /* Preamble length. Used in TX only. */
    DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    1,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,      /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    DWT_PHRRATE_STD, /* PHY header rate. */
    (129 + 8 - 8),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF,
    DWT_STS_LEN_64,  /* STS length, see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0      /* PDOA mode off */
};
extern dwt_txconfig_t txconfig_options;
bool config_tx(void)
{
    while (!dwt_checkidlerc())
    { };
    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        Serial.printf("INIT FAILED\r\n");
        return false;
    }
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK) ;
    if(dwt_configure(&config)){
        Serial.printf("CONFIG FAILED\r\n");
        return false;
    }
    dwt_configuretxrf(&txconfig_options);
    return true;

}


bool config_rx(void)
{
    while (!dwt_checkidlerc())
    { };
    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        Serial.printf("INIT FAILED\r\n");
        return false;
    }
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK) ;
    if(dwt_configure(&config)){
        Serial.printf("CONFIG FAILED\r\n");
        return false;
    }
    return true;
}

bool simple_tx(void);
bool simple_rx(void);

void uwb_loop(void) {
    //DUMP_VAR_I(dwt_spi);
#if RUNTX
    simple_tx();
#endif
#if RUNRX
    simple_rx();
#endif
}


bool simple_tx(void)
{
    std::string msg;
    static int msgId = 0;
    msg = std::to_string(msgId++);
    msg += "mapp tx test!!!";
    uint8_t buffer[msg.length() + 2 ];
    memcpy(buffer,msg.c_str(),msg.length());
    dwt_writetxdata(msg.length(), buffer, 0);
    dwt_writetxfctrl(msg.length(),0,0);
    dwt_starttx(DWT_START_TX_IMMEDIATE);
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
    { };
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    Serial.printf("%s\r\n",msg.c_str());
    return true;
}

static uint8_t rx_buffer[FRAME_LEN_MAX];
bool simple_rx(void)
{
    uint32_t status_reg;

    memset(rx_buffer,0,sizeof(rx_buffer));
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR )))
    { };
    if (status_reg & SYS_STATUS_RXFCG_BIT_MASK)
    {
        /* A frame has been received, copy it to our local buffer. */
        uint16_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_BIT_MASK;
        if (frame_len <= FRAME_LEN_MAX)
        {
            dwt_readrxdata(rx_buffer, frame_len-FCS_LEN, 0); /* No need to read the FCS/CRC. */
            DUMP_VAR_S(rx_buffer);
            std::string msg((char*)rx_buffer,frame_len-FCS_LEN);
            Serial.printf("%s\r\n",msg.c_str());
        }
        /* Clear good RX frame event in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK); 
    }
    else
    {
        /* Clear RX error events in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
    }
    return true;
}