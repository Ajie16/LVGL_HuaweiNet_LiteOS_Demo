/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/



#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "usart.h"
#include "main.h"
#include "cmsis_os.h"
#include "transport.h"

extern osSemaphoreId_t TX_DMA_SemaphoreHandle;
extern osSemaphoreId_t RX_DMA_SemaphoreHandle;

/**
This simple low-level implementation assumes a single connection for a single thread. Thus, a static
variable is used for that connection.
On other scenarios, the user must solve this by taking into account that the current implementation of
MQTTPacket_read() has a function pointer for a function call to get the data to a buffer, but no provisions
to know the caller or other indicator (the socket id): int (*getfn)(unsigned char*, int)
*/

//发送数据
int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen)
{
	int res = 1;
	if(osSemaphoreAcquire(TX_DMA_SemaphoreHandle,0xFF) == osOK)
		res = HAL_UART_Transmit_DMA(&hlpuart1, buf, buflen);
	return res;
}


/*
*   作用: 替代原来的transport_getdata函数，因为没有用到socket
*/

int transport_getdata(unsigned char* buf,int n)
{
	if(osSemaphoreAcquire(RX_DMA_SemaphoreHandle,0xFFFF) == osOK)
	{
		hlpuart1.Instance->RDR &= 0x00;
		memset(hlpuart1.pRxBuffPtr,0,hlpuart1.RxXferCount);
		HAL_UART_Receive_DMA(&hlpuart1,buf,n);
		if(osSemaphoreAcquire(RX_DMA_SemaphoreHandle,0xFFFF) == osOK)
			osSemaphoreRelease(RX_DMA_SemaphoreHandle);
		else
			osSemaphoreRelease(RX_DMA_SemaphoreHandle);
	}
	return n;
}


//打开网络并连接服务器
int transport_open(char* addr, int port)
{
	return 0;
}

//关闭网络
int transport_close(int sock)
{
	return 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == hlpuart1.Instance)
		osSemaphoreRelease(TX_DMA_SemaphoreHandle);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == hlpuart1.Instance)
		osSemaphoreRelease(RX_DMA_SemaphoreHandle);
}
