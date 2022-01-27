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

		printf("���������Ʒ���������\r\n");
		MQTTPacket_connectData data = MQTTPacket_connectData_initializer;//���ò��ֿɱ�ͷ����ֵ
	
		printf("ClientId:%s\r\n",ClientID);
		printf("Username:%s\r\n",Username);
		printf("Password:%s\r\n",Password);
		
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

