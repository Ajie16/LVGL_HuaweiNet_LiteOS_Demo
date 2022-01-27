#include "huawei.h"
#include "utils_hmac.h"
#include "cmsis_os.h"
uint16_t buflen=200;
unsigned char buf[200];

#define ClientID "61e7b86fc7fb24029b0d6cb1_1642576943991_0_0_2022012719"

#define Username "61e7b86fc7fb24029b0d6cb1_1642576943991"

#define Password "52f9f12ba2835e627499ec1a9f9b38a3c21983903f51caeb436ba4d940942b37"


uint8_t huawei_connect()
{
		uint32_t len;

		printf("进入连接云服务器函数\r\n");
		MQTTPacket_connectData data = MQTTPacket_connectData_initializer;//配置部分可变头部的值
	
		printf("ClientId:%s\r\n",ClientID);
		printf("Username:%s\r\n",Username);
		printf("Password:%s\r\n",Password);
		
		data.MQTTVersion = 3;
		data.clientID.cstring = ClientID;						//客户端标识，用于区分每个客户端xxx为自定义，后面为固定格式
		data.keepAliveInterval = 120;							//保活计时器，定义了服务器收到客户端消息的最大时间间隔,单位是秒
		data.cleansession = 1;									//该标志置1服务器必须丢弃之前保持的客户端的信息，将该连接视为“不存在”
		data.username.cstring = Username;						//用户名 DeviceName&ProductKey
		data.password.cstring = Password;						//密码，工具生成
		
		len = MQTTSerialize_connect(buf, buflen, &data); 		/*1 构造连接报文*/
		printf("LEN: %d \r\n",len);
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

