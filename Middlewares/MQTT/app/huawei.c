#include "huawei.h"
#include "utils_hmac.h"
#include "cmsis_os.h"
#include <signal.h>
#include "lvgl.h"
uint16_t buflen=200;
unsigned char buf[200]={0};

#define ClientID "61f4c4b0de9933029be530e7_ESP8266_0_0_2022020321"
#define Username "61f4c4b0de9933029be530e7_ESP8266"
#define Password "42492f8f9fca346c5142f7088ab88dd964087663729fa3a3468bb50632d2960b"

#define POST_TOPIC "$oc/devices/61f4c4b0de9933029be530e7_ESP8266/sys/properties/report"

#define GET_TOPIC "$oc/devices/61f4c4b0de9933029be530e7_ESP8266/sys/commands/#"

char RETURN_TOPIC[]="$oc/devices/61f4c4b0de9933029be530e7_ESP8266/sys/commands/response/request_id=0b2e0999-55a1-43f9-a432-d46a8d58ae7e";

#define PAY_LOAD "{\"services\":[{\"service_id\":\"Home\",\"properties\":{\"Temp\":\"%d\"},\"eventTime\":\"NULL\"}]}"

uint8_t huawei_connect()
{
		uint32_t len;

		printf("进入连接云服务器函数\r\n");
	
		memset(buf,0,buflen);
		MQTTPacket_connectData data = MQTTPacket_connectData_initializer;//配置部分可变头部的值
		
		data.MQTTVersion = 3;
		data.clientID.cstring = ClientID;						//客户端标识，用于区分每个客户端xxx为自定义，后面为固定格式
		data.keepAliveInterval = 120;							//保活计时器，定义了服务器收到客户端消息的最大时间间隔,单位是秒
		data.cleansession = 1;									//该标志置1服务器必须丢弃之前保持的客户端的信息，将该连接视为“不存在”
		data.username.cstring = Username;						//用户名 DeviceName&ProductKey
		data.password.cstring = Password;						//密码，工具生成
		
		len = MQTTSerialize_connect(buf, buflen, &data); 		/*1 构造连接报文*/
		transport_sendPacketBuffer(3,buf, len);				//发送连接请求
		
		unsigned char sessionPresent, connack_rc;
		
		do
		{
				while(MQTTPacket_read(buf, buflen, transport_getdata) != CONNACK)//对接收到的报文进行解析
				{
						osDelay(10);
				}
		}
		while(MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0);
		
		if(connack_rc != 0)
		{
			printf("connack_rc:%uc\r\n",connack_rc);
		}else			
			printf("Connect Success!\r\n");
					
		return 0;
}

uint8_t huawei_ping(void)
{
		uint32_t len;
		memset(buf,0,buflen);
		len = MQTTSerialize_pingreq(buf, buflen);
		transport_sendPacketBuffer(3, buf, len);
		printf("Ping...\r\n");
		if (MQTTPacket_read(buf, buflen, transport_getdata) == PINGRESP){
			printf("Ping OK!\r\n");
			return 0;
		}
		else {
			printf("Ping Wrong!\r\n");
			return 1;
		}

}


void test_post(void)
{
	int len = 0;
	int rc = 0;
	unsigned char topic[]= POST_TOPIC;
	unsigned char payload[100]={0};
	
	sprintf((char*)payload,PAY_LOAD,19);
	
	memset(buf,0,buflen);
	MQTTString topicString = MQTTString_initializer;
	topicString.cstring = (char *)topic;
	
	len=MQTTSerialize_publish(buf,buflen,0,0,0,0,topicString,payload,strlen((char *)payload));
	
	rc = transport_sendPacketBuffer(3, buf, len);
	if(!rc)
		printf("Send OK!\r\n");
	else
		printf("Send WRONG!\r\n");
}

int toStop = 0;

void test_get(void)
{
	int msgid = 1;
	MQTTString topicString = MQTTString_initializer;
	MQTTString ropicString = MQTTString_initializer;
	int req_qos = 0;
	char* payload = "{\"result_code\":0,\"response_name\":\"COMMAND_RESPONSE\",\"paras\":{\"result\":\"success\"}}";
	int payloadlen = strlen(payload);
	int len = 0;
	int rc=0;
	char * id;
	
	memset(buf,0,buflen);
	/* subscribe */
	topicString.cstring = GET_TOPIC;
	len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);

	rc = transport_sendPacketBuffer(3, buf, len);
	if (MQTTPacket_read(buf, buflen, transport_getdata) == SUBACK) 	/* wait for suback */
	{
		unsigned short submsgid;
		int subcount;
		int granted_qos;

		rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, buf, buflen);
		if (granted_qos != 0)
		{
			printf("granted qos != 0, %d\n", granted_qos);
		}
	}

	/* loop getting msgs on subscribed topic */
	ropicString.cstring = RETURN_TOPIC;
	while (!toStop)
	{
		/* transport_getdata() has a built-in 1 second timeout,
		your mileage will vary */
		if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBLISH)
		{
			unsigned char dup;
			int qos;
			unsigned char retained;
			unsigned short msgid;
			int payloadlen_in;
			unsigned char* payload_in;
			int rc;
			MQTTString receivedTopic;

			rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
					&payload_in, &payloadlen_in, buf, buflen);
			
			id = &receivedTopic.lenstring.data[strlen("$oc/devices/61f4c4b0de9933029be530e7_ESP8266/sys/commands/request_id=")];
			
			memcpy(&ropicString.cstring[strlen("$oc/devices/61f4c4b0de9933029be530e7_ESP8266/sys/commands/response/request_id=")],
						id,
						strlen("0b2e0999-55a1-43f9-a432-d46a8d58ae7e")
						);

			printf("message arrived %.*s\r\n", payloadlen_in, payload_in);

		}
		memset(buf,0,buflen);
		printf("publishing reading\r\n");
		
		len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, ropicString, (unsigned char*)payload, payloadlen);
		rc = transport_sendPacketBuffer(3, buf, len);
	}
}
