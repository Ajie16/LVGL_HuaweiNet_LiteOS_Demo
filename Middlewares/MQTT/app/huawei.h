#ifndef __HUAWEI_H
#define __HUAWEI_H


#include "stm32l4xx_hal.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "string.h"
#include "stdlib.h"
#include "usart.h"
 
 
extern uint16_t buflen;
extern unsigned char buf[200];

uint8_t huawei_connect(void);
uint8_t huawei_ping(void);

void test_post(void);

#endif


