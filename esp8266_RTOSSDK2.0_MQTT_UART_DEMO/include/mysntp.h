#ifndef __mysntp__
#define __mysntp__

#include "esp_common.h"
#include "sntp.h"


void mysntp_init(void);
u32  sntp_gettime(void);
void mysntp_gettime(u32 timeStamp,u8 Time_data[6]);

#endif


