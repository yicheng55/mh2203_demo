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
//#include "atcommand.h"
//#include "etherbridge.h"


// AT Command 
struct at_funcation
{
	/* TCP and UDP Applicaiton Mode */
	uint8_t role;
	/* Switch DHCP Client Mode ON/OFF*/
	uint8_t dhcpc_mode;
	/* Host IP, netmask, Getwau */
	uip_ipaddr_t hostip;
	uip_ipaddr_t hostmask;
	uip_ipaddr_t hostgw;
	/* TCP Server Listen Port */
	uint16_t t_lport;
	/* TCP Client Connect Remote Address & Port */
	uip_ipaddr_t tcp_raddr;
	uint16_t tcp_rport;
	/* UDP Listen Port */
	uint16_t u_lport;
	/* UDP Client Remote Address & Remote Port */
	uip_ipaddr_t udp_raddr;
	uint16_t udp_rport;
	/* DNS Server IP */
	uint8_t dns_mode;
	uip_ipaddr_t dns_saddr;    //DNS Server IP
	/* Keep alive switch */
	uint8_t keepalive_mode;
	/* Setup Baudrate */
	uint32_t baudrate;
	uint8_t wordlen;
	uint8_t parity;
	uint8_t stop;
#if 0	
	/* TCP and UDP Applicaiton Mode */
	uint8_t role;
	/* Host IP, netmask, Getwau */
	uip_ipaddr_t hostip;
	uip_ipaddr_t hostmask;
	uip_ipaddr_t hostgw;
	/* TCP Server Listen Port */
	uint16_t t_lport;
	/* TCP Client Connect Remote Address & Port */
	uip_ipaddr_t tcp_raddr;
	uint16_t tcp_rport;
	
	/* UDP Listen Port */
	uint16_t u_lport;
	/* UDP Client Remote Address & Remote Port */
	uip_ipaddr_t udp_raddr;
	uint16_t udp_rport;
	
	/* DNS Server IP */
	uint8_t dns_mode;
	uip_ipaddr_t dns_saddr;    //DNS Server IP
	char dns_srvname[32]; //DNS Server Name
	char dns_srvpath[32]; //DNS Server Path
	uip_ipaddr_t reslove_addr; //resolve server name get ip address
	uint16_t dns_srvport;        //manual setup server port
	
	/* Keep alive switch */
	uint8_t keepalive_mode;
	uint8_t keepalive_send_period;
	uint8_t keepalive_no_data_period;
	uint8_t keepalive_send_count;
	
	/* Setup Baudrate */
	uint32_t baudrate;
	uint8_t wordlen;
	uint8_t parity;
	uint8_t stop;
	
	/* Setup Transparent lenght */
	uint16_t trans_len;
	
	/* For Customer Test count reset */
	uint16_t rst_count;
#endif
};

//DM9051 EEPROM 
struct eeprom_funcation {
	uint8_t  macaddr[6];
#if 0
	uint16_t autoload;
	uint16_t vid;
	uint16_t pid;
	uint16_t pincontrol;
	uint16_t wakeupcontrol2;
	uint8_t  control3;
#endif
};

struct at_funcation at_type = { 0 };
struct eeprom_funcation eeprom_type = { 0 };
char show_role[20] = { '$' };
uint8_t reconn_addr[16] = { 0 }; //remote connect address
uint16_t reconn_port;
extern struct uip_eth_addr uip_ethaddr;

/**
* @brief  This function handles Read AT DATA for Flash request.
* @param  None
* @retval None
*/
uint8_t Read_AT_DataFlash(void)
{
		uint8_t	mflag = 0;
		return mflag;
}
void show_role_msg(void)
{
	switch(at_type.role){
		case 0:
			sprintf(show_role, "(TCP Server)");
		break;
		case 1:
			sprintf(show_role, "(TCP Dynamic Client)");
		break;
		case 2:
			sprintf(show_role, "(TCP Static Client)");
		break;
		case 3:
			sprintf(show_role, "(UDP Server)");
		break;
		case 4:
			sprintf(show_role, "(UDP Dynamic Client)");
		break;
		case 5:
			sprintf(show_role, "(UDP Static Client)");
		break;
	}
}

//#define HTTPD_CGI_CALL(name, str, function) 
//static PT_THREAD(function(struct httpd_state *, char *)); 
//static const struct httpd_cgi_call name = {str, function}

static PT_THREAD(current_stats(struct httpd_state *, char *)); 
static PT_THREAD(parameter1_stats(struct httpd_state *, char *)); 
static PT_THREAD(parameter2_stats(struct httpd_state *, char *)); 
static PT_THREAD(hipconfig_stats(struct httpd_state *, char *)); 
static PT_THREAD(mipconfig_stats(struct httpd_state *, char *)); 
static PT_THREAD(gipconfig_stats(struct httpd_state *, char *)); 
static PT_THREAD(dnsipconfig_stats(struct httpd_state *, char *)); 
static PT_THREAD(ipconfig_page1_stats(struct httpd_state *, char *)); 
static const struct httpd_cgi_call current = {"current-stats", current_stats};
static const struct httpd_cgi_call parameter1 = {"parameter1-stats", parameter1_stats};
static const struct httpd_cgi_call parameter2 = {"parameter2-stats", parameter2_stats};
static const struct httpd_cgi_call hipconfig = {"hipconfig-stats", hipconfig_stats};
static const struct httpd_cgi_call mipconfig = {"mipconfig-stats", mipconfig_stats};
static const struct httpd_cgi_call gipconfig = {"gipconfig-stats", gipconfig_stats};
static const struct httpd_cgi_call dnsipconfig = {"dnsipconfig-stats", dnsipconfig_stats};
static const struct httpd_cgi_call ipconfig_page1 = {"ipconfig_page1-stats", ipconfig_page1_stats};

static PT_THREAD(file_list_stats(struct httpd_state *, char *)); 
static PT_THREAD(tcp_stats(struct httpd_state *, char *)); 
static const struct httpd_cgi_call file_list = {"file-list-cgi", file_list_stats};
static const struct httpd_cgi_call tcp = {"tcp-connectionss-cgi", tcp_stats};

/*HTTPD_CGI_CALL(current, "current-stats", current_stats);
HTTPD_CGI_CALL(parameter1, "parameter1-stats", parameter1_stats);
HTTPD_CGI_CALL(parameter2, "parameter2-stats", parameter2_stats);
HTTPD_CGI_CALL(hipconfig, "hipconfig-stats", hipconfig_stats);
HTTPD_CGI_CALL(mipconfig, "mipconfig-stats", mipconfig_stats);
HTTPD_CGI_CALL(gipconfig, "gipconfig-stats", gipconfig_stats);
HTTPD_CGI_CALL(dnsipconfig, "dnsipconfig-stats", dnsipconfig_stats);
HTTPD_CGI_CALL(ipconfig_page1, "ipconfig_page1-stats", ipconfig_page1_stats);*/

static const struct httpd_cgi_call *calls[] = { &current, &parameter1, &parameter2, 
												&hipconfig, &mipconfig, &gipconfig,
												&dnsipconfig, &ipconfig_page1, 
												&file_list, &tcp, NULL };

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
	struct httpd_state *s = (struct httpd_state *)arg;
 
	/*struct uip_conn *conn;
	conn = &uip_conns[s->count];*/
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"	<td height=\"18\" class=\"gr\"><div align=\"right\">MAC Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %02x:%02x:%02x:%02x:%02x:%02x    </td> \
			</tr> \
			</tr> \
			\r\n",
			0x00,0x60,0x6e,0x90,0x55,s->count
		);
	/*if(at_type.dhcpc_mode == 1){
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
	}*/
}

static
PT_THREAD(ipconfig_page1_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    //if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
		for(s->count = 0; s->count < UIP_CONNS; s->count+=3) {
			PSOCK_GENERATOR_SEND(&s->sout, generate_ipconfig_page1_stats, s);
		}
    //}
  //}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static unsigned short
generate_dnsipconfig_stats(void *arg)
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
			\r\n",
			0x00,0x60,0x6e,0x90,0x55,0x02
		);
	/*return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"<input maxlength=\"3\" size=\"3\" name=\"dip1\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"dip2\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"dip3\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"dip4\" value=\"%d\"/>\
		\r\n",
		// dns server ip address
		uip_ipaddr1(at_type.dns_saddr), uip_ipaddr2(at_type.dns_saddr),
			uip_ipaddr3(at_type.dns_saddr), uip_ipaddr4(at_type.dns_saddr)
		);*/
}

static
PT_THREAD(dnsipconfig_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    //if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_dnsipconfig_stats, s);
    //}
  //}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static unsigned short
generate_gipconfig_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
  
	Read_AT_DataFlash();

	conn = &uip_conns[s->count];
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"	<td height=\"18\" class=\"gr\"><div align=\"right\">MAC Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %02x:%02x:%02x:%02x:%02x:%02x    </td> \
			</tr> \
			</tr> \
			\r\n",
			0x00,0x60,0x6e,0x90,0x55,0x03
		);
	/*return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"<input maxlength=\"3\" size=\"3\" name=\"gip1\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"gip2\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"gip3\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"gip4\" value=\"%d\"/>\
		\r\n",
		// Getway address
		uip_ipaddr1(at_type.hostgw), uip_ipaddr2(at_type.hostgw),
			uip_ipaddr3(at_type.hostgw), uip_ipaddr4(at_type.hostgw)
		);*/
}

static
PT_THREAD(gipconfig_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    //if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_gipconfig_stats, s);
    //}
  //}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static unsigned short
generate_mipconfig_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
  
	Read_AT_DataFlash();

	conn = &uip_conns[s->count];
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"	<td height=\"18\" class=\"gr\"><div align=\"right\">MAC Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %02x:%02x:%02x:%02x:%02x:%02x    </td> \
			</tr> \
			</tr> \
			\r\n",
			0x00,0x60,0x6e,0x90,0x55,0x04
		);
	/*return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"<input maxlength=\"3\" size=\"3\" name=\"mip1\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"mip2\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"mip3\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"mip4\" value=\"%d\"/>\
		\r\n",
		// network mask 
		uip_ipaddr1(at_type.hostmask), uip_ipaddr2(at_type.hostmask),
			uip_ipaddr3(at_type.hostmask), uip_ipaddr4(at_type.hostmask)
		);*/
}

static
PT_THREAD(mipconfig_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    //if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_mipconfig_stats, s);
    //}
  //}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static unsigned short
generate_hipconfig_stats(void *arg)
{
	struct uip_conn *conn;
	struct httpd_state *s = (struct httpd_state *)arg;
  
	Read_AT_DataFlash();

	conn = &uip_conns[s->count];
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"	<td height=\"18\" class=\"gr\"><div align=\"right\">MAC Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %02x:%02x:%02x:%02x:%02x:%02x    </td> \
			</tr> \
			</tr> \
			\r\n",
			0x00,0x60,0x6e,0x90,0x55,0x05
		);
	/*return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"<input maxlength=\"3\" size=\"3\" name=\"sip1\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"sip2\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"sip3\" value=\"%d\"/>. \
			<input maxlength=\"3\" size=\"3\" name=\"sip4\" value=\"%d\"/>\
		\r\n",
		// IP address
		uip_ipaddr1(at_type.hostip), uip_ipaddr2(at_type.hostip),
			uip_ipaddr3(at_type.hostip), uip_ipaddr4(at_type.hostip)
		);*/
}

static
PT_THREAD(hipconfig_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    //if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_hipconfig_stats, s);
    //}
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
			\r\n",
			0x00,0x60,0x6e,0x90,0x55,0x06
		);
  /*return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
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
		(unsigned int)eeprom_type.macaddr[0], (unsigned int)eeprom_type.macaddr[1], 
			(unsigned int)eeprom_type.macaddr[2], (unsigned int)eeprom_type.macaddr[3], 
				(unsigned int)eeprom_type.macaddr[4], (unsigned int)eeprom_type.macaddr[5],
		// IP address
		uip_ipaddr1(at_type.hostip), uip_ipaddr2(at_type.hostip),
			uip_ipaddr3(at_type.hostip), uip_ipaddr4(at_type.hostip),
		// network mask 
		uip_ipaddr1(at_type.hostmask), uip_ipaddr2(at_type.hostmask),
			uip_ipaddr3(at_type.hostmask), uip_ipaddr4(at_type.hostmask),
		// Getway address
		uip_ipaddr1(at_type.hostgw), uip_ipaddr2(at_type.hostgw),
			uip_ipaddr3(at_type.hostgw), uip_ipaddr4(at_type.hostgw),
		//TCP Server Listen Port
		at_type.t_lport,
		//TCP Client Static Connect
		uip_ipaddr1(at_type.tcp_raddr), uip_ipaddr2(at_type.tcp_raddr),
			uip_ipaddr3(at_type.tcp_raddr), uip_ipaddr4(at_type.tcp_raddr),
				at_type.tcp_rport, 
		//UDP　Server Listen Port
		at_type.u_lport,
		//UDP Client Static Connect
		uip_ipaddr1(at_type.udp_raddr), uip_ipaddr2(at_type.udp_raddr),
			uip_ipaddr3(at_type.udp_raddr), uip_ipaddr4(at_type.udp_raddr),
				 at_type.udp_rport,
		//AT UART Baudrate
		at_type.baudrate, at_type.wordlen ,at_type.parity, at_type.stop
		);*/
}

static
PT_THREAD(parameter2_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    //if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_parameter2_stats, s);
    //}
  //}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static unsigned short
generate_parameter1_stats(void *arg)
{
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
  
	show_role_msg();
	Read_AT_DataFlash();
	
  conn = &uip_conns[s->count];
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
			"	<td height=\"18\" class=\"gr\"><div align=\"right\">MAC Address：</div></td> \
			<td><div align=\"left\"> \
			</select>  %02x:%02x:%02x:%02x:%02x:%02x    </td> \
			</tr> \
			</tr> \
			\r\n",
			0x00,0x60,0x6e,0x90,0x55,0x07
		);
  /*return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
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
		at_type.role, show_role, 
		//DHCP Client
		at_type.dhcpc_mode,
		//Keepalive mode
		at_type.keepalive_mode,
		//DNS　Client mode
		at_type.dns_mode
		);*/
}

static
PT_THREAD(parameter1_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    //if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_parameter1_stats, s);
    //}
  //}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static unsigned short
generate_current_stats(void *arg)
{
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
	uip_ipaddr_t ipaddr;;
    
	show_role_msg();
	
  conn = &uip_conns[s->count];
	
	//test.
	uip_ipaddr(ipaddr, 8, 8, 8, 8);	
	memset(reconn_addr, 0, sizeof(reconn_addr));
	sprintf((char *)reconn_addr, "%d.%d.%d.%d",uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr),
			uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
  
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
			\r\n",
			//Role 
			at_type.role, show_role, 
			//DHCP Client
			at_type.dhcpc_mode, 
			// MAC address
			(unsigned int)uip_ethaddr.addr[0], (unsigned int)uip_ethaddr.addr[1], 
				(unsigned int)uip_ethaddr.addr[2], (unsigned int)uip_ethaddr.addr[3], 
					(unsigned int)uip_ethaddr.addr[4], (unsigned int)uip_ethaddr.addr[5],
			// IP address
			(uip_hostaddr[0] & 0xff), ((uip_hostaddr[0] >> 8) ),
				(uip_hostaddr[1] & 0xff), ((uip_hostaddr[1] >> 8))
		);
}

static
PT_THREAD(current_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    //if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
		for(s->count = 0; s->count < UIP_CONNS; s->count += 2) {
			PSOCK_GENERATOR_SEND(&s->sout, generate_current_stats, s);
		}
    //}
  //}

  PSOCK_END(&s->sout);
}
/*-------------*/
/*-------------*/
/*-------------*/

//Not add httpd-debug.c to this project,
//Below two lines setup partial of "httpd-debug.c"
//  [declare var] const struct httpd_fsdata_file *fs_ROOT_HTML = NULL;
//  [assign data] httpd_report_root_tree();
//= 
#include "httpd-debug.h"
const struct httpd_fsdata_file *fs_ROOT_HTML = NULL;

static unsigned short
generate_file_list_stats(void *arg)
{
	int n = 0;
	int fsc = 9; //9; //temp 'HTTPD_FS_NUMFILES'
  //struct httpd_state *s = (struct httpd_state *)arg;
	int fc = 0;
  struct httpd_fsdata_file_noconst *f = (struct httpd_fsdata_file_noconst *)arg;
  char *appdata = (char *) uip_appdata;
//if (1){
  //struct httpd_fsdata_file_noconst *f;
  //for(f = (struct httpd_fsdata_file_noconst *)fs_ROOT_HTML;
	  //f != NULL;
	  //f = (struct httpd_fsdata_file_noconst *)f->next) {
		for( ; fc < fsc && f != NULL; f = (struct httpd_fsdata_file_noconst *)f->next) {
		  n += snprintf((char *)(appdata+n), UIP_APPDATA_SIZE,
			  "<li id=\"lia\"> %s </li>",
			/*"<tr><td> %s </td></tr>\r\n",*/
			  f->name);
		  fc++;
		}
		  
		  //n += snprintf((char *)(appdata+n), UIP_APPDATA_SIZE,
			//  "<tr><td> %s </td></tr>\r\n", f->name);
		  
		  //n += snprintf((char *)(appdata+n), UIP_APPDATA_SIZE,
			//  "<tr><td> %s </td></tr>\r\n", f->name);
		  return  n;
  //}
//}
}
static
PT_THREAD(file_list_stats(struct httpd_state *s, char *ptr))
{
	struct httpd_fsdata_file_noconst *f;
  
  PSOCK_BEGIN(&s->sout);
	
	httpd_report_root_tree();
	f = (struct httpd_fsdata_file_noconst *)fs_ROOT_HTML;
	PSOCK_GENERATOR_SEND(&s->sout, generate_file_list_stats, f);
	//f = (struct httpd_fsdata_file_noconst *)f->next;
	//PSOCK_GENERATOR_SEND(&s->sout, generate_file_list_stats, f);
  //for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    //if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      //PSOCK_GENERATOR_SEND(&s->sout, generate_file_list_stats, s);
		/*if (0){
		  struct httpd_fsdata_file_noconst *f;
		  httpd_report_root_tree();
		  for(f = (struct httpd_fsdata_file_noconst *)fs_ROOT_HTML;
			  f != NULL;
			  f = (struct httpd_fsdata_file_noconst *)f->next) {
				  PSOCK_GENERATOR_SEND(&s->sout, generate_file_list_stats, f);
				  //printf(" %s\n", f->name);
		  }
		}*/
    //}
  //}

  PSOCK_END(&s->sout);
}

/*-------------*/
static const char closed[] =   /*  "CLOSED",*/
{0x43, 0x4c, 0x4f, 0x53, 0x45, 0x44, 0};
static const char syn_rcvd[] = /*  "SYN-RCVD",*/
{0x53, 0x59, 0x4e, 0x2d, 0x52, 0x43, 0x56,
 0x44,  0};
static const char syn_sent[] = /*  "SYN-SENT",*/
{0x53, 0x59, 0x4e, 0x2d, 0x53, 0x45, 0x4e,
 0x54,  0};
static const char established[] = /*  "ESTABLISHED",*/
{0x45, 0x53, 0x54, 0x41, 0x42, 0x4c, 0x49, 0x53, 0x48,
 0x45, 0x44, 0};
static const char fin_wait_1[] = /*  "FIN-WAIT-1",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
 0x54, 0x2d, 0x31, 0};
static const char fin_wait_2[] = /*  "FIN-WAIT-2",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
 0x54, 0x2d, 0x32, 0};
static const char closing[] = /*  "CLOSING",*/
{0x43, 0x4c, 0x4f, 0x53, 0x49,
 0x4e, 0x47, 0};
static const char time_wait[] = /*  "TIME-WAIT,"*/
{0x54, 0x49, 0x4d, 0x45, 0x2d, 0x57, 0x41,
 0x49, 0x54, 0};
static const char last_ack[] = /*  "LAST-ACK"*/
{0x4c, 0x41, 0x53, 0x54, 0x2d, 0x41, 0x43,
 0x4b, 0};

static const char *states[] = {
  closed,
  syn_rcvd,
  syn_sent,
  established,
  fin_wait_1,
  fin_wait_2,
  closing,
  time_wait,
  last_ack};

static unsigned short
generate_tcp_stats(void *arg)
{
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
    
  conn = &uip_conns[s->count];
  return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		 "<tr><td>%d</td><td>%u.%u.%u.%u:%u</td><td>%s</td><td>%u</td><td>%u</td><td>%c %c</td></tr>\r\n",
		 htons(conn->lport),
		 htons(conn->ripaddr[0]) >> 8,
		 htons(conn->ripaddr[0]) & 0xff,
		 htons(conn->ripaddr[1]) >> 8,
		 htons(conn->ripaddr[1]) & 0xff,
		 htons(conn->rport),
		 states[conn->tcpstateflags & UIP_TS_MASK],
		 conn->nrtx,
		 conn->timer,
		 (uip_outstanding(conn))? '*':' ',
		 (uip_stopped(conn))? '!':' ');
}
static
PT_THREAD(tcp_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    //if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_tcp_stats, s);
    //}
  }

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** @} */
