/**
 * \addtogroup uip
 * @{
 */

/**
 * \defgroup uiparp uIP Address Resolution Protocol
 * @{
 *
 * The Address Resolution Protocol ARP is used for mapping between IP
 * addresses and link level addresses such as the Ethernet MAC
 * addresses. ARP uses broadcast queries to ask for the link level
 * address of a known IP address and the host which is configured with
 * the IP address for which the query was meant, will respond with its
 * link level address.
 *
 * \note This ARP implementation only supports Ethernet.
 */
 
/**
 * \file
 * Implementation of the ARP Address Resolution Protocol.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 */

/*
 * Copyright (c) 2001-2003, Adam Dunkels.
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
 * 3. The name of the author may not be used to endorse or promote
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
 * $Id: uip_arp.c,v 1.8 2006/06/02 23:36:21 adam Exp $
 *
 */

#include "includes.h"
#include "uip_arp.h"
#include "atcommand.h"
#include <string.h>
#include <stdio.h>

#define ARP_REQUEST 1
#define ARP_REPLY   2

#define ARP_HWTYPE_ETH 1

static const struct uip_eth_addr broadcast_ethaddr =
  {{0xff,0xff,0xff,0xff,0xff,0xff}};
static const u16_t broadcast_ipaddr[2] = {0xffff,0xffff};

//static
struct arp_entry arp_table[UIP_ARPTAB_SIZE];
static u16_t ipaddr[2];
static u8_t i, c;

static u8_t arptime;
static u8_t tmpage;

#define BUF   ((struct arp_hdr *)&uip_buf[0])
#define IPBUF ((struct ethip_hdr *)&uip_buf[0])
/*-----------------------------------------------------------------------------------*/
/**
 * Initialize the ARP module.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
uip_arp_init(void)
{
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    memset(arp_table[i].ipaddr, 0, 4);
  }
}
/*-----------------------------------------------------------------------------------*/
/**
 * Periodic ARP processing function.
 *
 * This function performs periodic timer processing in the ARP module
 * and should be called at regular intervals. The recommended interval
 * is 10 seconds between the calls.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
uip_arp_timer(void)
{
  struct arp_entry *tabptr;
  
  ++arptime;
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    tabptr = &arp_table[i];
    if((tabptr->ipaddr[0] | tabptr->ipaddr[1]) != 0 &&
       arptime - tabptr->time >= UIP_ARP_MAXAGE) {
      memset(tabptr->ipaddr, 0, 4);
    }
  }

}
/*-----------------------------------------------------------------------------------*/

void uip_arp_update(u16_t *ipaddr, struct uip_eth_addr *ethaddr)
{
  register struct arp_entry *tabptr;
	int k=0;
  /* Walk through the ARP mapping table and try to find an entry to
     update. If none is found, the IP -> MAC address mapping is
     inserted in the ARP table. */
  for(k = 0; k < UIP_ARPTAB_SIZE; ++k) {

    tabptr = &arp_table[k];
    /* Only check those entries that are actually in use. */
    if(tabptr->ipaddr[0] != 0 &&
       tabptr->ipaddr[1] != 0) {

      /* Check if the source IP address of the incoming packet matches
         the IP address in this ARP table entry. */
      if(ipaddr[0] == tabptr->ipaddr[0] &&
	 ipaddr[1] == tabptr->ipaddr[1]) {
	 
	/* An old entry found, update this and return. */
	memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
	tabptr->time = arptime;
  //printf("0. uip_arp_update k %x tabptr->ipaddr %x %x \r\n", k, tabptr->ipaddr[0], tabptr->ipaddr[1]);
	return;
      }
    }
  }

  /* If we get here, no existing ARP table entry was found, so we
     create one. */

  /* First, we try to find an unused entry in the ARP table. */
  for(k = 0; k < UIP_ARPTAB_SIZE; ++k) {
    tabptr = &arp_table[k];
    if(tabptr->ipaddr[0] == 0 &&
       tabptr->ipaddr[1] == 0) {
      break;
    }
  }
  //printf("1. uip_arp_update k %x \r\n", k);
  /* If no unused entry is found, we try to find the oldest entry and
     throw it away. */
  if(k == UIP_ARPTAB_SIZE) {
    tmpage = 0;
    c = 0;
    for(k = 0; k < UIP_ARPTAB_SIZE; ++k) {
      tabptr = &arp_table[k];
      if(arptime - tabptr->time > tmpage) {
	tmpage = arptime - tabptr->time;
	c = k;
      }
    }
    k = c;
    tabptr = &arp_table[k];
  }
//printf("2. uip_arp_update k %x tabptr->ipaddr %x %x \r\n", k, tabptr->ipaddr[0], tabptr->ipaddr[1]);
  /* Now, i is the ARP table entry which we will fill with the new
     information. */
  memcpy(tabptr->ipaddr, ipaddr, 4);
  memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
  tabptr->time = arptime;
  arp_need_new(); //arp update
}
/*-----------------------------------------------------------------------------------*/
/**
 * ARP processing for incoming IP packets
 *
 * This function should be called by the device driver when an IP
 * packet has been received. The function will check if the address is
 * in the ARP cache, and if so the ARP cache entry will be
 * refreshed. If no ARP cache entry was found, a new one is created.
 *
 * This function expects an IP packet with a prepended Ethernet header
 * in the uip_buf[] buffer, and the length of the packet in the global
 * variable uip_len.
 */
/*-----------------------------------------------------------------------------------*/
#if 0
void
uip_arp_ipin(void)
{
  uip_len -= sizeof(struct uip_eth_hdr);
	
  /* Only insert/update an entry if the source IP address of the
     incoming IP packet comes from a host on the local network. */
  if((IPBUF->srcipaddr[0] & uip_netmask[0]) !=
     (uip_hostaddr[0] & uip_netmask[0])) {
    return;
  }
  if((IPBUF->srcipaddr[1] & uip_netmask[1]) !=
     (uip_hostaddr[1] & uip_netmask[1])) {
    return;
  }
  uip_arp_update(IPBUF->srcipaddr, &(IPBUF->ethhdr.src));
  
  return;
}
#endif /* 0 */
/*-----------------------------------------------------------------------------------*/
/**
 * ARP processing for incoming ARP packets.
 *
 * This function should be called by the device driver when an ARP
 * packet has been received. The function will act differently
 * depending on the ARP packet type: if it is a reply for a request
 * that we previously sent out, the ARP cache will be filled in with
 * the values from the ARP reply. If the incoming ARP packet is an ARP
 * request for our IP address, an ARP reply packet is created and put
 * into the uip_buf[] buffer.
 *
 * When the function returns, the value of the global variable uip_len
 * indicates whether the device driver should send out a packet or
 * not. If uip_len is zero, no packet should be sent. If uip_len is
 * non-zero, it contains the length of the outbound packet that is
 * present in the uip_buf[] buffer.
 *
 * This function expects an ARP packet with a prepended Ethernet
 * header in the uip_buf[] buffer, and the length of the packet in the
 * global variable uip_len.
 */
/*-----------------------------------------------------------------------------------*/
void
uip_arp_arpin(void)
{
//  int j=0;
  if(uip_len < sizeof(struct arp_hdr)) {
    uip_len = 0;
    return;
  }
  uip_len = 0;
  
  switch(BUF->opcode) {
  case HTONS(ARP_REQUEST):
  	//printf("ARP_REQUEST...\r\n");
    /* ARP request. If it asked for our address, we send out a
       reply. */
	  //printf("ARP_REQUEST BUF->dipaddr %x %x \r\n", BUF->dipaddr[0], BUF->dipaddr[1]);
	
    if(uip_ipaddr_cmp(BUF->dipaddr, uip_hostaddr)) {
      /* First, we register the one who made the request in our ARP
	 table, since it is likely that we will do more communication
	 with this host in the future. */
#if 1			
      uip_arp_update(BUF->sipaddr, &BUF->shwaddr);
#endif
			
      /* The reply opcode is 2. */
      BUF->opcode = HTONS(2);

      memcpy(BUF->dhwaddr.addr, BUF->shwaddr.addr, 6);
      memcpy(BUF->shwaddr.addr, uip_ethaddr.addr, 6);
      memcpy(BUF->ethhdr.src.addr, uip_ethaddr.addr, 6);
      memcpy(BUF->ethhdr.dest.addr, BUF->dhwaddr.addr, 6);
      
      BUF->dipaddr[0] = BUF->sipaddr[0];
      BUF->dipaddr[1] = BUF->sipaddr[1];
      BUF->sipaddr[0] = uip_hostaddr[0];
      BUF->sipaddr[1] = uip_hostaddr[1];

      BUF->ethhdr.type = HTONS(UIP_ETHTYPE_ARP);
      uip_len = sizeof(struct arp_hdr);
     			
    }
    break;
  case HTONS(ARP_REPLY):
    /* ARP reply. We insert or update the ARP table if it was meant
       for us. */
	  //printf("ARP_REPLY...\r\n");
	  
    //printf("ARP_REPLY BUF->dipaddr %x %x \r\n", BUF->dipaddr[0], BUF->dipaddr[1]);
	  //printf("ARP_REPLY BUF->shwaddr %x:%x:%x:%x:%x:%x\r\n", BUF->shwaddr.addr[0], BUF->shwaddr.addr[1], BUF->shwaddr.addr[2], BUF->shwaddr.addr[3], BUF->shwaddr.addr[4], BUF->shwaddr.addr[5]);	
	
#if 1	
    if(uip_ipaddr_cmp(BUF->dipaddr, uip_hostaddr)) {
      uip_arp_update(BUF->sipaddr, &BUF->shwaddr);
	  
	  display_arp("arp_updated");
//	  display_udp_conn("arp_updated,show udp conn,");
    }
#endif		
    break;
  }

  return;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Prepend Ethernet header to an outbound IP packet and see if we need
 * to send out an ARP request.
 *
 * This function should be called before sending out an IP packet. The
 * function checks the destination IP address of the IP packet to see
 * what Ethernet MAC address that should be used as a destination MAC
 * address on the Ethernet.
 *
 * If the destination IP address is in the local network (determined
 * by logical ANDing of netmask and our IP address), the function
 * checks the ARP cache to see if an entry for the destination IP
 * address is found. If so, an Ethernet header is prepended and the
 * function returns. If no ARP cache entry is found for the
 * destination IP address, the packet in the uip_buf[] is replaced by
 * an ARP request packet for the IP address. The IP packet is dropped
 * and it is assumed that they higher level protocols (e.g., TCP)
 * eventually will retransmit the dropped packet.
 *
 * If the destination IP address is not on the local network, the IP
 * address of the default router is used instead.
 *
 * When the function returns, a packet is present in the uip_buf[]
 * buffer, and the length of the packet is in the global variable
 * uip_len.
 */
/*-----------------------------------------------------------------------------------*/
void
uip_arp_out(void)
{
  struct arp_entry *tabptr;
	int k;
	k=0;
  
  /* Find the destination IP address in the ARP table and construct
     the Ethernet header. If the destination IP addres isn't on the
     local network, we use the default router's IP address instead.

     If not ARP table entry is found, we overwrite the original IP
     packet with an ARP request for the IP address. */

  /* First check if destination is a local broadcast. */
  if(uip_ipaddr_cmp(IPBUF->destipaddr, broadcast_ipaddr)) {
    memcpy(IPBUF->ethhdr.dest.addr, broadcast_ethaddr.addr, 6);
  } else {
    /* Check if the destination address is on the local network. */
    if(!uip_ipaddr_maskcmp(IPBUF->destipaddr, uip_hostaddr, uip_netmask)) {
      /* Destination address was not on the local network, so we need to
	 use the default router's IP address instead of the destination
	 address when determining the MAC address. */
#if 0			
			printf("1. uip_arp_out IPBUF->destipaddr %x %x \r\n", IPBUF->destipaddr[0], IPBUF->destipaddr[1]);
#endif			
      uip_ipaddr_copy(ipaddr, uip_draddr);
#if 0			
			printf("1-1. uip_arp_out ipaddr %x %x \r\n", ipaddr[0], ipaddr[1]);
#endif			
    } else {
      /* Else, we use the destination IP address. */
#if 0			
			printf("2. uip_arp_out ipaddr %x %x \r\n", ipaddr[0], ipaddr[1]);
#endif			
      uip_ipaddr_copy(ipaddr, IPBUF->destipaddr);
#if 0			
			printf("2-1. uip_arp_out ipaddr %x %x \r\n", ipaddr[0], ipaddr[1]);
#endif			
    }
      
    for(k = 0; k < UIP_ARPTAB_SIZE; ++k) {
      tabptr = &arp_table[k];
#if 0	  
	  printf("X.%d tabptr->ethaddr.addr ", UIP_ARPTAB_SIZE);
	  for (i=0; i<6; i++)
		 printf("%02x ", tabptr->ethaddr.addr[i]);
	  printf("tabptr->ipaddr ");
	  printf("%d.%d.%d.%d\r\n", uip_ipaddr1(tabptr->ipaddr), uip_ipaddr2(tabptr->ipaddr),
				uip_ipaddr3(tabptr->ipaddr), uip_ipaddr4(tabptr->ipaddr));
#endif
      if(uip_ipaddr_cmp(ipaddr, tabptr->ipaddr)) {
#if 1
//		printf("ARP.OUT arp FOUND\r\n");
#endif				
		break;
      }
    }
    
		
    if(k == UIP_ARPTAB_SIZE) {
      /* The destination address was not in our ARP table, so we
	 overwrite the IP packet with an ARP request. */
      //if(!uip_ipaddr_cmp(ipaddr, at_type.tcp_raddr)){
			
//      printf("1. ipaddr %x %x uip_len %u (uip_slen %u)\r\n", ipaddr[0], ipaddr[1], uip_len, uip_slen);
//      if(uip_slen == 0x0){
      memset(BUF->ethhdr.dest.addr, 0xff, 6);
      memset(BUF->dhwaddr.addr, 0x00, 6);
      memcpy(BUF->ethhdr.src.addr, uip_ethaddr.addr, 6);
      memcpy(BUF->shwaddr.addr, uip_ethaddr.addr, 6);
    
      uip_ipaddr_copy(BUF->dipaddr, ipaddr);
      uip_ipaddr_copy(BUF->sipaddr, uip_hostaddr);
      BUF->opcode = HTONS(ARP_REQUEST); /* ARP request. */
      BUF->hwtype = HTONS(ARP_HWTYPE_ETH);
      BUF->protocol = HTONS(UIP_ETHTYPE_IP);
      BUF->hwlen = 6;
      BUF->protolen = 4;
      BUF->ethhdr.type = HTONS(UIP_ETHTYPE_ARP);

      uip_appdata = &uip_buf[UIP_TCPIP_HLEN + UIP_LLH_LEN];
    
      uip_len = sizeof(struct arp_hdr);
//      printf("X.%d arp_out %d.%d.%d.%d uip_len %u (uip_slen %u)\r\n", UIP_ARPTAB_SIZE, uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr),
//				uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr), uip_len, uip_slen);
      return;
//      }				
    }
    
		
    /* Build an ethernet header. */
    memcpy(IPBUF->ethhdr.dest.addr, tabptr->ethaddr.addr, 6);

#if 0		
		//Stone add for UART RX DMA error...
		if ((IPBUF->ethhdr.dest.addr[0] == 0x00) && (IPBUF->ethhdr.dest.addr[1] == 0x00) &&
			 (IPBUF->ethhdr.dest.addr[2] == 0x00) && (IPBUF->ethhdr.dest.addr[3] == 0x00) &&
		   (IPBUF->ethhdr.dest.addr[4] == 0x00) && (IPBUF->ethhdr.dest.addr[5] == 0x00) )
		{
			tabptr = &arp_table[0];
			memcpy(IPBUF->ethhdr.dest.addr, tabptr->ethaddr.addr, 6);
			printf("#### k=%x uip_slen %x Update IPBUF->ethhdr.dest.addr ####\r\n", k, uip_slen);
			printf("arp_table[0] => ipaddr %x %x \r\n", tabptr->ipaddr[0], tabptr->ipaddr[1]);
			
			tabptr = &arp_table[1];
			printf("arp_table[1] => ipaddr %x %x \r\n", tabptr->ipaddr[0], tabptr->ipaddr[1]);
			
		}
#endif		
  }

#if 0	
	if (i==8){
		  printf("2. uip_arp_out arp_table %x ipaddr %x %x \r\n", i, ipaddr[0], ipaddr[1]);
	}
#endif
	
  memcpy(IPBUF->ethhdr.src.addr, uip_ethaddr.addr, 6);
  
  IPBUF->ethhdr.type = HTONS(UIP_ETHTYPE_IP);

  uip_len += sizeof(struct uip_eth_hdr);
	
#if 0
	printf("\r\n 2. k=%x IPBUF->ethhdr.dest.addr ", k);	
	for (k=0; k<6; k++)
	 printf("%x ", IPBUF->ethhdr.dest.addr[k]);
  printf("\r\n");	
#endif
	
#if 0
	tabptr = &arp_table[0];
	printf("\r\n 2. tabptr->ethaddr.addr ");	
	for (k=0; k<6; k++)
	 printf("%x ", tabptr->ethaddr.addr[k]);
	printf("\r\n");
#endif	

#if 0
   printf("uip_arp_out udp_connected %x _atcmd_flag %x \r\n", (udp_connected & UPDATE_UDP_CONNECTED), _atcmd_flag);
#endif
#if 0	
	printf("\r\n 2. ipaddr %x %x uip_len %x \r\n", ipaddr[0], ipaddr[1], uip_len);
#endif	
	
}
/*-----------------------------------------------------------------------------------*/

/** @} */
/** @} */
