#include "huawei.h"
#include "utils_hmac.h"
#include "cmsis_os.h"
#include <signal.h>

int toStop = 0;



uint16_t buflen=200;
unsigned char buf[200];



#define ClientID "61e7b86fc7fb24029b0d6cb1_1642576943991_0_0_2022012815"

#define Username "61e7b86fc7fb24029b0d6cb1_1642576943991"

#define Password "7a64d1450ccf854b128ad16d839e01ec56968006365874de868a543e0c89b738"

uint8_t huawei_connect()
{
		uint32_t len;

		printf("���������Ʒ���������\r\n");
		MQTTPacket_connectData data = MQTTPacket_connectData_initializer;//���ò��ֿɱ�ͷ����ֵ
		
		data.MQTTVersion = 3;
		data.clientID.cstring = ClientID;						//�ͻ��˱�ʶ����������ÿ���ͻ���xxxΪ�Զ��壬����Ϊ�̶���ʽ
		data.keepAliveInterval = 120;							//�����ʱ���������˷������յ��ͻ�����Ϣ�����ʱ����,��λ����
		data.cleansession = 1;									//�ñ�־��1���������붪��֮ǰ���ֵĿͻ��˵���Ϣ������������Ϊ�������ڡ�
		data.username.cstring = Username;						//�û��� DeviceName&ProductKey
		data.password.cstring = Password;						//���룬��������
		
		len = MQTTSerialize_connect(buf, buflen, &data); 		/*1 �������ӱ���*/
		printf("LEN: %d \r\n",len);
		transport_sendPacketBuffer(3,buf, len);				//������������
		
		unsigned char sessionPresent, connack_rc;
		
		do
		{
				while(MQTTPacket_read(buf, buflen, transport_getdata) != CONNACK)//�Խ��յ��ı��Ľ��н���
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
	char payload[100]={0};
	int rc = 0;
	int len = 0;
	
	sprintf(payload,
					"{\"services\":[{\"service_id\":\"BasicData\",\"properties\":{\"luminance\":%d},\"eventTime\":\"NULL\"}]}",
					66);
	
	memset(buf,0,buflen);
	MQTTString topicString = MQTTString_initializer;
	
	topicString.cstring = "$oc/devices/61e7b86fc7fb24029b0d6cb1_1642576943991/sys/properties/report";
	printf("StrLen: %d\r\n",strlen((char*)payload));
	len=MQTTSerialize_publish(buf,buflen,0,0,0,0,topicString,(unsigned char*)payload,strlen(payload));
	printf("Publish Len : %d\r\n",len);
	rc = transport_sendPacketBuffer(3, buf, len);
	if(rc == 0)
		printf("Send OK!\r\n");
}
