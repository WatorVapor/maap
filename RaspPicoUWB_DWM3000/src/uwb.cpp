#include <Arduino.h>
extern "C" {
    #include "deca_device_api.h"
    #include "deca_regs.h"
    #include "deca_spi.h"
}
#include "shared_defines.h"
#include "port.h"
#include "debug.hpp"

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
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,  /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0      /* PDOA mode off */
};

extern dwt_txconfig_t txconfig_options;

int simple_tx_setup(void);
int simple_tx_loop(void);

void uwb_setup (void) {
    peripherals_init();
    spi_peripheral_init();
    port_set_dw_ic_spi_fastrate();
    reset_DWIC();
    delay(2000);
    while (!dwt_checkidlerc())
    {

    }
    int ret = dwt_initialise(DWT_DW_INIT);
    if ( ret == DWT_ERROR) {
        ERROR_VAR(ret);
    }
    int ret2 = dwt_configure(&config);
    if ( ret2) {
        ERROR_VAR(ret2);
    }
    dwt_configuretxrf(&txconfig_options);
    simple_tx_setup();
}


static uint8_t tx_msg[] = {0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E'};
static const int32_t BLINK_FRAME_SN_IDX  = 1;
static const int32_t  FRAME_LENGTH   =  sizeof(tx_msg)+FCS_LEN;
static const int32_t TX_DELAY_MS = 1000;


void uwb_one_loop(void) {
    simple_tx_loop();
}

int simple_tx_setup(void) {
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
    dwt_configuresleep(DWT_CONFIG, DWT_PRES_SLEEP | DWT_WAKE_CSN | DWT_WAKE_WUP | DWT_SLP_EN);
}
int simple_tx_loop(void) {
    dwt_writetxdata(FRAME_LENGTH-FCS_LEN, tx_msg, 0);
    dwt_writetxfctrl(FRAME_LENGTH, 0, 0);
    dwt_starttx(DWT_START_TX_IMMEDIATE);
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
    { };
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    tx_msg[BLINK_FRAME_SN_IDX]++;
}
