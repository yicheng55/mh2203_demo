#include "uip.h"
#include <string.h>
#include <stdio.h>	
#include <stdbool.h>
#include "udp_t.h"

uint16_t UDP_ServerPort = UDP_SERVER_PORT;			//UDP服务器本地端口,用于新数据端口识别
 bool isAnyPort = false;					//客户端任意端口标志

typedef struct
{
	uint16_t RxLen;		//接收数据长度
	uint16_t	TxLen;		//发送数据长度
	uint16_t ClientPort;	//客户端端口
	uint16_t	ServerPort;	//服务器端口
	uint8_t	RxBuff[UIP_RX_BUFF_ZISE];	//接收缓冲区
	uint8_t	TxBuff[UIP_TX_BUFF_ZISE];	//接收缓冲区
}UIP_USER;

UIP_USER udp_Server;



/*************************************************************************************************************************

* 函数			:	void udp_server_connected(uint16_t ServerPort,uint16_t ClientPort)

* 功能			:	建立一个UDP服务器(广播方式)
* 参数			:	ServerPort:服务器本地端口,ClientPort:客户端端口,0:任意端口;非0:指定端口
* 返回			:	无
* 说明			: 	必须放在UDP客户端初始化之前
*************************************************************************************************************************/

void udp_server_connected(uint16_t ServerPort,uint16_t ClientPort)
{
   uip_ipaddr_t ipaddr;
    static struct uip_udp_conn *c=0;
	uip_ipaddr(&ipaddr,0xff,0xff,0xff,0xff); 
	
	UDP_ServerPort = ServerPort;							//本地端口
	uip_listen(HTONS(ServerPort));
	uip_udp_bind(&uip_udp_conns[0], htons(ServerPort));		//绑定本地端口
	
	udp_Server.RxLen = 0;
	udp_Server.TxLen = 0;
	udp_Server.ServerPort = ServerPort;						//服务器端口
	if(ClientPort != 0)										//指定端口
	{
		uip_udp_remove(c);
		uip_udp_conns[0].rport = HTONS(ClientPort);
		udp_Server.ClientPort = ClientPort;
		isAnyPort = false;									//客户端指定端口
	}

	else
	{
		uip_udp_new(&ipaddr,0);     //远程端口为0
		isAnyPort = true;									//客户端任意端口
	}
  
}


/*************************************************************************************************************************

* 函数			:	void udp_server_appcall(void)

* 功能			:	服务器回调函数,用于uip处理数据

* 参数			:	无

* 返回			:	无

* 依赖			:	uip

* 作者			:	cp1300@139.com

* 时间			:	2014-06-04

* 最后修改时间	: 	2014-06-05

* 说明			: 	无

*************************************************************************************************************************/

void udp_server_appcall(void)

{
	if (uip_newdata())
    {
		if(uip_datalen() > UIP_RX_BUFF_ZISE) uip_len = UIP_RX_BUFF_ZISE;	//限制大小
		memcpy(udp_Server.RxBuff, uip_appdata, uip_len);					//复制接收的数据到接收缓冲区
		udp_Server.RxLen = uip_len;											//存储接收数据长度
		udp_Server.ClientPort = (uint16_t)(uip_buf[34]<<8) | uip_buf[35];		//强制获取客户端端口地址
		printf("%s\r\n",(char*)uip_appdata);
	  printf("udp_Server.ClientPort ---%d",udp_Server.ClientPort);
    }

	//新数据到达,轮询,发送数据 
	if(uip_newdata() || uip_poll())
	{
		if(udp_Server.TxLen) 
		{
			uip_send(udp_Server.TxBuff, udp_Server.TxLen);	//发送UDP数据包
			udp_Server.TxLen = 0;
			printf("111\r\n");
		}
		if(udp_Server.RxLen)
		{
			printf("ceshi\r\n");
			udp_ServerSendDataPackage(uip_appdata,5,8080);
			udp_Server.RxLen = 0;
		}
	}
	
}	


/*************************************************************************************************************************

* 函数			:	void udp_ServerSendDataPackage(uint8_t *pBuff, uint16_t len,uint16_t ClientPort)

* 功能			:	UDP 服务器发送数据

* 参数			:	pBuff:发送数据缓冲区,len:发送数据长度,ClientPort:客户端端口

* 返回			:	无

* 依赖			:	uip

* 作者			:	cp1300@139.com

* 时间			:	2014-06-04

* 最后修改时间	: 	2014-06-05

* 说明			: 	无

*************************************************************************************************************************/

void udp_ServerSendDataPackage(uint8_t *pBuff, uint16_t len,uint16_t ClientPort)
{
	if(len > UIP_TX_BUFF_ZISE) len = UIP_TX_BUFF_ZISE;
	memcpy(udp_Server.TxBuff, pBuff, len);
	udp_Server.TxLen = len;
	uip_udp_conns[0].rport = HTONS(ClientPort);			//暂时将客户端端口设置为上一次发送数据的客户端端口
}


/*************************************************************************************************************************

* 函数			:	void udp_ServerSendEndCallBack(uint16_t conn)

* 功能			:	UDP发送数据完成回调函数,目前只支持一个服务器端口

* 参数			:	pBuff:发送数据缓冲区,len:发送数据长度,ClientPort:客户端端口

* 返回			:	无

* 依赖			:	uip

* 作者			:	cp1300@139.com

* 时间			:	2014-06-04

* 最后修改时间	: 	2014-06-05

* 说明			: 	由于UDP服务器的客户端IP设置为0后可以接收任意端口发来的数据,但是却无法发送数据

					到0端口,因此在发送前将客户端端口设置为实际端口,发送完成后修改为0

*************************************************************************************************************************/
void udp_ServerSendEndCallBack(uint16_t conn)
{
	uip_ipaddr_t ipaddr;
	if((conn == 0) && (isAnyPort == true))
	{
		uip_udp_new(&ipaddr,0);     //远程端口为0		//将端口设置为0
	}
}
