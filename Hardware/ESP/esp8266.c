#include "esp8266.h"
#include "transport.h"
#include "lvgl.h"
#include "string.h"
#include "cmsis_os.h"
uint8_t dat_buf[100]={"\0"};
uint8_t esp8266_send_cmd(char *cmd,char *ack)
{
	uint8_t len=0;
	len = strlen(ack);
	memset(dat_buf,0,100);
	uint8_t res = 0;
	transport_sendPacketBuffer(0,(uint8_t *)cmd,strlen(cmd));
	if(ack != NULL)
	{
		transport_getdata(dat_buf,len);

		res = strcmp((char *)dat_buf,ack);
		printf("%s",dat_buf);
//		printf("Dat:%s\r\nAck:%s\r\nRead num:%d Check num:%d \r\n",dat_buf,ack,len,res);
		return res;
	}else
		return res;
}


uint8_t esp8266_Connect_IOTServer(void)
{
  uint8_t res =0;
	printf("Close the echo!\r\n");
	esp8266_send_cmd("ATE0\r\n",NULL);
	osDelay(50);
	
	printf("Monitor online!\r\n");
	res = esp8266_send_cmd("AT\r\n","\r\nOK\r\n");
	if(res)
		printf("AT Wrong Code£º%d\r\n",res);
	osDelay(50);
	
	printf("Setting pattern!\r\n");
	res = esp8266_send_cmd("AT+CWMODE=1\r\n","\r\nOK\r\n");
	if(res)
		printf("AT CWMODE Wrong Code£º%d\r\n",res);
	osDelay(50);
	
	printf("Reset the module!\r\n");
	res = esp8266_send_cmd("AT+RST\r\n",NULL);
	osDelay(3000);
	printf("Reset OK!\r\n");
	
	printf("Close the echo!\r\n");
	esp8266_send_cmd("ATE0\r\n",NULL);
	osDelay(50);	

	printf("Cancel automatic connection!\r\n");
	res = esp8266_send_cmd("AT+CWAUTOCONN=0\r\n","\r\nOK\r\n");
	if(res)
		printf("AT CWAUTOCONN Wrong Code£º%d\r\n",res);
	
	printf("Connect to WIFI!\r\n");
	res = esp8266_Connect_AP();
	if(res)
		printf("Connect AP Wrong Code£º%d\r\n",res);
	else
		printf("AP CONNECT OK!\r\n");
	
	osDelay(3000);
	
	res = esp8266_send_cmd("AT+CWDHCP=1,1\r\n","\r\nOK\r\n");
	if(res)
		printf("AP CWDHCP Wrong Code£º%d\r\n",res);
	else
		printf("AP CWDHCP OK!\r\n");
	
	res = esp8266_send_cmd("AT+CIPMUX=0\r\n","\r\nOK\r\n");
	if(res)
		printf("AT CIPMUX Wrong Code£º%d\r\n",res);
	else
		printf("AT CIPMUX OK!\r\n");
	
	printf("CONNECT to Server!\r\n");
	res = esp8266_Connect_Server();
	if(res)
		printf("Connect to Server Wrong Code£º%d\r\n",res);
	else
		printf("AT Server OK!\r\n");
	
	res = esp8266_send_cmd("AT+CIPMODE=1\r\n","\r\nOK\r\n");
	if(res)
		printf("AT CIPMODE Wrong Code£º%d\r\n",res);
	else
		printf("AP CIPMODE OK!\r\n");
	
	res = esp8266_send_cmd("AT+CIPSEND\r\n","\r\nOK\r\n");
	if(res)
		printf("AT CIPSEND Wrong Code£º%d\r\n",res);
	else
			printf("AT CIPSEND OK!\r\n");
	
	return 0;
}

uint8_t esp8266_Connect_AP()
{
	char cmd[50]={0};
	sprintf(cmd,"AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,PASS);
	return esp8266_send_cmd(cmd,"WIFI CONNECTED\r\nWIFI GOT IP\r\n\r\nOK\r\n");
}

uint8_t esp8266_Connect_Server()
{
	char cmd[100]={0};
	sprintf(cmd,"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",IOT_DOMAIN_NAME,IOT_PORTNUM);
	return esp8266_send_cmd(cmd,"CONNECT\r\n\r\nOK\r\n");
}
