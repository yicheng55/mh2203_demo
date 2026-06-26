
#ifndef _UDP_T_H_

#define _UDP_T_H_


#include "uip.h"



#define DHCP_TIME_OUT				60					//DHCP获取超时时间,单位S
#define TCP_LINK_SERVER_TIME_OUT	10					//连接服务器超时时间,单位S
#define TCP_LINK_SERVER_CNT			5					//连接服务器重试次数
#define TCP_CLIENT_PORT_AUTO		1					//客户端端口随机分配
#define TCP_CLIENT_DEFAULT_PORT		2400				//客户端默认端
#define TCP_SERVER_DEFAULT_IP		192,168,16,104		//服务器默认IP地址
#define TCP_SERVER_DEFAULT_PORT		8888				//服务器默认端口
#define TCP_SEND_TIME_OUT			5					//数据发送超时时间,单位S

 

#define UDP_LOCAL_PORT				8000				//UDP连接本地端口
#define UDP_REMOTE_PORT				8899				//UDP连接远程端口
#define UDP_SERVER_PORT				8100	

#define UIP_RX_BUFF_ZISE	512		//接收数据缓冲区大小
#define UIP_TX_BUFF_ZISE	512		//发送数据缓冲区大小





//extern UIP_USER udp_Server;	//UDP 服务器数据结构
extern uint16_t UDP_ServerPort;	//UDP服务器本地端口




void udp_server_connected(uint16_t ServerPort,uint16_t ClientPort);				//建立一个UDP服务器(广播方式)

void udp_server_appcall(void);											//服务器回调函数,用于uip处理数据

void udp_ServerSendDataPackage(uint8_t *pBuff, uint16_t len,uint16_t ClientPort);		//UDP 服务器发送数据

void udp_ServerSendEndCallBack(uint16_t conn);								//UDP发送数据完成回调函数

 

 

 

 

#endif //_UDP_SERVER_H_
