#include <Arduino.h>
#include "deca_device_api.h"
#include "deca_probe_interface.h"
#include "port.h"

#include "debug.hpp"

int read_dev_id(void);

void uwb_setup(void) {
  Serial.printf("UWB Go!!!!\r\n");
  int32_t api = dwt_apiversion();
  DUMP_VAR_I(api);
  SPI.begin();
  //char *apis = dwt_version_string();
  //DUMP_VAR_S(apis);
  //read_dev_id();
}

void uwb_loop(void) {
}


int read_dev_id(void)
{
    int err;
    uint32_t dev_id;


    /* Configure SPI rate, DW3000 supports up to 36 MHz */
    port_set_dw_ic_spi_fastrate();

    /* Reset DW IC */
    reset_DWIC(); /* Target specific drive of RSTn line into DW IC low for a period. */

    Sleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

    /* Probe for the correct device driver. */
    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

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

