#ifndef _ESP_H
#define _ESP_H

#include "stm32l4xx_hal.h"
#include "string.h"
#include "stdlib.h"
#include "usart.h"

#define SSID "TP-LINK_6666"
#define PASS "xj15855065203"

#define IOT_DOMAIN_NAME "a1604049d8.iot-mqtts.cn-north-4.myhuaweicloud.com"
#define IOT_PORTNUM 	"1883"

uint8_t esp8266_send_cmd(char *cmd,char *ack);
uint8_t esp8266_Connect_IOTServer(void);
uint8_t esp8266_Connect_AP(void);
uint8_t esp8266_Connect_Server(void);
uint8_t esp8266_quit_trans(void);

#endif
