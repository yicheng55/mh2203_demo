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

#include "includes.h"
#include "uip.h"
#include "psock.h"
#include "webopts.h" //20240510.Joseph
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include <stdio.h>
#include <string.h>
#include "atcommand.h"
#include "etherbridge.h"

extern struct at_funcation at_show;
extern struct eeprom_funcation eeprom_show;

//uint16_t user_mode(uint16_t showpdata, uint16_t showdata)
//{
//	return user_sel_mode ? showpdata : showdata;
//}

//uint8_t user_sel_mode = 1; // 1: showp_mode, 0: show_mode
//#define USR_MODE(va, vb) ((user_sel_mode) ? va : vb)
#define USR_MODE(va, vb) (va)

HTTPD_CGI_CALL(current, "current-stats", current_stats);

HTTPD_CGI_CALL(parameter1, "parameter1-stats", parameter1_stats);
HTTPD_CGI_CALL(parameter2, "parameter2-stats", parameter2_stats);

HTTPD_CGI_CALL(hipconfig, "hipconfig-stats", hipconfig_stats);
HTTPD_CGI_CALL(mipconfig, "mipconfig-stats", mipconfig_stats);
HTTPD_CGI_CALL(gipconfig, "gipconfig-stats", gipconfig_stats);
HTTPD_CGI_CALL(tcps_lport, "tcps_lport-stats", tcps_lport_stats);
HTTPD_CGI_CALL(tcpc_raddr, "tcpc_raddr-stats", tcpc_raddr_stats);

HTTPD_CGI_CALL(DNS_Mode, "DNS_Mode-stats", DNS_Mode_stats);
HTTPD_CGI_CALL(dnsipconfig, "dnsipconfig-stats", dnsipconfig_stats);
HTTPD_CGI_CALL(DNS_srvname, "DNS_srvname-stats", DNS_srvname_stats);

HTTPD_CGI_CALL(role_mode, "role_mode-stats", role_mode_stats);
HTTPD_CGI_CALL(ipconfig_page1, "ipconfig_page1-stats", ipconfig_page1_stats);

static const struct httpd_cgi_call *calls[] = { &current, &parameter1, &parameter2, 
																									&hipconfig, &mipconfig, &gipconfig,
																										&dnsipconfig, &ipconfig_page1, 
																											&DNS_Mode, &DNS_srvname, &role_mode, 
																												&tcpc_raddr, &tcps_lport, NULL };

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
/*---------------------------------------------------------------------------*/
static unsigned short
generate_ipconfig_page1_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
 
	conn = &uip_conns[s->count];
	if(USR_MODE(at_show.dhcpc_mode, at_type.dhcpc_mode) == 1){
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<option value=\"1\" selected=\"selected\">DHCP</option>\
			 <option value=\"0\">Static IP</option> \
			\r\n"
		);
	}else{
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<option value=\"1\">DHCP</option>\
			 <option value=\"0\" selected=\"selected\">Static IP</option> \
			\r\n"
		);
	}
}

static
PT_THREAD(ipconfig_page1_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_ipconfig_page1_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}

static unsigned short
generate_role_mode_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
 
	conn = &uip_conns[s->count];
	if(USR_MODE(at_show.role, at_type.role) == 0){
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<option value=\"0\" selected=\"selected\">TCP Server</option> \
				<option value=\"2\">TCP Static Client</option> \
				<option value=\"3\">UDP Server</option> \
				<option value=\"5\">UDP Static Client</option> \
				</select>%s\
			\r\n",
			reconn_addr
		);
	}else if(USR_MODE(at_show.role, at_type.role) == 2){
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<option value=\"0\">TCP Server</option> \
				<option value=\"2\" selected=\"selected\">TCP Static Client</option> \
				<option value=\"3\">UDP Server</option> \
				<option value=\"5\">UDP Static Client</option> \
				</select>%s\
			\r\n",
			reconn_addr
		);
	}else if(USR_MODE(at_show.role, at_type.role) == 3){
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<option value=\"0\">TCP Server</option> \
				<option value=\"2\">TCP Static Client</option> \
				<option value=\"3\" selected=\"selected\">UDP Server</option> \
				<option value=\"5\">UDP Static Client</option> \
				</select>%s\
			\r\n",
			reconn_addr
		);
	}else if(USR_MODE(at_show.role, at_type.role) == 5){
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<option value=\"0\">TCP Server</option> \
				<option value=\"2\">TCP Static Client</option> \
				<option value=\"3\">UDP Server</option> \
				<option value=\"5\" selected=\"selected\">UDP Static Client</option> \
				</select>%s\
			\r\n",
			reconn_addr
		);
	}
}

static
PT_THREAD(role_mode_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_role_mode_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
unsigned short add_cgi_shtml(char *buff, char *appends); //called to appends. string.

static unsigned short
generate_DNS_srvname_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
 
	conn = &uip_conns[s->count];
	
#ifndef WEB_BAUD_UPDATE
//[for TEST cgi inter-pcs]
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"<input type=\"text\" name=\"srvname\" value=\"%s\"> \
		\r\n",
		USR_MODE(at_show.dns_srvname, at_type.dns_srvname)
	); // : <input maxlength=\"5\" size=\"5\" name=\"dsp\" value=\"%d\"/> (1~65535)
	   //USR_MODE(at_show.dns_srvport, at_type.dns_srvport)
#else 
//[TESTing cgi inter-pcs]
	do {
		unsigned short len = snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<input type=\"text\" name=\"srvname\" value=\"%s\"> \
			\r\n",
			USR_MODE(at_show.dns_srvname, at_type.dns_srvname)
		); // : <input maxlength=\"5\" size=\"5\" name=\"dsp\" value=\"%d\"/>\ (1~65535)
		   //USR_MODE(at_show.dns_srvport, at_type.dns_srvport)
		
		len += add_cgi_shtml((char *)uip_appdata + len, "</td></tr>");
		len += add_cgi_shtml((char *)uip_appdata + len, "<tr>");
		len += add_cgi_shtml((char *)uip_appdata + len, "<td height=\"20\" bordercolor=\"#F0F0F0\" bgcolor=\"#FFFFFF\" class=\"gr\"><div align=\"right\">Baud Rate：</div>");
		#if 0
			<td><div align="left">
				<select size="1" name="bd">
				<option value="1200" selected="selected">1200</option>
				<option value="2400">2400</option>
				<option value="4800">4800</option>
				<option value="9600">9600</option>
				<option value="14400">14400</option>
				<option value="19200">19200</option>
				<option value="56000">56000</option>
				<option value="57600">57600</option>
				<option value="38400">38400</option>
				<option value="115200">115200</option>
				<option value="194000">194000</option>
				</select>
				<select size="1" name="wln">
				<option value="7">7</option>
				<option value="8" selected="selected">8</option>
				<option value="9">9</option>
				</select>
				<select name="pty">
				<option value="0" selected="selected">None</option>            
				<option value="1">Odd</option>
				<option value="2">Even</option>
				</select>
				<select size="1" name="stop">
				<option value="1" selected="selected">1</option>
				<option value="3">1.5</option>
				<option value="2">2</option>
				</select>
				</div>
		#endif
		//Connect-to original "</td></tr>"
		return len;
	} while(0);
#endif
}

unsigned short add_cgi_shtml(char *buff, char *appends) {
	unsigned short len = strlen(appends);
	memcpy(buff, appends, len);
	return len;
}

static
PT_THREAD(DNS_srvname_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_DNS_srvname_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}

static unsigned short
generate_dnsipconfig_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
 
	conn = &uip_conns[s->count];
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"<input maxlength=\"3\" size=\"3\" name=\"dip1\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"dip2\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"dip3\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"dip4\" value=\"%d\"/>\
		\r\n",
		// dns server ip address
			uip_ipaddr1(USR_MODE(at_show.dns_saddr,at_type.dns_saddr)), 
			uip_ipaddr2(USR_MODE(at_show.dns_saddr,at_type.dns_saddr)),
			uip_ipaddr3(USR_MODE(at_show.dns_saddr,at_type.dns_saddr)), 
			uip_ipaddr4(USR_MODE(at_show.dns_saddr,at_type.dns_saddr))
		);
}

static
PT_THREAD(dnsipconfig_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_dnsipconfig_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}

static unsigned short
generate_DNS_Mode_stats(void *arg)
{
//	struct uip_conn *conn;
//	struct httpd_state *s = (struct httpd_state *)arg;
	//.NO USED (arg)
//	conn = &uip_conns[s->count];
	
	if(USR_MODE(at_show.dns_mode,at_type.dns_mode) == 1){
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<option value=\"1\" selected=\"selected\">ON</option>\
			 <option value=\"0\">OFF</option>\
			\r\n"
		);
	}else{
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<option value=\"1\">ON</option>\
			 <option value=\"0\" selected=\"selected\">OFF</option>\
			\r\n"
		);
	}
}

static
PT_THREAD(DNS_Mode_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_DNS_Mode_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/

static unsigned short
generate_tcpc_raddr_stats(void *arg)
{
//	struct uip_conn *conn;
//	struct httpd_state *s = (struct httpd_state *)arg;
//	conn = &uip_conns[s->count];
	//Read_AT_DataFlash();
	//.NO USED (arg)
	
	if((USR_MODE(at_show.role, at_type.role) == 0) || (USR_MODE(at_show.role, at_type.role) == 2)){
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
				"<input maxlength=\"3\" size=\"3\" name=\"trip1\" value=\"%d\"/>. \
					<input maxlength=\"3\" size=\"3\" name=\"trip2\" value=\"%d\"/>. \
					<input maxlength=\"3\" size=\"3\" name=\"trip3\" value=\"%d\"/>. \
					<input maxlength=\"3\" size=\"3\" name=\"trip4\" value=\"%d\"/> : \
					<input maxlength=\"5\" size=\"5\" name=\"trp\" value=\"%d\"/>\
				\r\n",
				// IP address
				uip_ipaddr1(USR_MODE(at_show.tcp_raddr, at_type.tcp_raddr)), 
				uip_ipaddr2(USR_MODE(at_show.tcp_raddr, at_type.tcp_raddr)),
				uip_ipaddr3(USR_MODE(at_show.tcp_raddr, at_type.tcp_raddr)), 
				uip_ipaddr4(USR_MODE(at_show.tcp_raddr, at_type.tcp_raddr)),
				USR_MODE(at_show.tcp_rport, at_type.tcp_rport)
			);
	}else if((USR_MODE(at_show.role, at_type.role) == 3) || (USR_MODE(at_show.role, at_type.role) == 5)){
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<input maxlength=\"3\" size=\"3\" name=\"trip1\" value=\"%d\"/>. \
				<input maxlength=\"3\" size=\"3\" name=\"trip2\" value=\"%d\"/>. \
				<input maxlength=\"3\" size=\"3\" name=\"trip3\" value=\"%d\"/>. \
				<input maxlength=\"3\" size=\"3\" name=\"trip4\" value=\"%d\"/> : \
				<input maxlength=\"5\" size=\"5\" name=\"trp\" value=\"%d\"/>\
			\r\n",
			// IP address
			uip_ipaddr1(USR_MODE(at_show.udp_raddr, at_type.udp_raddr)),
			uip_ipaddr2(USR_MODE(at_show.udp_raddr, at_type.udp_raddr)),
			uip_ipaddr3(USR_MODE(at_show.udp_raddr, at_type.udp_raddr)),
			uip_ipaddr4(USR_MODE(at_show.udp_raddr, at_type.udp_raddr)),
			USR_MODE(at_show.udp_rport, at_type.udp_rport)
		);
	}
}

static
PT_THREAD(tcpc_raddr_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_tcpc_raddr_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}

static unsigned short
generate_tcps_lport_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
  
	//Read_AT_DataFlash();

	conn = &uip_conns[s->count];
	if((USR_MODE(at_show.role, at_type.role) == 0) || (USR_MODE(at_show.role, at_type.role) == 2)){
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<input maxlength=\"5\" size=\"5\" name=\"tlp\" value=\"%d\"/>\
						(1~65535)\
			\r\n",
				USR_MODE(at_show.t_lport, at_type.t_lport)
			);
	}else if((USR_MODE(at_show.role, at_type.role) == 3) || (USR_MODE(at_show.role, at_type.role) == 5)){
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"<input maxlength=\"5\" size=\"5\" name=\"tlp\" value=\"%d\"/>\
			(1~65535)\
			\r\n",
			USR_MODE(at_show.u_lport, at_type.u_lport)
		);
	}
}

static
PT_THREAD(tcps_lport_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_tcps_lport_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}

static unsigned short
generate_gipconfig_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
  
	//Read_AT_DataFlash();

	conn = &uip_conns[s->count];
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"<input maxlength=\"3\" size=\"3\" name=\"gip1\" value=\"%d\"/>. \
		<input maxlength=\"3\" size=\"3\" name=\"gip2\" value=\"%d\"/>. \
		<input maxlength=\"3\" size=\"3\" name=\"gip3\" value=\"%d\"/>. \
		<input maxlength=\"3\" size=\"3\" name=\"gip4\" value=\"%d\"/>\
		\r\n",
		// Getway address
			uip_ipaddr1(USR_MODE(at_show.hostgw, at_type.hostgw)),
			uip_ipaddr2(USR_MODE(at_show.hostgw, at_type.hostgw)),
			uip_ipaddr3(USR_MODE(at_show.hostgw, at_type.hostgw)),
			uip_ipaddr4(USR_MODE(at_show.hostgw, at_type.hostgw))
		);
}

static
PT_THREAD(gipconfig_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_gipconfig_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}

static unsigned short
generate_mipconfig_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
  
	//Read_AT_DataFlash();

	conn = &uip_conns[s->count];
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"<input maxlength=\"3\" size=\"3\" name=\"mip1\" value=\"%d\"/>. \
		<input maxlength=\"3\" size=\"3\" name=\"mip2\" value=\"%d\"/>. \
		<input maxlength=\"3\" size=\"3\" name=\"mip3\" value=\"%d\"/>. \
		<input maxlength=\"3\" size=\"3\" name=\"mip4\" value=\"%d\"/>\
		\r\n",
		// network mask 
			uip_ipaddr1(USR_MODE(at_show.hostmask, at_type.hostmask)), 
			uip_ipaddr2(USR_MODE(at_show.hostmask, at_type.hostmask)),
			uip_ipaddr3(USR_MODE(at_show.hostmask, at_type.hostmask)),
			uip_ipaddr4(USR_MODE(at_show.hostmask, at_type.hostmask))
		);
}

static
PT_THREAD(mipconfig_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_mipconfig_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}

static unsigned short
generate_hipconfig_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
  
	//Read_AT_DataFlash();

	conn = &uip_conns[s->count];
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"<input maxlength=\"3\" size=\"3\" name=\"sip1\" value=\"%d\"/>. \
		<input maxlength=\"3\" size=\"3\" name=\"sip2\" value=\"%d\"/>. \
		<input maxlength=\"3\" size=\"3\" name=\"sip3\" value=\"%d\"/>. \
		<input maxlength=\"3\" size=\"3\" name=\"sip4\" value=\"%d\"/>\
		\r\n",
		// IP address
			uip_ipaddr1(USR_MODE(at_show.hostip, at_type.hostip)),
			uip_ipaddr2(USR_MODE(at_show.hostip, at_type.hostip)),
			uip_ipaddr3(USR_MODE(at_show.hostip, at_type.hostip)),
			uip_ipaddr4(USR_MODE(at_show.hostip, at_type.hostip))
		);
}

static
PT_THREAD(hipconfig_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_hipconfig_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static unsigned short
generate_parameter2_stats(void *arg)
{
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
	
  conn = &uip_conns[s->count];
  return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		 "	<td height=\"18\" class=\"gr\"><div align=\"right\">MAC Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %02x:%02x:%02x:%02x:%02x:%02x    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">IP Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d.%d.%d.%d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">Netmask Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d.%d.%d.%d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">Getway Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d.%d.%d.%d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">TCP Listen Port：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">TCP Static CONN：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d.%d.%d.%d:%d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">UDP Listen Port：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">UDP Static CONN：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d.%d.%d.%d:%d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">AT UART Baudrate：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d.%d.%d.%d    </td> \
			</tr> \
			</tr> \
			\r\n",
			(unsigned int)USR_MODE(eeprom_show.macaddr[0], eeprom_type.macaddr[0]), 
			(unsigned int)USR_MODE(eeprom_show.macaddr[1], eeprom_type.macaddr[1]), 
			(unsigned int)USR_MODE(eeprom_show.macaddr[2], eeprom_type.macaddr[2]), 
			(unsigned int)USR_MODE(eeprom_show.macaddr[3], eeprom_type.macaddr[3]), 
			(unsigned int)USR_MODE(eeprom_show.macaddr[4], eeprom_type.macaddr[4]), 
			(unsigned int)USR_MODE(eeprom_show.macaddr[5], eeprom_type.macaddr[5]),
		// IP address
			uip_ipaddr1(USR_MODE(at_show.hostip, at_type.hostip)), 
			uip_ipaddr2(USR_MODE(at_show.hostip, at_type.hostip)),
			uip_ipaddr3(USR_MODE(at_show.hostip, at_type.hostip)), 
			uip_ipaddr4(USR_MODE(at_show.hostip, at_type.hostip)),
		// network mask 
			uip_ipaddr1(USR_MODE(at_show.hostmask, at_type.hostmask)), 
			uip_ipaddr2(USR_MODE(at_show.hostmask, at_type.hostmask)),
			uip_ipaddr3(USR_MODE(at_show.hostmask, at_type.hostmask)), 
			uip_ipaddr4(USR_MODE(at_show.hostmask, at_type.hostmask)),
		// Getway address
			uip_ipaddr1(USR_MODE(at_show.hostgw, at_type.hostgw)), 
			uip_ipaddr2(USR_MODE(at_show.hostgw, at_type.hostgw)),
			uip_ipaddr3(USR_MODE(at_show.hostgw, at_type.hostgw)), 
			uip_ipaddr4(USR_MODE(at_show.hostgw, at_type.hostgw)),
		//TCP Server Listen Port
			USR_MODE(at_show.t_lport, at_type.t_lport),
		//TCP Client Static Connect
			uip_ipaddr1(USR_MODE(at_show.tcp_raddr, at_type.tcp_raddr)), 
			uip_ipaddr2(USR_MODE(at_show.tcp_raddr, at_type.tcp_raddr)),
			uip_ipaddr3(USR_MODE(at_show.tcp_raddr, at_type.tcp_raddr)), 
			uip_ipaddr4(USR_MODE(at_show.tcp_raddr, at_type.tcp_raddr)),
			USR_MODE(at_show.tcp_rport, at_type.tcp_rport), 
		//UDP　Server Listen Port
			USR_MODE(at_show.u_lport, at_type.u_lport),
		//UDP Client Static Connect
			uip_ipaddr1(USR_MODE(at_show.udp_raddr, at_type.udp_raddr)), 
			uip_ipaddr2(USR_MODE(at_show.udp_raddr, at_type.udp_raddr)),
			uip_ipaddr3(USR_MODE(at_show.udp_raddr, at_type.udp_raddr)), 
			uip_ipaddr4(USR_MODE(at_show.udp_raddr, at_type.udp_raddr)),
			USR_MODE(at_show.udp_rport, at_type.udp_rport),
		//AT UART Baudrate
			USR_MODE(at_show.baudrate, at_type.baudrate), 
			USR_MODE(at_show.wordlen, at_type.wordlen),
			USR_MODE(at_show.parity, at_type.parity), 
			USR_MODE(at_show.stop, at_type.stop)
		);
}

static
PT_THREAD(parameter2_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_parameter2_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static unsigned short
generate_parameter1_stats(void *arg)
{
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
#if 0
	make_role_msg();
#endif
	//Read_AT_DataFlash();
	
  conn = &uip_conns[s->count];
  return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		 "<td height=\"18\" class=\"gr\"><div align=\"right\">Role：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d (%s)    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">DHCP Mode：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">Keepalive mode：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">DNS Client mode：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d    </td> \
			</tr> \
			</tr> \
			\r\n",
		//Role 
		USR_MODE(at_show.role, at_type.role), 
		role_desc_string(USR_MODE(at_show.role, at_type.role)), /* refer to 'show_role' */
		//DHCP Client
		USR_MODE(at_show.dhcpc_mode, at_type.dhcpc_mode),
		//Keepalive mode
		USR_MODE(at_show.keepalive_mode, at_type.keepalive_mode),
		//DNS　Client mode
		USR_MODE(at_show.dns_mode, at_type.dns_mode)
		);
}

static
PT_THREAD(parameter1_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_parameter1_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static unsigned short
generate_current_stats(void *arg)
{
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
#if 0
	make_role_msg();
#endif
  conn = &uip_conns[s->count];
  
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		 "<td height=\"18\" class=\"gr\"><div align=\"right\">Role：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d %s    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">DHCP Mode：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">DNS Client Mode：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">MAC Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %02x:%02x:%02x:%02x:%02x:%02x    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">IP Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d.%d.%d.%d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">Netmask Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d.%d.%d.%d    </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">Getway Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %d.%d.%d.%d  </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">DNS Found name：</div></td> \
			<td><div align=\"left\"> \
			</select> %s </td> \
			</tr> \
			</tr> \
			<td height=\"18\" class=\"gr\"><div align=\"right\">Module Status：</div></td> \
			<td><div align=\"left\"> \
			</select>%s\
			</td> \
			</tr> \
			</tr> \
			\r\n",
		//Role 
		USR_MODE(at_show.role, at_type.role), 
		role_desc_string(USR_MODE(at_show.role, at_type.role)), /* refer to "show_role" */
		//DHCP Client
		USR_MODE(at_show.dhcpc_mode, at_type.dhcpc_mode), 
		//DNS Client Mode
		USR_MODE(at_show.dns_mode, at_type.dns_mode), 
		// MAC address
		(unsigned int)uip_ethaddr.addr[0], 
		(unsigned int)uip_ethaddr.addr[1], 
		(unsigned int)uip_ethaddr.addr[2], 
		(unsigned int)uip_ethaddr.addr[3], 
		(unsigned int)uip_ethaddr.addr[4], 
		(unsigned int)uip_ethaddr.addr[5],
		// IP address
		(uip_hostaddr[0] & 0xff), 
		((uip_hostaddr[0] >> 8)),
		(uip_hostaddr[1] & 0xff), 
		((uip_hostaddr[1] >> 8)),
		// network mask
		(uip_netmask[0] & 0xff),
		((uip_netmask[0] >> 8)),
		(uip_netmask[1] & 0xff), 
		((uip_netmask[1] >> 8)),
		// Getway address
		(uip_draddr[0] & 0xff), 
		((uip_draddr[0] >> 8)),
		(uip_draddr[1] & 0xff), 
		((uip_draddr[1] >> 8)), 
		//DNS status message
		dhs_staus_msg,
		//Remote connection IP & Port
		reconn_addr
		);
}

static
PT_THREAD(current_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_current_stats, s);
    }
  //}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** @} */
