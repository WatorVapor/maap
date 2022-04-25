#include <Arduino.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include <deca_spi.h>
#include "port.h"
#include "shared_defines.h"
#include "shared_functions.h"

#include "debug.hpp"

//#define RUN_TX 1
//#define RUN_RX 1
#define RUN_INITIATOR 1
//#define RUN_RESPONDER 1


int read_dev_id(void);
bool config_tx(void);
bool config_rx(void);
bool config_initiator(void);
bool config_responder(void);

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

    read_dev_id();
#if RUN_TX
    config_tx();
#endif
#if RUN_RX
    config_rx();
#endif
#if RUN_INITIATOR
    config_initiator();
#endif
#if RUN_RESPONDER
    config_responder();
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

#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385
/* Delay between frames, in UWB microseconds. See NOTE 4 below. */
/* This is the delay from the end of the frame transmission to the enable of the receiver, as programmed for the DW IC's wait for response feature. */
#define POLL_TX_TO_RESP_RX_DLY_UUS 700
/* This is the delay from Frame RX timestamp to TX reply timestamp used for calculating/setting the DW IC's delayed TX function. This includes the
 * frame length of approximately 190 us with above configuration. */
#define RESP_RX_TO_FINAL_TX_DLY_UUS 700
/* Receive response timeout. See NOTE 5 below. */
#define RESP_RX_TIMEOUT_UUS 300
/* Preamble timeout, in multiple of PAC size. See NOTE 7 below. */
#define PRE_TIMEOUT 5
bool config_initiator(void)
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
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);
    dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
    dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);
    dwt_setpreambledetecttimeout(PRE_TIMEOUT);
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
    port_set_dwic_isr(dwt_isr);
    return true;
}


bool config_responder(void)
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
    
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);

    
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
    port_set_dwic_isr(dwt_isr);
    return true;
}


bool simple_tx(void);
bool simple_rx(void);
bool ds_twr_initiator(void);
bool ds_twr_responder(void);

void uwb_loop(void) {
    //DUMP_VAR_I(dwt_spi);
#if RUN_TX
    simple_tx();
#endif
#if RUN_RX
    simple_rx();
#endif
#if RUN_INITIATOR
    ds_twr_initiator();
#endif
#if RUN_RESPONDER
    ds_twr_responder();
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

bool simple_rx(void)
{
    uint32_t status_reg;

    static uint8_t rx_buffer[FRAME_LEN_MAX];
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


/* Frames used in the ranging process. See NOTE 2 below. */
/* Length of the common part of the message (up to and including the function code, see NOTE 2 below). */
#define ALL_MSG_COMMON_LEN 10
/* Indexes to access some of the fields in the frames defined above. */
#define ALL_MSG_SN_IDX 2
#define FINAL_MSG_POLL_TX_TS_IDX 10
#define FINAL_MSG_RESP_RX_TS_IDX 14
#define FINAL_MSG_FINAL_TX_TS_IDX 18
/* Frame sequence number, incremented after each transmission. */

/* Buffer to store received response message.
 * Its size is adjusted to longest frame that this example code is supposed to handle. */
#define RX_BUF_LEN 20

/* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */
static uint32_t status_reg = 0;

bool ds_twr_initiator(void)
{
    static uint8_t tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x21};
    static uint8_t frame_seq_nb = 0;
    tx_poll_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
    dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0); /* Zero offset in TX buffer. */
    dwt_writetxfctrl(sizeof(tx_poll_msg)+FCS_LEN, 0, 1); /* Zero offset in TX buffer, ranging. */
    /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
    * set by dwt_setrxaftertxdelay() has elapsed. */
    dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

    /* We assume that the transmission is achieved correctly, poll for reception of a frame or error/timeout. See NOTE 10 below. */
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    { };

    /* Increment frame sequence number after transmission of the poll message (modulo 256). */
    frame_seq_nb++;
    DUMP_VAR_I(frame_seq_nb);
    DUMP_VAR_I(status_reg & SYS_STATUS_RXFCG_BIT_MASK);

    if (status_reg & SYS_STATUS_RXFCG_BIT_MASK)
    {
        uint32_t frame_len;

        /* Clear good RX frame event and TX frame sent in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_TXFRS_BIT_MASK);

        /* A frame has been received, read it into the local buffer. */
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & FRAME_LEN_MAX_EX;
        DUMP_VAR_I(frame_len);


        static uint8_t rx_buffer[RX_BUF_LEN];
        if (frame_len <= RX_BUF_LEN)
        {
            dwt_readrxdata(rx_buffer, frame_len, 0);
        }

        /* Check that the frame is the expected response from the companion "DS TWR responder" example.
            * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
        rx_buffer[ALL_MSG_SN_IDX] = 0;
        static uint8_t rx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0x10, 0x02, 0, 0};
        if (memcmp(rx_buffer, rx_resp_msg, ALL_MSG_COMMON_LEN) == 0)
        {
            uint32_t final_tx_time;
            int ret;

            /* Retrieve poll transmission and response reception timestamp. */
            static uint64_t poll_tx_ts;
            static uint64_t resp_rx_ts;
            static uint64_t final_tx_ts;

            poll_tx_ts = get_tx_timestamp_u64();
            resp_rx_ts = get_rx_timestamp_u64();

            /* Compute final message transmission time. See NOTE 11 below. */
            final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
            dwt_setdelayedtrxtime(final_tx_time);

            /* Final TX timestamp is the transmission time we programmed plus the TX antenna delay. */
            final_tx_ts = (((uint64_t)(final_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

            static uint8_t tx_final_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            /* Write all timestamps in the final message. See NOTE 12 below. */
            final_msg_set_ts(&tx_final_msg[FINAL_MSG_POLL_TX_TS_IDX], poll_tx_ts);
            final_msg_set_ts(&tx_final_msg[FINAL_MSG_RESP_RX_TS_IDX], resp_rx_ts);
            final_msg_set_ts(&tx_final_msg[FINAL_MSG_FINAL_TX_TS_IDX], final_tx_ts);

            /* Write and send final message. See NOTE 9 below. */
            tx_final_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
            dwt_writetxdata(sizeof(tx_final_msg), tx_final_msg, 0); /* Zero offset in TX buffer. */
            dwt_writetxfctrl(sizeof(tx_final_msg)+FCS_LEN, 0, 1); /* Zero offset in TX buffer, ranging bit set. */

            ret = dwt_starttx(DWT_START_TX_DELAYED);
            /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 13 below. */
            if (ret == DWT_SUCCESS)
            {
                /* Poll DW IC until TX frame sent event set. See NOTE 10 below. */
                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
                { };

                /* Clear TXFRS event. */
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

                /* Increment frame sequence number after transmission of the final message (modulo 256). */
                frame_seq_nb++;
            }
        }
    }
    else
    {
        /* Clear RX error/timeout events in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR | SYS_STATUS_TXFRS_BIT_MASK);
    }

    return true;
}

#define RX_BUF_LEN 24
/* Delay between frames, in UWB microseconds. See NOTE 4 below. */
/* This is the delay from Frame RX timestamp to TX reply timestamp used for calculating/setting the DW IC's delayed TX function. This includes the
 * frame length of approximately 190 us with above configuration. */
#define POLL_RX_TO_RESP_TX_DLY_UUS 900
/* This is the delay from the end of the frame transmission to the enable of the receiver, as programmed for the DW IC's wait for response feature. */
#define RESP_TX_TO_FINAL_RX_DLY_UUS 500
/* Receive final timeout. See NOTE 5 below. */
#define FINAL_RX_TIMEOUT_UUS 220
/* Preamble timeout, in multiple of PAC size. See NOTE 6 below. */
#define PRE_TIMEOUT 5

bool ds_twr_responder(void)
{
    dwt_setpreambledetecttimeout(0);
    /* Clear reception timeout to start next ranging process. */
    dwt_setrxtimeout(0);

    /* Activate reception immediately. */
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    /* Poll for reception of a frame or error/timeout. See NOTE 8 below. */
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    { };

    DUMP_VAR_I(status_reg & SYS_STATUS_RXFCG_BIT_MASK);
    if (status_reg & SYS_STATUS_RXFCG_BIT_MASK)
    {
        uint32_t frame_len;

        /* Clear good RX frame event in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

        /* A frame has been received, read it into the local buffer. */
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & FRAME_LEN_MAX_EX;
        DUMP_VAR_I(frame_len);
        static uint8_t rx_buffer[RX_BUF_LEN];
        if (frame_len <= RX_BUF_LEN)
        {
            dwt_readrxdata(rx_buffer, frame_len, 0);
        }

        /* Check that the frame is a poll sent by "DS TWR initiator" example.
            * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
        rx_buffer[ALL_MSG_SN_IDX] = 0;
        static uint8_t rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x21, 0, 0};
        DUMP_VAR_I(memcmp(rx_buffer, rx_poll_msg, ALL_MSG_COMMON_LEN));
        if (memcmp(rx_buffer, rx_poll_msg, ALL_MSG_COMMON_LEN) == 0)
        {
            uint32_t resp_tx_time;
            int ret;

            /* Retrieve poll reception timestamp. */
            static uint64_t poll_rx_ts;
            poll_rx_ts = get_rx_timestamp_u64();

            /* Set send time for response. See NOTE 9 below. */
            resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
            dwt_setdelayedtrxtime(resp_tx_time);

            /* Set expected delay and timeout for final message reception. See NOTE 4 and 5 below. */
            dwt_setrxaftertxdelay(RESP_TX_TO_FINAL_RX_DLY_UUS);
            dwt_setrxtimeout(FINAL_RX_TIMEOUT_UUS);
            /* Set preamble timeout for expected frames. See NOTE 6 below. */
            dwt_setpreambledetecttimeout(PRE_TIMEOUT);

            /* Write and send the response message. See NOTE 10 below.*/
            static uint8_t tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0x10, 0x02, 0, 0, 0, 0};
            static uint8_t frame_seq_nb = 0;
            tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
            dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0); /* Zero offset in TX buffer. */
            dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1); /* Zero offset in TX buffer, ranging. */
            
            #if 1
            ret = dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
            DUMP_VAR_I(ret);
            /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 11 below. */
            if (ret == DWT_ERROR)
            {
                return false;
            }
            #endif

            /* Poll for reception of expected "final" frame or error/timeout. See NOTE 8 below. */
            while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
            { };

            /* Increment frame sequence number after transmission of the response message (modulo 256). */
            frame_seq_nb++;

            DUMP_VAR_I(frame_seq_nb);
            DUMP_VAR_I(status_reg);
            DUMP_VAR_I(status_reg & SYS_STATUS_RXFCG_BIT_MASK);
            if (status_reg & SYS_STATUS_RXFCG_BIT_MASK)
            {
                /* Clear good RX frame event and TX frame sent in the DW IC status register. */
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_TXFRS_BIT_MASK);

                /* A frame has been received, read it into the local buffer. */
                frame_len = dwt_read32bitreg(RX_FINFO_ID) & FRAME_LEN_MAX_EX;
                DUMP_VAR_I(frame_len);
                if (frame_len <= RX_BUF_LEN)
                {
                    dwt_readrxdata(rx_buffer, frame_len, 0);
                }

                /* Check that the frame is a final message sent by "DS TWR initiator" example.
                    * As the sequence number field of the frame is not used in this example, it can be zeroed to ease the validation of the frame. */
                rx_buffer[ALL_MSG_SN_IDX] = 0;
                static uint8_t rx_final_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                DUMP_VAR_I(memcmp(rx_buffer, rx_final_msg, ALL_MSG_COMMON_LEN) == 0);
                if (memcmp(rx_buffer, rx_final_msg, ALL_MSG_COMMON_LEN) == 0)
                {
                    uint32_t poll_tx_ts, resp_rx_ts, final_tx_ts;
                    uint32_t poll_rx_ts_32, resp_tx_ts_32, final_rx_ts_32;
                    double Ra, Rb, Da, Db;
                    int64_t tof_dtu;

                    /* Retrieve response transmission and final reception timestamps. */
                    static uint64_t resp_tx_ts;
                    static uint64_t final_rx_ts;
                    resp_tx_ts = get_tx_timestamp_u64();
                    final_rx_ts = get_rx_timestamp_u64();

                    /* Get timestamps embedded in the final message. */
                    final_msg_get_ts(&rx_buffer[FINAL_MSG_POLL_TX_TS_IDX], &poll_tx_ts);
                    final_msg_get_ts(&rx_buffer[FINAL_MSG_RESP_RX_TS_IDX], &resp_rx_ts);
                    final_msg_get_ts(&rx_buffer[FINAL_MSG_FINAL_TX_TS_IDX], &final_tx_ts);

                    /* Compute time of flight. 32-bit subtractions give correct answers even if clock has wrapped. See NOTE 12 below. */
                    poll_rx_ts_32 = (uint32_t)poll_rx_ts;
                    resp_tx_ts_32 = (uint32_t)resp_tx_ts;
                    final_rx_ts_32 = (uint32_t)final_rx_ts;
                    Ra = (double)(resp_rx_ts - poll_tx_ts);
                    Rb = (double)(final_rx_ts_32 - resp_tx_ts_32);
                    Da = (double)(final_tx_ts - resp_rx_ts);
                    Db = (double)(resp_tx_ts_32 - poll_rx_ts_32);
                    tof_dtu = (int64_t)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));

                    static double tof;
                    static double distance;
                    tof = tof_dtu * DWT_TIME_UNITS;
                    distance = tof * SPEED_OF_LIGHT;
                    DUMP_VAR_F(tof);
                    DUMP_VAR_F(distance);
                }
            }
            else
            {
                /* Clear RX error/timeout events in the DW IC status register. */
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
            }
        }
    }
    else
    {
        /* Clear RX error/timeout events in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    }

    return true;
}
