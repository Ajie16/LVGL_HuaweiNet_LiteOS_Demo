#ifndef __HUAWEI_H
#define __HUAWEI_H


#include "stm32l4xx_hal.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "string.h"
#include "stdlib.h"
#include "usart.h"
 
 

#define ClientID "61f4c4b0de9933029be530e7_ESP8266_0_0_2022020321"
#define Username "61f4c4b0de9933029be530e7_ESP8266"
#define Password "42492f8f9fca346c5142f7088ab88dd964087663729fa3a3468bb50632d2960b"

#define POST_TOPIC "$oc/devices/61f4c4b0de9933029be530e7_ESP8266/sys/properties/report"

#define GET_TOPIC "$oc/devices/61f4c4b0de9933029be530e7_ESP8266/sys/commands/#"
#define PAY_LOAD "{\"services\":[{\"service_id\":\"Home\",\"properties\":{\"Temp\":\"%d\",\"Hum\":\"%d\"},\"eventTime\":\"NULL\"}]}"
 
extern uint16_t buflen;
extern unsigned char buf[200];

uint8_t huawei_connect(void);
uint8_t huawei_ping(void);

void test_post(void);
void test_get(void);
#endif


