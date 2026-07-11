/* ========================================================================== */
/*                                                                            */
/*   Windows_app.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */
#include <stdio.h>
#include <string.h>
#include "includes.h"

#include "uip.h"
#include "app_call.h"
#include "windows_app.h"
#include "atcommand.h"
#if 0

#define BUFUD ((u8_t *)&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN])
#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])
unsigned char irevBuf[IREVBUF_SIZE];
unsigned int TNumber = 0;
extern uint8_t udp_connected_flag;
extern uint8_t windows_app_set_configuration;

AGENT_SERVER_INFOMATION AgentSrvInfo; // Agent Device server packet message
P_AGENTSRV_UDP_HEAD pUDPFrontHead = (P_AGENTSRV_UDP_HEAD)irevBuf;
P_AGENTSRV_UDP_SETGET_HEAD pUDPFrontHead_SETGET = (P_AGENTSRV_UDP_SETGET_HEAD)irevBuf;

u16_t htons(u16_t val);
uint32_t htonl(uint32_t x);

#if 0
void udp_discovery_init(void)
{
//?إߤ@?udp_client?????
		uip_ipaddr_t ipaddr;
		static struct uip_udp_conn *c = 0;    

		uip_ipaddr(&ipaddr,255,255,255,255);    //??mIP?192.168.1.101

		if(c!=0){ //?w??إ߿?????????
      printf("udp_discovery_init uip_udp_remove \r\n");
			uip_udp_remove(c);
		}	
		c = uip_udp_new(&ipaddr,htons(3094)); //??e?ݿ?e???յ?ݤf?3096
}
#endif

void udp_recv_connect(void)
{
	uip_ipaddr_t ipaddr;
	static struct uip_udp_conn *c=0;
	uip_ipaddr(&ipaddr,255,255,255,255);	//??mIP?255,255,255,255
	
	if(c!=0){							//?w??إ߿?????????
	  printf("udp_recv_connect uip_udp_remove %x \r\n", c->lport);
		uip_udp_remove(c);
	}
	//c = uip_udp_new(&ipaddr,HTONS(3096)); 	//??{?ݤf?0
	c = uip_udp_new(&ipaddr,0); 	//??{?ݤf?0
	printf("* udp_rcv_connect * \r\n");
	if(c){
		
		uip_udp_bind(c, HTONS(UDP_PORT));
		//udp_connected_flag = TRUE;
		}
}

AGENT_UDP_DISC_DATA *AgentSrv_UDP_ReplyData(uint16_t type, uint16_t charformat)
{
		uint8_t i;
		uip_ipaddr_t ipaddr;
		char temp[40];
		
		P_AGENT_UDP_DISC_DATA pUDPRpyData = (P_AGENT_UDP_DISC_DATA)((char *)pUDPFrontHead + sizeof(AGENTSRV_UDP_HEAD));
		
		static AGENT_SERVER_INFOMATION AgentSrvInfo = {
		.Version_Major = DEVICE_INFO_MAJOR,
		.Version_Minor = DEVICE_INFO_MINOR,
		.Version_Release = DEVICE_INFO_RELEASE,
		.Version_Build = DEVICE_INFO_BUILD,
		
		.DeviceID = {AGENTSRV_DEVICE_ID},
		.GetwayID = {AGENTSRV_GETWAY_ID},
		
		.StrName = AGENTSRV_DEVICE_NAME_UART_TO_ETHERNET,
		.StrModel = AGENTSRV_DEVICE_MODEL,
		.StrVendor = AGENTSRV_DEVICE_VENDOR,
		.StrManufact = AGENTSRV_DEVICE_MANUFACTURER,
		
		.IP_Mode = 0,
		.IPv4Addr = {((IP_ADDRESS0) | (IP_ADDRESS1 << 8)), ((IP_ADDRESS2) | (IP_ADDRESS3 << 8))},
		.Subnet_Address = {((Subnet_Address0) | (Subnet_Address1 << 8)), ((Subnet_Address2) | (Subnet_Address3 << 8))},
		.Getway_Address = {((Getway_Address0) | (Getway_Address1 << 8)), ((Getway_Address2) | (Getway_Address3 << 8))},
	
		.TCP_Port = TCP_PORT,
		.UDP_Port = UDP_PORT,
		
		.RFChannel = RF_CHANNEL,
		.RF_DataRate = DATARATE500K,
		.TagSleepTimer = TagSleepTime, // 1 2 5 10 20 40 second
		.AuthenUserName = AGENTSRV_DEVICE_AUTHENTICATION_USER,
		.AuthenUserPassword = AGENTSRV_DEVICE_AUTHENTICATION_PASS,
		.LoginUserName = AGENTSRV_DEVICE_LOGIN_USER,
		.LoginUserPassword = AGENTSRV_DEVICE_LOGIN_PASS,
	};
		
		pUDPRpyData->Major_Version = DEVICE_INFO_MAJOR;
		pUDPRpyData->Minor_Version = DEVICE_INFO_MINOR;
		pUDPRpyData->Release_Version = DEVICE_INFO_RELEASE;
		pUDPRpyData->Build_Version = DEVICE_INFO_BUILD;
		
		pUDPRpyData->Firmware_Major_Version = DEVICE_FIRMWARE_INFO_MAJOR;
		pUDPRpyData->Firmware_Minor_Version = DEVICE_FIRMWARE_INFO_MINOR;
		pUDPRpyData->Firmware_Release_Version = DEVICE_FIRMWARE_INFO_RELEASE;
		pUDPRpyData->Firmware_Build_Version = DEVICE_FIRMWARE_INFO_BUILD;
		pUDPRpyData->Type = htons(type);
		pUDPRpyData->Reserved1 = NONE;

		for(i = 0; i < MAC_Length; i++){
			pUDPRpyData->MAC_Address[i] = uip_ethaddr.addr[i];
		}
		pUDPRpyData->MAC_Address[6] = 0x00;
    pUDPRpyData->MAC_Address[7] = 0x00; 
		pUDPRpyData->MAC_Address_Length = MAC_Length;
		pUDPRpyData->Reserved2 = NONE;
		pUDPRpyData->Port = HTONS(TCP_PORT);
		pUDPRpyData->Reserved3 = NONE;
		pUDPRpyData->CharFormat = htons(charformat);
		
		uip_gethostaddr(ipaddr);
		uip_ipaddr_copy(pUDPRpyData->IPv4, ipaddr);
		
		uip_getnetmask(ipaddr);
		uip_ipaddr_copy(pUDPRpyData->SubmaskV4, ipaddr);

		uip_getdraddr(ipaddr);
		uip_ipaddr_copy(pUDPRpyData->GatewayV4, ipaddr);
		
		memset(pUDPRpyData->IPv6, 0, IPv6_SIZE);
		memset(pUDPRpyData->GatewayV6, 0, IPv6_SIZE);
		
		
    sprintf(pUDPRpyData->ID, "%02X%02X%02X%02X%02X%02X%02X%02X",AgentSrvInfo.GetwayID[0], AgentSrvInfo.GetwayID[1], AgentSrvInfo.GetwayID[2],
																													AgentSrvInfo.GetwayID[3], AgentSrvInfo.GetwayID[4], AgentSrvInfo.GetwayID[5],
																														AgentSrvInfo.GetwayID[6], AgentSrvInfo.GetwayID[7]);
    
    sprintf(pUDPRpyData->Name, "%s", AgentSrvInfo.StrName);
		sprintf(pUDPRpyData->Model, "%s", AgentSrvInfo.StrModel);
		sprintf(pUDPRpyData->Vendor, "%s", AgentSrvInfo.StrVendor);
		sprintf(pUDPRpyData->Manufacturer, "%s", AgentSrvInfo.StrManufact);
		    
    memset(pUDPRpyData->Data, 0, 64);
    
    //Stone add for DeviceName[16]
		//memset(pUDPRpyData->Reserved4, 0, 16);
		for(i = 0; i <16; i++)
		 pUDPRpyData->DeviceName[i] = at_type.devicename[i];
		
		return (pUDPRpyData);
}

AGENTSRV_UDP_HEAD *AgentSrv_UDP_Head(uint16_t length, uint8_t message, uint8_t command, uint8_t encrypt)
{	
		if(TNumber++ > 65535) 
				TNumber = 1;

		pUDPFrontHead->Length = htons(length);
		pUDPFrontHead->Message = message;
		pUDPFrontHead->Command = command;
		pUDPFrontHead->Transaction_Number = TNumber;
		pUDPFrontHead->Encrypt = encrypt;
		pUDPFrontHead->Reserved = NONE;
		pUDPFrontHead->Reserved1 = NONE;
		
		return (pUDPFrontHead);
}

//Stone add for return UDP Set/Get packet

void AgentSrv_UDP_SETGET_Head(uint16_t length, uint8_t message, uint8_t command, uint8_t encrypt)
{
    int i;
    pUDPFrontHead_SETGET->Length = htons(length);
		pUDPFrontHead_SETGET->Message = message;
		pUDPFrontHead_SETGET->Command = command;
		pUDPFrontHead_SETGET->Reserved = NONE;
		pUDPFrontHead_SETGET->Encrypt = encrypt;
		pUDPFrontHead_SETGET->Reserved1 = NONE;
		
		for (i=0; i<18; i++)
		pUDPFrontHead_SETGET->Reserved2[i] = NONE;
 }

void AgentSrv_UDP_SET_ReplyData(uint16_t length, uint16_t status)
{
  P_AGENTSRV_UDP_SET_REPLY_RCVDATA pUDPRpyData_SET = (P_AGENTSRV_UDP_SET_REPLY_RCVDATA)((char *)pUDPFrontHead_SETGET + sizeof(AGENTSRV_UDP_SETGET_HEAD));
  int i;
  
  Read_AT_DataFlash();		//First Read Flash value form dataflash.c
  
  pUDPRpyData_SET->ExternLength = htons(length);
	pUDPRpyData_SET->ExternFormat = NONE;
	for (i=0; i<6; i++)
	 pUDPRpyData_SET->MACaddress[i] = eeprom_type.macaddr[i];
	pUDPRpyData_SET->MACaddress[6] = NONE;
  pUDPRpyData_SET->MACaddress[7] = NONE; 
	pUDPRpyData_SET->Status = status;
	pUDPRpyData_SET->Reserved = NONE;

}

void AgentSrv_UDP_GET_ReplyData(uint16_t length)
{
 P_AGENTSRV_UDP_GET_REPLY_RCVDATA pUDPRpyData_GET = (P_AGENTSRV_UDP_GET_REPLY_RCVDATA)((char *)pUDPFrontHead_SETGET + sizeof(AGENTSRV_UDP_SETGET_HEAD));
 uip_ipaddr_t ipaddr;
 int i;
 
  Read_AT_DataFlash();		//First Read Flash value form dataflash.c
  
  pUDPRpyData_GET->ExternLength = htons(length);
	pUDPRpyData_GET->ExternFormat = NONE;
	pUDPRpyData_GET->DHCPCMode = at_type.dhcpc_mode;
  pUDPRpyData_GET->ROLE = at_type.role;
  
  if (at_type.dhcpc_mode){ //DHCP on
        uip_gethostaddr(ipaddr);
        pUDPRpyData_GET->HOSTIP[0] = ipaddr[0];
        pUDPRpyData_GET->HOSTIP[1] = ipaddr[1];
        uip_getnetmask(ipaddr);
        pUDPRpyData_GET->HOSTMASK[0] = ipaddr[0];
        pUDPRpyData_GET->HOSTMASK[1] = ipaddr[1];
        uip_getdraddr(ipaddr);
        pUDPRpyData_GET->HOSTGW[0] = ipaddr[0];
        pUDPRpyData_GET->HOSTGW[1] = ipaddr[1];
  }else{  //DHCP off
       pUDPRpyData_GET->HOSTIP[0] = at_type.hostip[0];
       pUDPRpyData_GET->HOSTIP[1] = at_type.hostip[1];
       pUDPRpyData_GET->HOSTMASK[0] = at_type.hostmask[0];
       pUDPRpyData_GET->HOSTMASK[1] = at_type.hostmask[1];
       pUDPRpyData_GET->HOSTGW[0] = at_type.hostgw[0];
       pUDPRpyData_GET->HOSTGW[1] = at_type.hostgw[1];
       }
       
  pUDPRpyData_GET->TCP_LPORT = HTONS(at_type.t_lport);
  pUDPRpyData_GET->TCP_RADDR[0] = at_type.tcp_raddr[0];
  pUDPRpyData_GET->TCP_RADDR[1] = at_type.tcp_raddr[1];
  pUDPRpyData_GET->TCP_RPORT = HTONS(at_type.tcp_rport);
  pUDPRpyData_GET->UDP_LPORT = HTONS(at_type.u_lport);
  pUDPRpyData_GET->UDP_RADDR[0] = at_type.udp_raddr[0];
  pUDPRpyData_GET->UDP_RADDR[1] = at_type.udp_raddr[1];
  pUDPRpyData_GET->UDP_RPORT = HTONS(at_type.udp_rport);
  pUDPRpyData_GET->DNS_MODE = at_type.dns_mode;
  pUDPRpyData_GET->DNS_SADDR[0] = at_type.dns_saddr[0];
  pUDPRpyData_GET->DNS_SADDR[1] = at_type.dns_saddr[1];
  for (i=0; i<32; i++) 
   pUDPRpyData_GET->DNS_SRVNAME[i] = at_type.dns_srvname[i];
  
  pUDPRpyData_GET->DNS_SRVPORT = HTONS(at_type.dns_srvport); 
  pUDPRpyData_GET->KEEPALIVE_MODE = at_type.keepalive_mode;
  pUDPRpyData_GET->BAUDRATE = htonl(at_type.baudrate);
  pUDPRpyData_GET->WORDLEN = at_type.wordlen;
  pUDPRpyData_GET->PARITY = at_type.parity;
  pUDPRpyData_GET->STOP = at_type.stop;
  
  //Stone add for DeviceName[16]
  for (i=0; i<16; i++)
   pUDPRpyData_GET->DeviceName[i] = at_type.devicename[i];
  
  pUDPRpyData_GET->Reserved[0] = NONE;
  pUDPRpyData_GET->Reserved[1] = NONE;
  pUDPRpyData_GET->Reserved[2] = NONE;

#if 0  
  for (i=0; i<6; i++)
   pUDPRpyData_GET->Reserved[i] = eeprom_type.macaddr[i];
  
  pUDPRpyData_GET->Reserved = NONE; 
#endif  
  
}

uint16_t AgentSrv_UDP_Set_RpyData(void)
{
 uint16_t sendlen = 0;
 sendlen = sizeof(AGENTSRV_UDP_SETGET_HEAD)+ sizeof(AGENTSRV_UDP_SET_REPLY_RCVDATA);
 
 AgentSrv_UDP_SETGET_Head(sendlen, DAVICOM_MESSAGE_SET_REPLY, NONE, NONE);
 
 AgentSrv_UDP_SET_ReplyData(sendlen, 0x5A5A);
 return sendlen;
}

uint16_t AgentSrv_UDP_Get_RpyData(void)
{
 uint16_t sendlen = 0;
 sendlen = sizeof(AGENTSRV_UDP_SETGET_HEAD)+ sizeof(AGENTSRV_UDP_GET_REPLY_RCVDATA);
 
 AgentSrv_UDP_SETGET_Head(sendlen, DAVICOM_MESSAGE_GET_REPLY, NONE, NONE);
 
 AgentSrv_UDP_GET_ReplyData(sendlen);
 
 return sendlen;
}

uint16_t AgentSrv_UDP_RpyData(void)
{  
		uint16_t sendlen = 0;
	
		sendlen = sizeof(AGENTSRV_UDP_HEAD) + sizeof(AGENT_UDP_DISC_DATA);
		printf("sendlen = %d\r\n", sendlen);
	
		AgentSrv_UDP_Head(sendlen, DAVICOM_MESSAGE_REPLY, DAVICOM_COMMAND_OFFER, ENCRYPTION);
    printf("AgentSrv_UDP_Head \r\n");			
		AgentSrv_UDP_ReplyData(DAVICOM_UART_TO_ETHERNET, CHAR_FORMAT_ANSI);
		
    printf("AgentSrv_UDP_ReplyData \r\n");
		if(pUDPFrontHead->Encrypt){ //means encode data need to encode
			//Codec((unsigned char*)irevBuf, sendlen, DAVICOM_DISCOVER_ENCRYPTION_STARTING_POSITION);
		}
    printf("Encrypt sendlen %d \r\n", sendlen);
		return sendlen;
}

void Read_UDP_packet_to_data_flash(void)
{
 P_AGENTSRV_UDP_GET_REPLY_RCVDATA pUDPRpyData_GET = (P_AGENTSRV_UDP_GET_REPLY_RCVDATA)((char *)pUDPFrontHead_SETGET + sizeof(AGENTSRV_UDP_SETGET_HEAD));
 int i;

	at_type.dhcpc_mode = pUDPRpyData_GET->DHCPCMode;
  at_type.role = pUDPRpyData_GET->ROLE; //Stone add
  //printf(">>> ip_addr %x %x <<<\r\n", pUDPRpyData_GET->HOSTIP[0], pUDPRpyData_GET->HOSTIP[1]);
  at_type.hostip[0] = pUDPRpyData_GET->HOSTIP[0];
  at_type.hostip[1] = pUDPRpyData_GET->HOSTIP[1];
  at_type.hostmask[0] = pUDPRpyData_GET->HOSTMASK[0];
  at_type.hostmask[1] = pUDPRpyData_GET->HOSTMASK[1];
  at_type.hostgw[0] = pUDPRpyData_GET->HOSTGW[0];
  at_type.hostgw[1] = pUDPRpyData_GET->HOSTGW[1];
         
  at_type.t_lport = HTONS(pUDPRpyData_GET->TCP_LPORT);
  at_type.tcp_raddr[0] = pUDPRpyData_GET->TCP_RADDR[0];
  at_type.tcp_raddr[1] = pUDPRpyData_GET->TCP_RADDR[1];
  //pUDPRpyData_GET->TCP_RPORT = NONE;
  at_type.tcp_rport = HTONS(pUDPRpyData_GET->TCP_RPORT);
  at_type.u_lport = HTONS(pUDPRpyData_GET->UDP_LPORT);
  at_type.udp_raddr[0] = pUDPRpyData_GET->UDP_RADDR[0];
  at_type.udp_raddr[1] = pUDPRpyData_GET->UDP_RADDR[1];
  //pUDPRpyData_GET->UDP_RPORT = NONE;
  at_type.udp_rport = HTONS(pUDPRpyData_GET->UDP_RPORT);
  at_type.dns_mode = pUDPRpyData_GET->DNS_MODE;
  at_type.dns_saddr[0] = pUDPRpyData_GET->DNS_SADDR[0];
  at_type.dns_saddr[1] = pUDPRpyData_GET->DNS_SADDR[1];
  for (i=0; i<32; i++) 
   at_type.dns_srvname[i] = pUDPRpyData_GET->DNS_SRVNAME[i];
   
  at_type.dns_srvport = HTONS(pUDPRpyData_GET->DNS_SRVPORT); 
  at_type.keepalive_mode = pUDPRpyData_GET->KEEPALIVE_MODE;
  //printf(">>> pUDPRpyData_GET->BAUDRATE %x <<<\r\n", pUDPRpyData_GET->BAUDRATE);
  at_type.baudrate = htonl(pUDPRpyData_GET->BAUDRATE);
  at_type.wordlen = pUDPRpyData_GET->WORDLEN;
  at_type.parity = pUDPRpyData_GET->PARITY;
  at_type.stop = pUDPRpyData_GET->STOP;
  
  /*pUDPRpyData_GET->Reserved[0] = NONE;
  pUDPRpyData_GET->Reserved[1] = NONE;
  pUDPRpyData_GET->Reserved[2] = NONE;*/
  
  //Stone add for DeviceName[16]
  for (i=0; i<16; i++)
    at_type.devicename[i] = pUDPRpyData_GET->DeviceName[i];
#if 0  
  for (i=0; i<6; i++)
   eeprom_type.macaddr[i] = pUDPRpyData_GET->Reserved[i];
#endif
  
  _Write_AT_DataFlash();
  
}


void Windows_appcall()
{
 uint8_t DisCommand = 0, c, i, j;
 uint16_t RxPort = 0, RxLen = 0;
 uint16_t sendlen = 0;
 P_AgentSrv_UdpState s = (P_AgentSrv_UdpState)&uip_udp_conn->appstate;
 P_AGENT_UDP_REQ_DATA pReqData = (P_AGENT_UDP_REQ_DATA)((char *)pUDPFrontHead + sizeof(AGENTSRV_UDP_HEAD));
	
 if (uip_newdata()){
			memset(irevBuf, 0, sizeof(irevBuf));
			memcpy(irevBuf, uip_appdata, uip_len);	
						
			RxLen = htons(pUDPFrontHead->Length);
			printf("Windows_appcall - Message = %x \r\n", pUDPFrontHead->Message);  //Stone add for Windows app
			//printf("Windows_appcall - Struct Rx len = %d.\r\n", RxLen);
			
			if(pUDPFrontHead->Encrypt == 1){ //means encode data need to decode
				//Codec((unsigned char*)irevBuf, uip_len, DAVICOM_DISCOVER_ENCRYPTION_STARTING_POSITION);//Decode
			}
			//copy remote ip & port to uip_udp_conn connection table		
			for(c = 0; c < UIP_UDP_CONNS; ++c) {
				if((uip_udp_conns[c].lport == HTONS(UDP_PORT)) /*&& (uip_udp_conns[c].rport == 0)*/) {
					uip_ipaddr_copy(&uip_udp_conns[c].ripaddr, UDPBUF->srcipaddr);
					uip_udp_conns[c].rport = HTONS(((uint16_t)(uip_buf[34] << 8) | uip_buf[35]));
					break;
				}else{
					break;
				}
			}
			//DisCommand = htonl(pReqData->Discovery_Command);
			DisCommand = pReqData->Discovery_Command;
			//printf("DisCommand = %d.\n", DisCommand);
			
			RxPort = HTONS(pReqData->Port);
			printf("Windows_appcall Port = %d.\n", RxPort);
			
			//Stone add for Windows app
			switch(pUDPFrontHead->Message){
			case 0x01: //Discovery request
			if(DisCommand){
			  udp_connected_flag = TRUE;
			  
				s->len = AgentSrv_UDP_RpyData();
				printf("Windows_appcall s->len %d udp_connected_flag %d \r\n", s->len, udp_connected_flag);

				for (j=0; j<5; j++){
					for (i=0; i<16; i++)
				   printf(" %x ", irevBuf[(j*16)+i]);
				  printf("\r\n"); 
				 } 
				} //end of DisCommand
			      break;
       
       case 0x03:	//Get configuration
            s->len = AgentSrv_UDP_Get_RpyData();
            printf("+ RX UDP Get Configuration packet. s->len = %x \r\n", s->len);
            break;
       
       case 0x04:	//Set configuration
            printf("+ RX UDP Set Configuration packet. \r\n");
            windows_app_set_configuration =1;
            Read_UDP_packet_to_data_flash();
            
            sendlen = sizeof(AGENTSRV_UDP_SETGET_HEAD)+ sizeof(AGENTSRV_UDP_SET_REPLY_RCVDATA);
            AgentSrv_UDP_SETGET_Head(sendlen, DAVICOM_MESSAGE_SET_REPLY, NONE, NONE);
 
            AgentSrv_UDP_SET_ReplyData(sendlen, 0x5A5A);
            
            s->len = sendlen;
            break;
			} //end of switch
			//Stone add for return UDP packet!!
			s->ptr = irevBuf;
			memcpy(uip_appdata, s->ptr, s->len);
			uip_udp_send(s->len);
		}	
}
#endif
