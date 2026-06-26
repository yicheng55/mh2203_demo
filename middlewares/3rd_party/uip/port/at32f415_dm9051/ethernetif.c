/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "at32f403a_407_board.h"
//#include "lwip/opt.h"
//#include "lwip/def.h"
//#include "lwip/mem.h"
//#include "lwip/pbuf.h"
//#include "lwip/sys.h"
//#include <lwip/stats.h>
//#include <lwip/snmp.h>
//#include "netif/etharp.h"
//#include "netif/ppp/pppoe.h"
//#include "err.h"
#include "uip_arp.h"
#include "developer_conf.h"
#include "ethernetif.h"
#include "dm9051_env.h" //"dm9051f_netconf.h" //(contain "lwip_driver_dm9051.h", Also "dm9051f.h")

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

#include <string.h>

/* Define those to better describe your network interface. */
#define IFNAME0 'd'
#define IFNAME1 'm'

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
//struct ethernetif
//{
//  struct eth_addr *ethaddr;
//  /* Add whatever per-interface state that is needed here. */
//  int unused;
//};

//#define OVERSIZE_LEN			PBUF_POOL_BUFSIZE // defined in "lwipopts.h" (JJ20201006)
//#define RXBUFF_OVERSIZE_LEN		(OVERSIZE_LEN+2)
//union {
//	u8 rx;
//	u8 tx;
//} EthBuff[RXBUFF_OVERSIZE_LEN]; //[Single Task project.]

#define EthBuff_rx	&uip_buf[0]
#define EthBuff_tx	&uip_buf[0]

//u8 Rx_Buff[RXBUFF_OVERSIZE_LEN];
//u8 Tx_Buff[RXBUFF_OVERSIZE_LEN];
//u8 MAC[6];

static u16_t
low_level_input(void)
{
  u16_t len;
  u8 *buffer = EthBuff_rx; //&EthBuff[0].rx; //Rx_Buff;

  len = dm9051_rx(buffer, UIP_BUFSIZE);
  if (!len)
	  return 0;
  
 #if 1
		//printf("ethernetif.c(pbuf:%s Eth Buffer total len %d)\r\n", "rx", len);
 #else
		showpbuf("rx", netif, p);
 #endif
  return len;
}

#if 1 //[to use]
static err_t
low_level_output(void)
{
  u8 *buffer = EthBuff_tx; //&EthBuff[0].tx; //Rx_Buff;
  uint16_t len = uip_len;
  dm9051_tx(buffer, len);
  return ERR_OK;
}
#endif

//static void
//low_level_init(uint8_t* macadd)
//{
//  printf("dm9051_init\r\n");
//  dm9051_init(macadd);
//}

u16_t
ethernetif_input(void)
{
  u16_t len;

  /* move received packet into a new pbuf */
  len = low_level_input();

  /* no packet could be read, silently ignore this */
  if (len == 0) 
	return 0; //ERR_INPROGRESS; //JJ.

  //err_t err = netif->input(p, netif);
  //if (err != ERR_OK)
  //{
  //  pbuf_free(p);
  //  p = NULL;
  //}
  return len;
}

err_t
ethernetif_output(void)
{
	return low_level_output();
}

//err_t
//ethernetif_init(uint8_t* macadd)
//{
////.  netif->linkoutput = low_level_output;
//  low_level_init(macadd);
//  return ERR_OK;
//}

//#if 0
//static void
//low_level_init(struct netif *netif)
//{
//	...
//#endif
