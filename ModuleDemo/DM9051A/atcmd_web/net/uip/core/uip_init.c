/*
 * Copyright (c) 2001, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Adam Dunkels.
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: main.c,v 1.16 2006/06/11 21:55:03 adam Exp $
 *
 */
#include "includes.h"
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"
#include "uip_timer.h"
#include "uip-split.h"
#include "uip_init.h"
#include "dhcpc.h"

//#include "System_Init.h"
#include "stdio.h"
#include "at32f4xx.h"
#include "DM9051.h"
#include "app_call.h"
#include "atcommand.h"
#include "etherbridge.h"
#include "uip_keepalive.h"

#define BUF		((struct uip_eth_hdr *)&uip_buf[0])

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

void tcpip_periodic_timer_watch_func(void);

/* Function prototype declaration */
static struct timer periodic_timer, arp_timer, keepalive_timer;

/* Default Network Configuration, IP don't set from DHCP*/
#if DHCPC_EN
//static 
struct timer dhcp_timer;  //kj.ui.uiuiu...
#else
void show_netwaork_configure();
#endif //DHCPC_EN

#if DHCPC_EN
void dhcpc_configured(const struct dhcpc_state *s)
#else
void show_netwaork_configure()
#endif //DHCPC_EN
{
	uip_ipaddr_t ipaddr;

#if DHCPC_EN
	if(s->state == STATE_FAIL) {
		/*uip_ipaddr(ipaddr, ip[0], ip[1], ip[2], ip[3]);		//Host IP address
		uip_sethostaddr(ipaddr);
		uip_ipaddr(ipaddr, gw[0], gw[1], gw[2], gw[3]);		//Default Gateway
		uip_setdraddr(ipaddr);
		uip_ipaddr(ipaddr, ns[0], ns[1], ns[2], ns[3]);	//Network Mask
		uip_setnetmask(ipaddr);	*/
		_printf("\n--- Fixed IP addr ---\r\n");
	}else {
		//uip_sethostaddr(s->ipaddr);
		//uip_setnetmask(s->netmask);
		//uip_setdraddr(s->default_router);
		uip_ipaddr_copy(at_type.hostip, s->ipaddr);
		uip_ipaddr_copy(at_type.hostmask, s->netmask);
		uip_ipaddr_copy(at_type.hostgw, s->default_router);
		
		//resolv_conf(s->dnsaddr);			// Now don't need DNS
		_printf("\n--- DHCP IP addr ---\r\n"); //"\n--- IP address setting from DHCP ---\r\n"
	}
	uip_sethostaddr(at_type.hostip);
	uip_setnetmask(at_type.hostmask);	
	uip_setdraddr(at_type.hostgw);
#else
		/*uip_ipaddr(ipaddr, ip[0], ip[1], ip[2], ip[3]);		//Host IP address
  	uip_sethostaddr(ipaddr);
  	uip_ipaddr(ipaddr, gw[0], gw[1], gw[2], gw[3]);		//Default Gateway
  	uip_setdraddr(ipaddr);
  	uip_ipaddr(ipaddr, ns[0], ns[1], ns[2], ns[3]);	//Network Mask
  	uip_setnetmask(ipaddr);	*/
	uip_sethostaddr(at_type.hostip);
	uip_setnetmask(at_type.hostmask);	
	uip_setdraddr(at_type.hostgw);
  	_printf("\n--- Fixed IP addr ---\r\n");
#endif //DHCPC_EN
	
#if 0
	/* Display system information */
	_printf("MUCCPU @ %d Hz\r\n", SystemCoreClock);
	//printf("SPI0 Bus Freq. = %d\n", SPI_GetBusClock(SPI0));
	_printf("Network: DAVICOM DM9051 \r\n");
#endif
	_printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \r\n", uip_ethaddr.addr[0], uip_ethaddr.addr[1],
						uip_ethaddr.addr[2], uip_ethaddr.addr[3], uip_ethaddr.addr[4], uip_ethaddr.addr[5]);
	uip_gethostaddr(ipaddr);
	_printf("Host IP Address: %d.%d.%d.%d \r\n", uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
#if 0
	uip_getnetmask(ipaddr);
	_printf("Network Mask: %d.%d.%d.%d \r\n", uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
#endif
	uip_getdraddr(ipaddr);
	_printf("Gateway Address: %d.%d.%d.%d \r\n", uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
	_printf("---------------------\r\n");
//	DM9051_Write_Reg(DM9051_ISR, 0x20);

	EthernetInit_Done();
}

//uint8_t check_DM9051_link = 0;
uint8_t udp_connected_flag = FALSE;
uint8_t windows_app_set_configuration = 0;

uint8_t eth_netif_linkup = 0; //init.
uint8_t eth_netif_startup_wait = 3; //init
uint8_t eth_start_ethdown_display = 0; //ever code.

uint8_t startup_duration_pull(void)
{
	if (eth_netif_startup_wait) {
		eth_netif_startup_wait--;
		return 1;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
void tcpip_init(void)
{	
	timer_set(&periodic_timer, CLOCK_SECOND / 5);	//200ms
  timer_set(&arp_timer, CLOCK_SECOND * 10); // 10sec
	timer_set(&keepalive_timer, CLOCK_SECOND);	//1sec
		
	/* Init DM9051 Driver	*/
	tapdev_init();

  uip_init();
	uip_arp_init();	// Clear arp table.
	
//	if(at_type.keepalive_mode){
		keepalive_init(); //Clear keepalive table
//	}

	/*Initial and start system tick time = 1ms */
  SysTick_Config(SystemCoreClock / 1000);

#if DHCPC_EN

/* DHCPC_EN does exactly set dhcp_timer.
 * It check link_up status for re-new dhcpc.
 * In case, user could select change from dhcpc_mode 0
 *   to dhcpc_mode 1,
 * So the dhcp_timer should be setup, even if dhcpc_mode 0
 *   for DHCPC_EN 1
 * Hint: change CLOCK_SECOND * 600 to CLOCK_SECOND * 1 (actually less little)
 */
  timer_set(&dhcp_timer, CLOCK_SECOND / 3); //333 ms
  if(at_type.dhcpc_mode) {
	//to check start-up link up or link-change-link-up in dhcp_timer, to let below line executed.
	#if 0
  	//[Sched to goto, while 'dhcpc_mode' to Start a DHCP discover...] //dhcpc_init_pack(&uip_ethaddr, 6);
	#endif
  }else {
  	struct dhcpc_state stmp;
  	stmp.state = STATE_FAIL;
  	dhcpc_configured(&stmp);
  }
#else
	show_netwaork_configure();
#endif //DHCPC_EN
}
/*---------------------------------------------------------------------------*/

/*******************************************************************************
*	function : UipPro
*	Input  :
*	Output :
*	Function Introduce : Trigger Interrupt receive ethernet buffer
********************************************************************************/
static void tcpip_ethtype_ip(void)
{
#ifdef DM9051_INT
		if(DM9051_RX_INT_TRIGGER == 1){
rep:
			DM9051_RX_INT_TRIGGER = 0;
#endif //DM9051_INT
			
			uip_len = tapdev_read();
			
			if (uip_len > 850)
				printf("rxlen %u\n", uip_len);
			if (uip_len > 0) 
			{
				//printf("tcpip_ethtype_ip BUF->type %x \r\n", BUF->type);
				if(BUF->type == htons(UIP_ETHTYPE_IP)) {
					//uip_arp_ipin();   //Removed by Spenser
					uip_input();		// uip_process(UIP_DATA)
					
					/* If the above function invocation resulted in data that
						should be sent out on the network, the global variable
						uip_len is set to a value > 0. */
					if(uip_len > 0) {
#ifdef UIP_SPLIT_TCP_PACKET 
#ifdef UIP_USE_BIGDATA_SIZE
					uip_fast_send();
#else
					uip_split_output();
#endif // UIP_USE_BIGDATA_SIZE
#else                    
					uip_arp_out();
					tapdev_send();
#if 0						
					printf("5. tapdev_send uip_len %x \r\n", uip_len);	
#endif						
#endif //UIP_SPLIT_TCP_PACKET
					}
				} else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
					uip_arp_arpin();
					//printf("+++ uip_arp_arpin uip_len %x +++\r\n", uip_len);
					/* If the above function invocation resulted in data that
						 should be sent out on the network, the global variable
						 uip_len is set to a value > 0. */
					if(uip_len > 0) {
						tapdev_send();
#if 0						
						printf("6. tapdev_send\r\n");
#endif						
					}
				}
			}
#ifdef DM9051_INT
		}else{
			if(DM9051_RX_Final == 0) 
				goto rep; 	
		}
#endif //DM9051_INT
}

static void tcpip_periodic_timer(void)
{
	uint8_t i, sending = 0;

#if DHCPC_EN
	uint8_t status, linkch; //uint16_t
#endif //DHCPC_EN
	
	if ((timer_expired(&periodic_timer)) 
		|| 
		((atcmd_flag == TRUE) && on_trans_mode()) /* enter as many as possible. */
	)
	{
      timer_reset(&periodic_timer);
      for(i = 0; i < UIP_CONNS; i++) {
				uip_periodic(i);
				//printf("uip_periodic i=%x uip_len %x \r\n", i, uip_len);
				/* If the above function invocation resulted in data that
	   			should be sent out on the network, the global variable
	   			uip_len is set to a value > 0. */
				if(uip_len > 0) {
					if (uip_len == 0x2a)
					printf("1. uip_len== 0x2a uip_slen %x\r\n", uip_slen);	
#ifdef UIP_SPLIT_TCP_PACKET 
#ifdef UIP_USE_BIGDATA_SIZE
					uip_fast_send();
#else
					uip_split_output();
#endif // UIP_USE_BIGDATA_SIZE
#else                    
					uip_arp_out();
//					if (uip_len == 0x2a)
//						printf("2. uip_len== 0x2a uip_slen %x\r\n", uip_slen);	
					tapdev_send();
//					printf("3. _send uip_len %x\r\n", uip_len);
#endif //UIP_SPLIT_TCP_PACKET
					sending = 1;
				}
      }
	  bridge_clear_state();
			
#if UIP_UDP
      for(i = 0; i < UIP_UDP_CONNS; i++) {
				uip_udp_periodic(i);
				/* If the above function invocation resulted in data that
	   			should be sent out on the network, the global variable
	   			uip_len is set to a value > 0. */
				if(uip_len > 0) {
//uip_udp_conns[UIP_UDP_CONNS]
//				 printf("S.u arp_out %d.%d.%d.%d uip_len %u (uip_slen %u)\r\n", uip_ipaddr1(uip_udp_conns[i].ripaddr), uip_ipaddr2(uip_udp_conns[i].ripaddr),
//					uip_ipaddr3(uip_udp_conns[i].ripaddr), uip_ipaddr4(uip_udp_conns[i].ripaddr), uip_len, uip_slen);
//uip_ipaddr_copy(BUF->destipaddr, uip_udp_conn->ripaddr);
//				 printf("S.u arp_out %d.%d.%d.%d uip_len %u (uip_slen %u)\r\n", uip_ipaddr1(uip_udp_conn->ripaddr), uip_ipaddr2(uip_udp_conn->ripaddr),
//					uip_ipaddr3(uip_udp_conn->ripaddr), uip_ipaddr4(uip_udp_conn->ripaddr), uip_len, uip_slen);
					uip_arp_out();
//				do {
//				 uint8_t j;
//				 for (j = 0; j < uip_len; j++) //j += 8
//				 {
//					if (!(j % 8))
//						printf(" ");
//					if (!(j % 16))
//						printf("\r\n");
//					//if (uip_len <= (j + 8))
//					printf(" %02x", uip_buf[j]); 
//				 }
//				 printf("\r\n");
//				} while(0);
					tapdev_send();
//					display_udp_conn("Since,Send a udp packet,");
//					printf("4. UDP tapdev_send\r\n");
					sending = 1;
				}
      }
#endif /* UIP_UDP */
      /* Call the ARP timer function every 10 seconds. */
      if(timer_expired(&arp_timer)) {
				timer_reset(&arp_timer);
#if 1	 		//Stone add for udp & ping fail.
			    if((tcp_connected == FALSE) && !(udp_connected & UPDATE_UDP_CONNECTED))				
						uip_arp_timer();
#endif				
      }
	  if (sending) /* exit as sending as happend. */
		return;
	}
	
#if DHCPC_EN
		//else 
		
		/* timer check link-chg-
		 *
		 */
		/* continue as sending as not happend. */
		if (timer_expired(&dhcp_timer)) 
		{
			timer_reset(&dhcp_timer);

			/* for now turn off the led when we start the dhcp process */
		#if 0
//			status = DM9051_Read_Reg(DM9051_NSR);
//			linkch = DM9051_Read_Reg(DM9051_ISR);
//			DM9051_Write_Reg(DM9051_ISR, 0x20);
//			
//			if((status & 0x40) && (linkch & 0x20)){ /* exit by return as under as renew. */
//				_printf("DHCPC Renew...!!!\r\n");
//				dhcpc_renew();
//				return;
//			}
		#else
		/* in timer found link-chang-up- (also startup first time dhcpc start.)
		 *
		 */
			//status = 1;
			linkch = 0;
			status = DM9051_Read_Reg(DM9051_NSR);
			if (status & NSR_LINKST) {
				if (!eth_netif_linkup) {
					eth_netif_linkup = 1;
					linkch = 1;
				}
			} else {
				if (eth_netif_linkup) {
					eth_netif_linkup = 0;
					linkch = 1;
				}
			}
			#if 1
			if (/*at_type.dhcpc_mode &&*/ eth_netif_linkup) {
//				if (eth_netif_startup_wait)
//					printf("up on linkchg=%u; dur %u\r\n", linkch, eth_netif_startup_wait);
				if (linkch) {
					/* LINKCHG up on: startup duration must NOT display, duration complete then can display for LINKCHG up */
#if 0
					if (!startup_duration_pull()) {
#endif
						printf("%s\n", "link up");
						tcpip_periodic_timer_watch_func();
#if 0
//						.atcmd_resp_cmd(" ");
//						.atcmd_resp_cmd("link up");
					} else
						eth_netif_startup_wait = 0;
#endif
					if (eth_start_ethdown_display == 1)
						atcmd_ready(); //atcmd_resp_cmd("rdy"/"OK");
					//['linkch' and 'eth_netif_linkup', Scheded here, also 'dhcpc_mode' Start a DHCP discover...] 
					if (at_type.dhcpc_mode)
						dhcpc_init_pack(&uip_ethaddr, 6);
				}
			} else {
//				if (eth_netif_startup_wait)
//					printf("down on linkchg=%u; dur %u\r\n", linkch, eth_netif_startup_wait);
				if (linkch || (!startup_duration_pull() && eth_start_ethdown_display == 0) ) {
					/* LINKCHG: startup duration NOT a helpful case to display LINKCHG down, duration complete then force to display LINKCHG down */

					if (at_type.role == 3 || at_type.role == 5)
						; //Frank test, Off/On IPSPP to quick into link-down if role 3
					else
						printf("%s\n", "link down");

					if (eth_start_ethdown_display == 0) {

						if (at_type.role == 3 || at_type.role == 5)
							; //Frank test, Off/On IPSPP to quick into link-down if role 3
						else
							atcmd_resp_cmd("link down");

						eth_start_ethdown_display = 1;
					} else
						eth_start_ethdown_display = 2;
				}
			}
			#endif
			#if 0
			if (linkch) {
				if (/*at_type.dhcpc_mode &&*/ eth_netif_linkup) {
					//['linkch', also 'dhcpc_mode' and 'linkup', Scheded here, Start a DHCP discover...] 
					_dhcpc_init_pack(&uip_ethaddr, 6);
				}
			}
			#endif
		#endif
		}
#endif //DHCPC_EN
		//else /* continue as sending/renew as not happend. */
		if (timer_expired(&keepalive_timer)) // keepalive function
		{
			 //printf("ka.tmr\r\n");
			//timer_reset(&keepalive_timer); // reset keepalive timer
			//if(at_type.role == ROLE_TCP_SERVER){} //Stone add for check TCP server can send Keepalive packet
			 timer_reset(&keepalive_timer); // reset keepalive timer
			 
			 if(at_type.keepalive_mode){
				ka_type.ka_no_data_tick++;
				
				if (!(ka_type.ka_no_data_tick % 50))
					printf("tick/fire: %u/%u\r\n", ka_type.ka_no_data_tick /5, (at_type.keepalive_no_data_period * 5)/ 5);

				//printf("7. keepalive tapdev_send %x %x \r\n", ka_type.ka_no_data_tick, at_type.keepalive_no_data_period);	 
				if(ka_type.ka_no_data_tick >= ((at_type.keepalive_no_data_period * 5)+(at_type.keepalive_no_data_period >> 2))) { //re-sync
				
//				 printf("ka.tick, at_type.fire: %u/%u (fire)\r\n", ka_type.ka_no_data_tick, at_type.keepalive_no_data_period);
				 
				ka_type.ka_no_data_tick = 0; //keepalive renew
				//ka_type.ka_noack_tick++;
				//ka_type.ka_noack_tick = 0;
			 
#if 0				
			 for(i = 0; i < UIP_CONNS; ++i) {
							//sprintf(at_tmpstr, "uip_conn[c].rport = %d HTONS(at_type.tcp_rport) = %d \r\n", uip_conn[i].rport, HTONS(at_type.tcp_rport));
	            //atcmd_resp_cmd(at_tmpstr);
							//if((uip_conns[c].lport == HTONS(at_type.t_lport)) || (uip_conn[c].rport == HTONS(at_type.tcp_rport))){
							if ((uip_conns[i].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED ) {
               if(uip_conn[i].rport == HTONS(at_type.tcp_rport))
				         {
					        //uip_gethostaddr(ipaddr);
                  //snprintf((char *)Keepalive_buf, UIP_APPDATA_SIZE,"POST /w/Rjcon.aspx HTTP/1.1\r\nHost: %d.%d.%d.%d Content-Length: 0\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n", 
                  //uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
		              //uip_send(Keepalive_buf, 122);	
								  //uip_periodic(c);
								  uip_poll_conn(i);   //Stone add for keepalive sequence error
							  //atcmd_resp_cmd("2. uip_poll_conn(c)\r\n");
							  }
							}
						}
#endif		
					for (i=0; i<UIP_CONNS ; i++){
				 
					 if ((uip_conns[i].tcpstateflags & UIP_TS_MASK) == UIP_ESTABLISHED ) {

//					  if(at_type.role == ROLE_TCP_SERVER){ //Stone add for check TCP server can send Keepalive packet	 
//						   if(uip_conns[i].lport == HTONS(at_type.tcp_rport))
//						   {
//							   uip_conn = &uip_conns[i];
//							   uip_send_keepalive();
//							   //uip_slen = uip_len; 						
//							   //uip_arp_out();
//							   //printf("1. BUF->destipaddr %x %x %x %x %x %x \r\n", BUF->dest.addr[0], BUF->dest.addr[1] , BUF->dest.addr[2], BUF->dest.addr[3], BUF->dest.addr[4], BUF->dest.addr[5]);	
//							   tapdev_send();
//							   ka_type.ka_no_data_tick = 0;								 
//							   printf("6. keepalive tapdev_send %x \r\n", uip_len);
//						   } 
//						}
						
//						if(at_type.role == ROLE_TCP_SCLIENT){ //Stone add for check TCP client can send Keepalive packet	 
//							if(uip_conns[i].rport == HTONS(at_type.tcp_rport))
//							{
//								 uip_conn = &uip_conns[i];
//								 uip_send_keepalive();
//								 //uip_slen = uip_len; 						
//								 //uip_arp_out();
//								//printf("1. BUF->destipaddr %x %x %x %x %x %x \r\n", BUF->dest.addr[0], BUF->dest.addr[1] , BUF->dest.addr[2], BUF->dest.addr[3], BUF->dest.addr[4], BUF->dest.addr[5]);	
//								tapdev_send();
//								 ka_type.ka_no_data_tick = 0;								 
//								 printf("6.1 keepalive tapdev_send %x \r\n", uip_len);
//							}
//						}
//			  
//						if(at_type.role == ROLE_TCP_DCLIENT){ //Stone add for check TCP client can send Keepalive packet	 
//						 if(uip_conns[i].rport == HTONS(at_type.tcp_rport))
//						 {
//							 uip_conn = &uip_conns[i];
//							 uip_send_keepalive();
//							 //uip_slen = uip_len; 						
//							 //uip_arp_out();
//							//printf("1. BUF->destipaddr %x %x %x %x %x %x \r\n", BUF->dest.addr[0], BUF->dest.addr[1] , BUF->dest.addr[2], BUF->dest.addr[3], BUF->dest.addr[4], BUF->dest.addr[5]);	
//							tapdev_send();
//							 ka_type.ka_no_data_tick = 0;								 
//							 printf("6.2 keepalive tapdev_send %x \r\n", uip_len);
//						 }
//						}
						
						if(at_type.role == ROLE_TCP_DCLIENT || at_type.role == ROLE_TCP_SCLIENT){ //Stone add for check TCP client can send Keepalive packet, joseph combine.
//							printf("tcp conn[%d] rport %u, local port %u\r\n", i, HTONS(uip_conns[i].rport), HTONS(uip_conns[i].lport));
							if(uip_conns[i].rport == HTONS(at_type.tcp_rport))
							{
								 uip_conn = &uip_conns[i];
								 uip_send_keepalive();					
								 //uip_arp_out();
								 tapdev_send();							 
//								 printf(".1 keepalive len %x, to %x %x %x %x %x %x\r\n", 
//										uip_len, BUF->dest.addr[0], BUF->dest.addr[1] , BUF->dest.addr[2], BUF->dest.addr[3], BUF->dest.addr[4], BUF->dest.addr[5]);
							}
						}				
						}		
#if 0						
			 uip_send_keepalive();
       uip_slen = uip_len; 						
       //uip_arp_out();
			 //printf("1. BUF->destipaddr %x %x %x %x %x %x \r\n", BUF->dest.addr[0], BUF->dest.addr[1] , BUF->dest.addr[2], BUF->dest.addr[3], BUF->dest.addr[4], BUF->dest.addr[5]);	
			 tapdev_send();				
       printf("6. keepalive tapdev_send %x \r\n", uip_len);
#endif			 
			 //printf("2. BUF->destipaddr %x %x %x %x %x %x \r\n", BUF->dest.addr[0], BUF->dest.addr[1] , BUF->dest.addr[2], BUF->dest.addr[3], BUF->dest.addr[4], BUF->dest.addr[5]);
#if 0							
//				keepalive_trans_function();
//				
//				if(ka_type.ka_send_count >= at_type.keepalive_send_count){ //if ka_send_count over user set closed current connect
//						atcmd_resp_cmd("keepalive no ack, close current connect...\r\n");
//						keepalive_init();
//						
//						recv_cmd_auto_disconnect(UIP_PROTO_TCP);
//					
//						check_DM9051_link = 0;
//						
//						if((at_type.role == ROLE_TCP_SERVER) || (at_type.role == ROLE_TCP_SCLIENT)){
//							EthernetInit_Done();
//						}
//					}
#endif
					
					}
					tcpip_ethtype_ip(); //Stone add for keepalive
				}
			 } //Stone add for TCP server send keepalive packet
		}
		
	#if 0
		//chack ehternet line change
		if(check_DM9051_link == 0){ /* continue as they as want show. */
			status = DM9051_Read_Reg(DM9051_NSR);
			linkch = DM9051_Read_Reg(DM9051_ISR);	
			DM9051_Write_Reg(DM9051_ISR, 0x20);
				
			//if((status & 0x40) && (linkch & 0x20)){
			if(linkch == 0xA3){
				atcmd_resp_cmd("Network line change...\r\n");
			}
		}
	#endif
}



void tcpip_process(void)
{
	tcpip_ethtype_ip();
	tcpip_periodic_timer();
}
