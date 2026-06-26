#include "stdio.h"
#include "stdlib.h"
//#include "at32f4xx.h"

#include "uip.h"

//#include "app_call.h"
#include "udp_app.h"
#include "string.h"

#include "udp_t.h"

uint8_t udp_recv_databuf[200];       //发送数据缓存      
uint8_t udp_recv_sta;                //客户端状态
uint8_t udp_send_databuf[200];       //发送数据缓存    
uint8_t udp_send_sta;                //发送端状态
//[7]:0,无连接;1,已经连接;
//[6]:0,无数据;1,收到客户端数据
//[5]:0,无数据;1,有数据需要发送
void udp_send_data(char *str,short n);

void udp_send_data(char *str,short n)  
{  
   char   *nptr;    
   nptr = (char *)uip_appdata;        
   memcpy(nptr, str, n);  
   uip_udp_send(n);   //发送n个数据  
}


//回调函数不发数据，只接收数据
void udp_recv_appcall(void)
{
    //struct udp_appstate *s = (struct udp_appstate *)&uip_udp_conn->appstate;
    //static struct uip_udp_conn *c=0;  
	  uint16_t buf_port;
		uip_ipaddr_t ipaddr;
	
    //接收到一个新的udp数据包 
    if (uip_newdata())//收到客户端发过来的数据
    {
			strcpy((char*)udp_recv_databuf,uip_appdata);
			memset(udp_recv_databuf, 0, sizeof(udp_recv_databuf));
			memcpy(udp_recv_databuf, uip_appdata, uip_len);
			buf_port = (uint16_t)(uip_buf[34]<<8) | uip_buf[35];  ////强制获取客户端的端口地址
      printf("port--%d\r\n",buf_port);
      uip_udp_conns[1].rport =  HTONS(buf_port); 
			
      uip_udp_new(&ipaddr,buf_port);			////先设置客户端的端口为之前保存下来的				
			udp_send_data((char*)udp_recv_databuf,uip_len); ////目前测试这个发送函数不起作用
      printf("udp_recv_databuf -- %s\r\n",(char *)uip_appdata);	 ////测试接收上位机发送下来的数据		
			/*if(uip_len<4){  
				udp_send_data("Please check the command!\n",26);  
			}else if(strncmp(udp_recv_databuf,"getname",7)==0){  
        udp_send_data("STM32F103ZR Chip.\n",19);  
			}else {  
        udp_send_data("Unkown command!\n",16);  
			} */
		}			
    /*if(uip_poll())//udp空转
		{
        uip_log("udp_server uip_poll!\r\n");//打印log  
    }*/
}

//建立UDP接收链接
//建立UDP服务器需要将目标IP设置为全1 并对应端口为0,绑定相应的数据端口
void udp_recv_connect(void)
{
    uip_ipaddr_t ipaddr;
    static struct uip_udp_conn *c=0;
	
    uip_ipaddr(&ipaddr,0xff,0xff,0xff,0xff);    //将远程IP设置为 255.255.255.255 具体原理见uip.c的源码   ////这部分的测试没有搞明白
    
		if(c!=0)    //已经建立连接则删除连接
		{                            
        uip_udp_remove(c);  
    }
		
    c = uip_udp_new(&ipaddr,0);     //远程端口为0
		if(c)
    {
			uip_udp_bind(c, HTONS(1600));    ////本地端口设置为1600
    }
}


//这是一个udp 发送端应用回调函数。
//该函数通过UIP_APPCALL(udp_demo_appcall)调用,实现Web Client的功能.
//当uip事件发生时，UIP_APPCALL函数会被调用,根据所属端口(1400),确定是否执行该函数。
//例如 : 当一个udp连接被创建时、有新的数据到达、数据已经被应答、数据需要重发等事件
void udp_send_appcall(void)
{         
	struct udp_appstate *s = (struct udp_appstate *)&uip_udp_conn->appstate;
    
	if(uip_poll())//当前连接空闲轮训
	{    
		  //uip_log("udp_send uip_poll!\r\n");//打印log
			s->textptr = udp_send_databuf;
      s->textlen=strlen((const char*)udp_send_databuf);
      uip_send(s->textptr, s->textlen);//发送udp数据包    
      uip_udp_send(s->textlen);
  }
}

//建立一个udp_client的连接
void udp_send_connect(void)
{
	uip_ipaddr_t ipaddr;
	static struct uip_udp_conn *c=0;    
	
	uip_ipaddr(&ipaddr,192,168,1,19);    //设置IP为192.168.1.101
	
	if(c!=0)
  {                            //已经建立连接则删除连接
		uip_udp_remove(c);
		printf("remove_udp_connect\r\n");
  }	
  c = uip_udp_new(&ipaddr,htons(1500));     //端口为1500
  //发送端发送的数据端口为1500

}


























































