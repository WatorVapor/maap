#include <Arduino.h>
#include "deca_device_api.h"
#include "deca_probe_interface.h"
#include "port.h"

#include "debug.hpp"

void init_dwa_3000(void);
int read_dev_id(void);

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

    int32_t api = dwt_apiversion();
    DUMP_VAR_I(api);

    //char *apis = dwt_version_string();
    //DUMP_VAR_S(apis);
    //read_dev_id();
}

bool isDWM3000ProbeGood = false;

void uwb_loop(void) {
    DUMP_VAR_I(SPI);
    /*
    int retProbe = DWT_ERROR;
    retProbe = dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);
    DUMP_VAR_I(retProbe);
    */
   if(isDWM3000ProbeGood == false) {
     init_dwa_3000();
   } else {
       read_dev_id();
   }
}


void init_dwa_3000(void)
{

    /* Probe for the correct device driver. */
    int retProbe = DWT_ERROR;
    retProbe = dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);
    DUMP_VAR_I(retProbe);
    if(retProbe == DWT_SUCCESS) {
        isDWM3000ProbeGood = true;
    }
}

int read_dev_id(void)
{
    int err;
    uint32_t dev_id;

    dev_id = dwt_readdevid();

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

