/**
 * \addtogroup apps
 * @{
 */

/**
 * \defgroup httpd Web server
 * @{
 * The uIP web server is a very simplistic implementation of an HTTP
 * server. It can serve web pages and files from a read-only ROM
 * filesystem, and provides a very small scripting language.

 */

/**
 * \file
 *         Web server
 * \author
 *         Adam Dunkels <adam@sics.se>
 */


/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: httpd.c,v 1.2 2006/06/11 21:46:38 adam Exp $
 */

#include  <stdio.h>
#include "uip.h"
#include "httpd.h"
#include "httpd-fs.h"
#include "httpd-cgi.h"
#include "http-strings.h"

#include <string.h>
#include "web_led.h"

#define STATE_WAITING 0
#define STATE_OUTPUT  1

#define ISO_nl      0x0a
#define ISO_space   0x20
#define ISO_bang    0x21
#define ISO_percent 0x25
#define ISO_period  0x2e
#define ISO_slash   0x2f
#define ISO_colon   0x3a

void DM9051_DoRXDUMP(char *hdstr, uint8_t *payload, uint16_t len, uint16_t dumpsize, int beginlinefeed); //(print_packet_code.h)

/*---------------------------------------------------------------------------*/
static unsigned short
generate_part_of_file(void *state)
{
  struct httpd_state *s = (struct httpd_state *)state;

  if(s->file.len > uip_mss()) {
    s->len = uip_mss();
  } else {
    s->len = s->file.len;
  }
	//.printf(";file_gen_part_send: len %d\r\n", s->len);
  memcpy(uip_appdata, s->file.data, s->len);
  
  return s->len;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_file(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
  
	//.printf(";send_file(%s): len %d\r\n", s->filename, s->file.len);
  do {
    PSOCK_GENERATOR_SEND(&s->sout, generate_part_of_file, s);
    s->file.len -= s->len;
    s->file.data += s->len;
  } while(s->file.len > 0);
      
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
#if WEB_SCRIPT_CODE
static
PT_THREAD(send_part_of_file(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  PSOCK_SEND(&s->sout, s->file.data, s->len);
  
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static void
next_scriptstate(struct httpd_state *s)
{
  char *p;
  p = strchr(s->scriptptr, ISO_nl) + 1;
  s->scriptlen -= (unsigned short)(p - s->scriptptr);
  s->scriptptr = p;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_script(struct httpd_state *s))
{
  char *ptr;
  
  PT_BEGIN(&s->scriptpt);


  while(s->file.len > 0) {

    /* Check if we should start executing a script. */
    if(*s->file.data == ISO_percent &&
       *(s->file.data + 1) == ISO_bang) {
      s->scriptptr = s->file.data + 3;
      s->scriptlen = s->file.len - 3;
      if(*(s->scriptptr - 1) == ISO_colon) {
		httpd_fs_open(s->scriptptr + 1, &s->file); //(s->scriptptr + 1) as s->filename
		printf(";_script;send file=%s (%d -> %d)\r\n", s->scriptptr + 1, 
					HTONS(uip_conn->lport), HTONS(uip_conn->rport)); //[JJDbg], (s->scriptptr + 1) as s->filename
		PT_WAIT_THREAD(&s->scriptpt, send_file(s));
      } else {
		PT_WAIT_THREAD(&s->scriptpt,
				   httpd_cgi(s->scriptptr)(s, s->scriptptr));
      }
      next_scriptstate(s);
      
      /* The script is over, so we reset the pointers and continue
	 sending the rest of the file. */
      s->file.data = s->scriptptr;
      s->file.len = s->scriptlen;
    } else {
      /* See if we find the start of script marker in the block of HTML
	 to be sent. */

      if(s->file.len > uip_mss()) {
	s->len = uip_mss();
      } else {
	s->len = s->file.len;
      }

      if(*s->file.data == ISO_percent) {
	ptr = strchr(s->file.data + 1, ISO_percent);
      } else {
	ptr = strchr(s->file.data, ISO_percent);
      }
      if(ptr != NULL &&
	 ptr != s->file.data) {
	s->len = (int)(ptr - s->file.data);
	if(s->len >= uip_mss()) {
	  s->len = uip_mss();
	}
      }
      PT_WAIT_THREAD(&s->scriptpt, send_part_of_file(s));
      s->file.data += s->len;
      s->file.len -= s->len;
      
    }
  }
  
  PT_END(&s->scriptpt);
}
#endif
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr))
{
  char *ptr;
	const char *head_type;

  PSOCK_BEGIN(&s->sout);
	
//printf(";[Header]Len=%d [out1]= %s (%d -> %d)\r\n", strlen(statushdr), statushdr, HTONS(uip_conn->lport), HTONS(uip_conn->rport));
  PSOCK_SEND_STR(&s->sout, statushdr);

  ptr = strrchr(s->filename, ISO_period);
	
  if(ptr == NULL) 
		head_type = http_content_type_binary;
	else if(strncmp(http_html, ptr, 5) == 0 ||
	    strncmp(http_shtml, ptr, 6) == 0)
		head_type = http_content_type_html;
	else if(strncmp(http_css, ptr, 4) == 0)
		head_type = http_content_type_css;
	else if(strncmp(http_png, ptr, 4) == 0)
		head_type =http_content_type_png ;
	else if(strncmp(http_gif, ptr, 4) == 0)
		head_type = http_content_type_gif;
	else if(strncmp(http_jpg, ptr, 4) == 0)
		head_type = http_content_type_jpg;
	else
		head_type = http_content_type_plain;
	printf(";[Header]Len=%d [out2]= %s (%d -> %d)\r\n", strlen(head_type), head_type, HTONS(uip_conn->lport), HTONS(uip_conn->rport));
	
  if(ptr == NULL) {
    PSOCK_SEND_STR(&s->sout, http_content_type_binary);
  } else if(strncmp(http_html, ptr, 5) == 0 ||
	    strncmp(http_shtml, ptr, 6) == 0) {
    PSOCK_SEND_STR(&s->sout, http_content_type_html);
  } else if(strncmp(http_css, ptr, 4) == 0) {
    PSOCK_SEND_STR(&s->sout, http_content_type_css);
  } else if(strncmp(http_png, ptr, 4) == 0) {
    PSOCK_SEND_STR(&s->sout, http_content_type_png);
  } else if(strncmp(http_gif, ptr, 4) == 0) {
    PSOCK_SEND_STR(&s->sout, http_content_type_gif);
  } else if(strncmp(http_jpg, ptr, 4) == 0) {
    PSOCK_SEND_STR(&s->sout, http_content_type_jpg);
  } else {
    PSOCK_SEND_STR(&s->sout, http_content_type_plain);
  }
  PSOCK_END(&s->sout);
}

/*
(INPUT'S)
*/
#define LED2BIT_ON	(1 << 0) //0x01
#define LED3BIT_ON	(1 << 1) //0x02
#define LED4BIT_ON	(1 << 2) //0x04
char ledflag = 0;

char ledflag_of_input(void)
{
	return ledflag;
}
static void update_onoff_led(void)
{
#if 0	
	if (ledflag & LED2BIT_ON)
		AT32_LEDn_ON(LED2);
	else
		AT32_LEDn_OFF(LED2);
		
	if (ledflag & LED3BIT_ON)
		AT32_LEDn_ON(LED3);
	else
		AT32_LEDn_OFF(LED3);
	
	if (ledflag & LED4BIT_ON)
		AT32_LEDn_ON(LED4);
	else
		AT32_LEDn_OFF(LED4);
#endif
}
static void Set_LED_mode_button(char *inputb) //=process_submit_button()
{
	ledflag = 0;
	if (strncmp(inputb, "/method=get?", 12) == 0) {
		//.DM9051_DoRXDUMP("  [inputb]", (uint8_t *) inputb, strlen(inputb), strlen(inputb), 2); //ASCII
		//.DM9051_DoRXDUMP("  [inputb(29 char)]", (uint8_t *) inputb, 29, 29, 2); //ASCII
		if (strncmp(&inputb[12], "led=2", 5) == 0){
			ledflag |= LED2BIT_ON;
			//printf("[2 lc%d]PSOCK_READTO: led2\r\n", s->sin.pt.lc);
		}
		if (strncmp(&inputb[12], "led=3", 5) == 0){
			ledflag |= LED3BIT_ON;
			//printf("[2 lc%d]PSOCK_READTO: led3\r\n", s->sin.pt.lc);
		}
		if (strncmp(&inputb[12], "led=4", 5) == 0){
			ledflag |= LED4BIT_ON;	
			//printf("[2 lc%d]PSOCK_READTO: led4\r\n", s->sin.pt.lc);
		}
		/*
		//inputb[12]= 'l'
		//inputb[13]= 'e'
		//inputb[14]= 'd'
		//inputb[15]= '='
		if (strncmp(&inputb[12], "led=", 4) == 0) {
			if (inputb[16]== '2')
				ledflag |= 1;
			if (inputb[16]== '3')
				ledflag |= 2;
			if (inputb[16]== '4')
				ledflag |= 4;
		}*/
		//inputb[17]= '&'
		if (strncmp(&inputb[18], "led=", 4) == 0) {
			//if (inputb[22]== '2' || inputb[22]== '3' || inputb[22]== '4')
			//	printf("[2 lc%d]PSOCK_READTO: led%c\r\n", s->sin.pt.lc, inputb[16]);
			if (inputb[22]== '2')
				ledflag |= 1;
			if (inputb[22]== '3')
				ledflag |= 2;
			if (inputb[22]== '4')
				ledflag |= 4;
		}
		//inputb[23]= '&'
		if (strncmp(&inputb[24], "led=", 4) == 0) {
			//if (inputb[28]== '2' || inputb[28]== '3' || inputb[28]== '4')
			//	printf("[2 lc%d]PSOCK_READTO: led%c\r\n", s->sin.pt.lc, inputb[16]);
			if (inputb[28]== '2')
				ledflag |= 1;
			if (inputb[28]== '3')
				ledflag |= 2;
			if (inputb[28]== '4')
				ledflag |= 4;
		}
		update_onoff_led();
	}
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  char *ptr;
  
  PT_BEGIN(&s->outputpt);
 
	//.printf(";handle_output _fs_open: %s\r\n", s->filename);	

  #if (HOMEPAGE_SEL_CFG == 3)
  DataPrintf( (uint8_t *)s->filename, strlen(s->filename));
  { int i; for (i=0; (i< strlen(s->filename) || i < sizeof(s->filename)) ; i++) SER_PUTC(s->filename[i]); 
	  SER_PUTC('\n');
  } //[Debug]
  #endif
	
  if(!httpd_fs_open(s->filename, &s->file)) {
    httpd_fs_open(http_404_html, &s->file);
    strcpy(s->filename, http_404_html);
	  
	#if (HOMEPAGE_SEL_CFG == 3)
	strcpy(s->filename, HOME_HTML); //[Temp.fixed.]
	#endif
	  
    PT_WAIT_THREAD(&s->outputpt,
		   send_headers(s,
		   http_header_404));
		
		printf(";_output;send file=%s (%d -> %d)\r\n", http_404_html, HTONS(uip_conn->lport), HTONS(uip_conn->rport)); //[JJDbg]
    PT_WAIT_THREAD(&s->outputpt,
		   send_file(s));
  } else {

#define CODE_TEST_ZERO	0 // 0 to disable it (suit to HTTP/1.1)!!!
#if CODE_TEST_ZERO
		const char *p_http_header;
		//[JJAdd] FOR ADC page --- not here (in "httpd-fs.c" httpd_fs_open())~
		//"s->filename"
		
		p_http_header = http_header_200;
		#if 1	//[JJTest.404 Not FOUND.Add]//[JJAdd] FOR direct 404 Not Found!
		if (httpd_fs_strcmp(s->filename, "/404.html")==0)
			p_http_header = http_header_404;
		#endif			
		
		PT_WAIT_THREAD(&s->outputpt,
		   send_headers(s,
		   p_http_header));
#endif		

    ptr = strchr(s->filename, ISO_period);
    if(ptr != NULL && strncmp(ptr, http_shtml, 6) == 0) {
		
		#if WEB_SCRIPT_CODE	
		  //"/xxx/xxx.shtml"...
		  PT_INIT(&s->scriptpt);
		  PT_WAIT_THREAD(&s->outputpt, handle_script(s));
		#else
		  printf("uip disable handle_script for: %s\n", s->filename);
		#endif
    } else {
			
		//"/images/AR_S.gif"...
		printf(";_output;send file=%s (%d -> %d) len %d\r\n", s->filename, HTONS(uip_conn->lport), HTONS(uip_conn->rport), s->file.len); //[JJDbg]
		printf("\r\n");
		PT_WAIT_THREAD(&s->outputpt,
		 send_file(s));
    }
  }
  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(struct httpd_state *s))
{
	static int thread_in_len = 0;
  //PSOCK_BEGIN(&s->sin);=
	//PT_BEGIN(&((s->sin).pt))=
	{ 
	char PT_YIELD_FLAG = 1; 
	switch(((s->sin).pt).lc) 
	{ 
	case 0: //LC_RESUME(((s->sin).pt).lc)

	thread_in_len = PSOCK_DATALEN(&s->sin);
	
  //=PSOCK_READTO(&s->sin, ISO_space); //[JJ_READTO]
	s->sin.pt.lc = __LINE__; case __LINE__:
	if(psock_readto(&s->sin, ISO_space) == PT_WAITING) //[JJ_READTO]
		return PT_WAITING;
//[JJDebug]
	//.DM9051_DoRXDUMP("  [HTTP] [\"GET\" s->sin]", (uint8_t *)s->sin.bufptr, s->sin.readlen, 5, 1); 

	if(strncmp(s->inputbuf, http_get, 4) != 0) {
		PSOCK_CLOSE_EXIT(&s->sin);
	}
//[JJDebug]
	//.DM9051_DoRXDUMP("  [HTTP] [\"(s->sin)psock_readto\" s->inputbuf]", (uint8_t *)s->inputbuf, 4, 4, 1); 
	
	//=PSOCK_READTO(&s->sin, ISO_space); //[JJ_READTO]
	//=PT_WAIT_THREAD(&s->sin.pt, psock_readto(&s->sin, ISO_space))
	//=PT_WAIT_WHILE(&s->sin.pt, psock_readto(&s->sin, ISO_space) == PT_WAITING)
	//=PT_WAIT_UNTIL(&s->sin.pt, psock_readto(&s->sin, ISO_space) != PT_WAITING)
	s->sin.pt.lc = __LINE__; case __LINE__:
	if(psock_readto(&s->sin, ISO_space) == PT_WAITING) //[JJ_READTO]
		return PT_WAITING;
	
	if(s->inputbuf[0] != ISO_slash) {
		PSOCK_CLOSE_EXIT(&s->sin);
	}

	//[JJAdd-Flow]
	s->inputbuf[PSOCK_DATALEN(&s->sin)] = 0;
//[JJDebug]
	//.DM9051_DoRXDUMP("  [HTTP] [\"(s->sin)psock_readto\" s->inputbuf]", (uint8_t *)s->inputbuf, PSOCK_DATALEN(&s->sin), PSOCK_DATALEN(&s->sin), 1); 

	if (s->inputbuf[1] == ISO_space ||
			#if (HOMEPAGE_SEL_CFG == 3) //[LED TYPE0]
			((s->inputbuf[9] == 'L' && s->inputbuf[10] == 'E' && s->inputbuf[11] == 'D') &&
			(s->inputbuf[12] == '0' || s->inputbuf[12] == '1' || s->inputbuf[12] == '2')) ||
			#endif
			strncmp(s->inputbuf, "/method", 7) == 0)
	{
		//[Joseph add] TYPE0
		#if 0
		/* Control led, 0 = OFF, 1 = ON, 2 = Flash */
		//if(s->inputbuf[3] == 'L','E','D' && ((s->inputbuf[4] == '0') || 
		//	(s->inputbuf[4] == '1') || 
		//	(s->inputbuf[4] == '2')))
		if ((s->inputbuf[9] == 'L' && s->inputbuf[10] == 'E' && s->inputbuf[11] == 'D') && 
			(s->inputbuf[12] == '0' || s->inputbuf[12] == '1' || s->inputbuf[12] == '2')){
			printf("[2 lc%d]PSOCK_READTO: %c%c%c%c\r\n", s->sin.pt.lc, 
									s->inputbuf[9], s->inputbuf[10],s->inputbuf[11],s->inputbuf[12]);
			printf("[2 lc%d]PSOCK_READTO: and: %s\r\n", s->sin.pt.lc, s->inputbuf);
			Set_LED_mode(s->inputbuf[12]);
			s->inputbuf[12]= 0;
		}
		#endif
		//[uip] TYPE
		if(s->inputbuf[1] == ISO_space) {
			//strncpy(s->filename, "webMain.shtml", sizeof(s->filename));
			//strncpy(s->filename, http_index_html, sizeof(s->filename));
			//strncpy(s->filename, http_webMain_html, sizeof(s->filename));
		}
		//[JJAdd] TYPE1
		if (strncmp(s->inputbuf, "/method", 7) == 0)
			Set_LED_mode_button(s->inputbuf);
			//process_submit_button(s->inputbuf);
#if (TEST_404HTML_CFG == 1) //[test]
		strncpy(s->filename, "/404.html", sizeof(s->filename));
#else //[ORG]		
		strncpy(s->filename, (char *) HOME_HTML, sizeof(s->filename));
		//if (ledflag).. change ledpag by "update_led_fsfile()" in httpd-fs.c
#endif		
	}
	else {
		s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
		strncpy(s->filename, &s->inputbuf[0], sizeof(s->filename));
	}
	s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
	printf("[2 lc%d]PSOCK_READTO: in.data: %s (%d -> %d) len %d\r\n", s->sin.pt.lc, s->inputbuf, 
		HTONS(uip_conn->rport), HTONS(uip_conn->lport), thread_in_len); //[thread_in_len = PSOCK_DATALEN(&s->sin)]
	//.printf("[2 lc%d]PSOCK_READTO: go.fname: %s\r\n", s->sin.pt.lc, s->filename);
	//DM9051_DoRXDUMP("  [S_FILENAME] [\"STRNCPY\"]", (uint8_t *) s->filename, strlen(s->filename), strlen(s->filename), 2); //ASCII
	
  /*  httpd_log_file(uip_conn->ripaddr, s->filename);*/
  s->state = STATE_OUTPUT;
	
  while(1) {
    //PSOCK_READTO(&s->sin, ISO_nl); //[JJ_READTO]=
		s->sin.pt.lc = __LINE__; case __LINE__:
		if(psock_readto(&s->sin, ISO_nl) == PT_WAITING) //[JJ_READTO]
			return PT_WAITING;
		
    //"Referer:", 8
    if(strncmp(s->inputbuf, http_referer, 8) == 0) {
      s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0;
      /*      httpd_log(&s->inputbuf[9]);*/
			printf("[.+. lc%d]PSOCK_READTO: %s\r\n", s->sin.pt.lc, &s->inputbuf[0]);	
    }
		
		//[jjadd]
		else if (strncmp(s->inputbuf, "HTTP", 4) == 0) {
      //s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0; //[JJAdd]
			//printf("[.+. lc%d]PSOCK_READTO: %s\r\n", s->sin.pt.lc, &s->inputbuf[0]);	
		}
		else if (strncmp(s->inputbuf, "Host", 4) == 0) {
      //s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0; //[JJAdd]
			//printf("[.+. lc%d]PSOCK_READTO: %s\r\n", s->sin.pt.lc, &s->inputbuf[0]);	
		}
		else if (strncmp(s->inputbuf, "Conn", 4) == 0) {
      //s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0; //[JJAdd]
			//printf("[.+.]PSOCK_READTO: %s\r\n", &s->inputbuf[0]);	
		}
		else if (strncmp(s->inputbuf, "Cach", 4) == 0) {
      //s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0; //[JJAdd]
			//printf("[.+.]PSOCK_READTO: %s\r\n", &s->inputbuf[0]);	
		}
		else if (strncmp(s->inputbuf, "Acce", 4) == 0 ||
			strncmp(s->inputbuf, "Upgr", 4) == 0 ||
			strncmp(s->inputbuf, "User", 4) == 0)
			;
		else {
			if (PSOCK_DATALEN(&s->sin) - 2) {
				s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0; //[JJAdd]
				printf("[.+. lc%d]PSOCK_READTO: %s\r\n", s->sin.pt.lc, &s->inputbuf[0]);	
			}
		}
  }
	
 #if 0	
  ..httpd.c(431): warning:  #111-D: statement is unreachable
	printf("[e]SWITCH_CASE__LINE__+WHILE_ALWAYS_RETURN_PT_WAITING_SO_NEVER_FINISH\r\n"); //statement is unreachable
 #endif	
  //PSOCK_END(&s->sin);
	//=PT_END(&s->sin.pt)
	//=
	//LC_END((s->sin.pt).lc); 
	//PT_YIELD_FLAG = 0;
  //PT_INIT(s->sin.pt); 
	//return PT_ENDED; }
	//=
	} //LC_END((s->sin.pt).lc); 
	printf("[e lc%d]PSOCK_END\r\n", s->sin.pt.lc);	//[JJAdd] statement is unreachable
	PT_YIELD_FLAG = 0; 
  (s->sin.pt).lc = 0; //LC_INIT((s->sin.pt).lc); 
	return PT_ENDED; 
	}
}
/*---------------------------------------------------------------------------*/
static void
handle_connection(struct httpd_state *s)
{
  handle_input(s);
  if(s->state == STATE_OUTPUT) {
    handle_output(s);
  }
}
/*---------------------------------------------------------------------------*/
void
httpd_appcall(void)
{
  struct httpd_state *s = (struct httpd_state *)&(uip_conn->appstate);

  if(uip_closed() || uip_aborted() || uip_timedout()) {
  } else if(uip_connected()) {
    PSOCK_INIT(&s->sin, s->inputbuf, sizeof(s->inputbuf) - 1);
    PSOCK_INIT(&s->sout, s->inputbuf, sizeof(s->inputbuf) - 1);
    PT_INIT(&s->outputpt);
    s->state = STATE_WAITING;
    /*    timer_set(&s->timer, CLOCK_SECOND * 100);*/
    s->timer = 0;
    handle_connection(s);
  } else if(s != NULL) {
    if(uip_poll()) {
      ++s->timer;
      if(s->timer >= 20) {
				uip_abort();
      }
    } else {
      s->timer = 0;
    }
    handle_connection(s);
  } else {
    uip_abort();
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Initialize the web server
 *
 *             This function initializes the web server and should be
 *             called at system boot-up.
 */
void
httpd_init(void)
{
  uip_listen(HTONS(80));
	uip_listen(HTONS(5100));
	printf("HonePage: %s\n", HOME_HTML);
}
/*---------------------------------------------------------------------------*/
/** @} */
