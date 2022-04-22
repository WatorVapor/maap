#include <Arduino.h>
#include "deca_device_api.h"

#define DUMP_VAR_I(x) {\
  Serial.printf("[dump] %s::%s::%d:%s=<%d>\r\n",__FILE__,__func__ ,__LINE__,#x,x);\
}

void uwb_setup(void) {
  Serial.printf("UWB Go!!!!\r\n");
  int32_t api = dwt_apiversion();
  DUMP_VAR_I(api);
}

void uwb_loop(void) {
}
