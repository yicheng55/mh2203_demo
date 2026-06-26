/**
 * \addtogroup httpd
 * @{
 */

/**
 * \file
 *         Web server script interface
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Copyright (c) 2001-2006, Adam Dunkels.
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
 * $Id: httpd-cgi.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 *
 */

#include "uip.h"
#include "psock.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include <stdio.h>
#include <string.h>

#if WEB_SCRIPT_EXAMPLE //[Design 'withto' for your pages!]
/* Jerry Modify */
HTTPD_CGI_CALL(sys, "sys-stats", sys_stats);
static const struct httpd_cgi_call *calls[] = {&sys, NULL };

/* joseph add show system message */
#if UIP_FIXEDETHADDR
extern const struct uip_eth_addr uip_ethaddr;
#else
extern struct uip_eth_addr uip_ethaddr;
#endif
/* jerry add show system message */
static unsigned short
generate_sys_stats(void *arg)
{
  struct httpd_state *s = (struct httpd_state *)arg;
 
  return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<tr> <td >MCU Chip : </td><td style=\"text-align: left\">ARTERY AT32F413RCT7 Chip</td> </tr> \
				<tr> <td >Network Chip : </td> <td style=\"text-align: left\" >DAVICOM DM9051</td> </tr> \
					<tr> <td >MAC Address :</td> <td style=\"text-align: left\">%02x:%02x:%02x:%02x:%02x:%02x</td> </tr> \
						<tr> <td >IP Address : </td> <td style=\"text-align: left\"> %d.%d.%d.%d </td> </tr> \
							<tr> <td >Network Mask : </td> <td style=\"text-align: left\"> %d.%d.%d.%d </td> </tr> \
								<tr><td >Getway : </td> <td style=\"text-align: left\"> %d.%d.%d.%d </td> </tr>\r\n",
			/* MAC address */
			(unsigned int)uip_ethaddr.addr[0], (unsigned int)uip_ethaddr.addr[1], 
				(unsigned int)uip_ethaddr.addr[2], (unsigned int)uip_ethaddr.addr[3], 
					(unsigned int)uip_ethaddr.addr[4], (unsigned int)uip_ethaddr.addr[5],
			/* IP address */
			(uip_hostaddr[0]&0xff), ((uip_hostaddr[0] >> 8)&0xff),
				(uip_hostaddr[1]&0xff), ((uip_hostaddr[1] >> 8)&0xff),
			/* network mask */
			(uip_netmask[0]&0xff), ((uip_netmask[0] >> 8)&0xff),
				(uip_netmask[1]&0xff), ((uip_netmask[1] >> 8)&0xff),
			/* Getway address */
			(uip_draddr[0]&0xff), ((uip_draddr[0] >> 8)&0xff),
				(uip_draddr[1]&0xff), ((uip_draddr[1] >> 8)&0xff)
				);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(sys_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_sys_stats, s);
    }
  //}
		
  PSOCK_END(&s->sout);
}
#else
static const struct httpd_cgi_call *calls[] = { NULL };
#endif

#if WEB_SCRIPT_CODE
/*---------------------------------------------------------------------------*/
static
PT_THREAD(nullfunction(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_cgifunction
httpd_cgi(char *name)
{
  const struct httpd_cgi_call **f;

  /* Find the matching name in the table, return the function. */
  for(f = calls; *f != NULL; ++f) {
    if(strncmp((*f)->name, name, strlen((*f)->name)) == 0) {
      return (*f)->function;
    }
  }
  return nullfunction;
}
#endif
