#pragma once

#include <iostream>
#include <stdio.h>
#include <string.h>
/*
#define DUMP_VAR(x) { \
    std::cout << "DUMP " << __FILE__ << "::" << __LINE__  << "::" << #x << "=<" << x << ">" <<std::endl;\
}
*/
#define DUMP_VAR_I(x) { \
    char buff[128];\
    snprintf(buff,sizeof(buff),"DUMP %s::%s:%d:%s=<%d>\r\n",__FILE__,__func__,__LINE__,#x,x);\
    Serial.print(buff);\
}
#define DUMP_VAR_S(x) { \
    char buff[128];\
    snprintf(buff,sizeof(buff),"DUMP %s::%s:%d:%s=<%s>\r\n",__FILE__,__func__,__LINE__,#x,x);\
    Serial.print(buff);\
}


/*
#define ERROR_VAR(x) { \
    std::cout << "ERROR " << __FILE__ << "::" << __LINE__  << "::" << #x << "=<" << x << ">" <<std::endl;\
}
*/

#define ERROR_VAR_I(x) { \
    char buff[256];\
    sprintf(buff,"ERROR %s::%d:%s=<%d>\r\n",__FILE__,__LINE__,#x,x);\
    Serial.print(buff);\
}


