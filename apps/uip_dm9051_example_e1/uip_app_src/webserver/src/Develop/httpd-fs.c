/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
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
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: httpd-fs.c,v 1.1 2006/06/07 09:13:08 adam Exp $
 */

#include  <stdio.h>
#include  <string.h>
#include "uip.h"
//#include "httpd.h", insteaded by uip.h
#include "httpd-fs.h"
#include "httpd-fsdata.h"

#ifndef NULL
#define NULL 0
#endif /* NULL */

#if (TEST_404HTML_CFG == 1) //[Test]
 
/* "/404.html" */
/* "/404.html" must exist in 'HTTPD_FS_ROOT' fs-list */
#elif (HOMEPAGE_SEL_CFG == 0) // "/AT32F407LED.html" 

   //...DSFSDFS...
//#include "httpd-fsdata.c"
const char http_at32led_html[18] = {0x2f, 'A', 'T', '3', '2', 'F', '4', '0', '7', 'L', 'E', 'D', '.', 'h', 't', 'm', 'l', };
#include "httpd-fsdata_GIF_AT.c"
#include "httpd-fsdata_GIF_Logo.c"
#include "httpd-fsdata_GIF_newline.c"
#include "httpd-fsdata_AT32.c"
#include "httpd-fsdata_Tab_Pages.c" //[Tab_data]
	
#elif (HOMEPAGE_SEL_CFG == 1) // "/index.html" [as uip-0-9]

 
#include "fsdata.c"
#elif (HOMEPAGE_SEL_CFG == 2) // "Dmeo2"

 
#include "httpd-fsdata2.c"
#elif (HOMEPAGE_SEL_CFG == 3) // "Dmeo3"

 
#include "httpd-fsdata3.c"
#endif

//[JJAdd] FOR ADC page ---
//#include "at32f4xx_adc.h"
//[JJAdd]
//#include "main1.h"
#include "mhscpu.h"

#if HTTPD_FS_STATISTICS
static u16_t count[HTTPD_FS_NUMFILES];
#endif /* HTTPD_FS_STATISTICS */

/*-----------------------------------------------------------------------------------*/
/*static*/ u8_t
httpd_fs_strcmp(const char *str1, const char *str2)
{
  u8_t i;
  i = 0;
 loop:

  if(str2[i] == 0 ||
     str1[i] == '\r' ||
     str1[i] == '\n') {
    return 0;
  }

  if(str1[i] != str2[i]) {
    return 1;
  }


  ++i;
  goto loop;
}

#if WEB_FS_TABPAGES
static const char *file_LED_html_tab[] =
{
	NULL,
	data_AT32F407LED_html_ChkLED2, //file_AT32F407LED_html_ChkLED2,
	data_AT32F407LED_html_ChkLED3, //file_AT32F407LED_html_ChkLED3,
	data_AT32F407LED_html_ChkLED2_ChkLED3, //file_AT32F407LED_html_ChkLED2_ChkLED3,
	data_AT32F407LED_html_ChkLED4, //file_AT32F407LED_html_ChkLED4,
	data_AT32F407LED_html_ChkLED2_ChkLED4, //file_AT32F407LED_html_ChkLED2_ChkLED4,
	data_AT32F407LED_html_ChkLED3_ChkLED4, //file_AT32F407LED_html_ChkLED3_ChkLED4,
	data_AT32F407LED_html_ChkLED2_ChkLED3_ChkLED4, //file_AT32F407LED_html_ChkLED2_ChkLED3_ChkLED4,
};

static int bADC_Get = 0;
//static int SimuDiff = 0; //simulation!
static int ibADCVa;
static int ibCN;
#define NUM_FULL_BLANK	127 //128 //126 //120 //116
char html_tmp[4726] = {0}; //[JJAdd] FOR ADC page ---

void update_ajax_fsfile(struct httpd_fs_file *file)
{
	int ADCVal;
	float fADCVal;
#if 1
	//ADCVal = ADC_GetConversionValue(ADC1); //(ADC0); //[JJComment][at32f4xx_adc.c]
	ADCVal = 0; //4096;
	ADCVal = bADC_Get;
	//simu.ADC_GetConValue.
	bADC_Get += 256;
	if (bADC_Get > 4096) bADC_Get = 0;
#endif
	fADCVal = (float)((float)ADCVal / (float)4096.00) * (float)3.3;
	
	ibADCVa = ((ADCVal * 100 ) / 4096 ) * 512 / 100;
	ibCN = (ibADCVa*NUM_FULL_BLANK)/512; // We measurement as 'NUM_FULL_BLANK'
	//printf("-[ADCVal (%d) = %.2f]- ibCN %d\r\n", ADCVal, fADCVal, ibCN);
	//.printf("-[ibADCVa (%d)] ----------> [ibCN %d]-\r\n", ibADCVa, ibCN);
	
	memset(html_tmp, 0, sizeof(html_tmp));
	sprintf(html_tmp, "%.2fV", fADCVal);
	file->data = html_tmp;
	file->len = strlen(html_tmp);
}
void update_barinfo_fsfile(struct httpd_fs_file *file)
{
	int i;
	//static int CN = 0;
	//CN+=2;
	//if (CN > 116) CN = 116;
	//.printf("[pbar acc-add CN %d ]  ==========> [pbar ibCN %d]\r\n", CN, ibCN);
	memset(html_tmp, 0, sizeof(html_tmp));
	strcat(html_tmp, "&nbsp;");
	for (i=1;i<ibCN; i++)
		strcat(html_tmp, "&nbsp;");
	
	file->data = html_tmp;
	file->len = strlen(html_tmp);
}
void update_adc_fsfile(struct httpd_fs_file *file)
{
	int ADCVal;
	float fADCVal;
	int iADCVa;
	#if 1 
	//ADCVal = ADC_GetConversionValue(ADC1); //(ADC0); //[JJComment][at32f4xx_adc.c]
	ADCVal = 0; //4096;
	#endif
	fADCVal = (float)((float)ADCVal / (float)4096.00) * (float)3.3;

	iADCVa = ((ADCVal * 100 ) / 4096 ) * 512 / 100;
	
	printf("-[ADCVal (%d) = %.2f (width = %d)]-\r\n", ADCVal, fADCVal, iADCVa);
	// --- char html_tmp[4096] = {0}; ---
	// to
	// --- char html_tmp[4726] = {0}; ---
	sprintf(html_tmp, file->data, "100%");
	//sprintf(html_tmp, file->data, "100%", fADCVal, iADCVa); // We have "%s" in the 'file->data' to be "100%"
	file->data = html_tmp;
	file->len = strlen(html_tmp);
}
void update_led_fsfile(struct httpd_fs_file *file)
{
	char ledflag = ledflag_of_input();
	if (ledflag) {
		file->data = (char *) file_LED_html_tab[ledflag];
		file->len = (int) strlen(file_LED_html_tab[ledflag]);
	}
}
#endif

/*-----------------------------------------------------------------------------------*/
int
httpd_fs_open(const char *name, struct httpd_fs_file *file)
{
#if HTTPD_FS_STATISTICS
  u16_t i = 0;
#endif /* HTTPD_FS_STATISTICS */
  struct httpd_fsdata_file_noconst *f;
	
	//---[Temp here!] You can say, This is not a file system file---
	//[JJAdd] FOR "ajax_info.txt"
	#if WEB_FS_TABPAGES
	if (httpd_fs_strcmp(name, "/ajax_info.txt") == 0) {				
		update_ajax_fsfile(file);
		return 1;
	}
	if (httpd_fs_strcmp(name, "/bar_info.txt") == 0) {
		update_barinfo_fsfile(file);
		return 1;
	}
	#endif

  for(f = (struct httpd_fsdata_file_noconst *)HTTPD_FS_ROOT;
      f != NULL;
      f = (struct httpd_fsdata_file_noconst *)f->next) {

    if(httpd_fs_strcmp(name, f->name) == 0) {
      file->data = f->data;
      file->len = f->len;
			
			// --------------------------------------------------------------------------
			#if WEB_FS_TABPAGES
			//[JJAdd] FOR ADC page --- (name="s->filename" vs f->name)
			if (httpd_fs_strcmp(name, "/AT32F407ADC.html") == 0) {
				update_adc_fsfile(file);
			}
			if (httpd_fs_strcmp(name, "/AT32F407LED.html") == 0) {
				update_led_fsfile(file);
			}
			#endif
			
			//if (httpd_fs_strcmp(name, "/AT32F407ADC.html") == 0)
			// printf("file->len = %d is= new file_data's file_len <--------------------------\r\n", file->len);
			//else
			// printf("file->len = %d is= new file_data's file_len <--------------------------\r\n", file->len);
			// ------------------------------------------------------------------------------------------------
			//[JJAdd]
#if HTTPD_FS_STATISTICS
      ++count[i]; //........fgerg........
#endif /* HTTPD_FS_STATISTICS */
      return 1;
    }
#if HTTPD_FS_STATISTICS
    ++i;
#endif /* HTTPD_FS_STATISTICS */

  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
void
httpd_fs_init(void)
{
#if HTTPD_FS_STATISTICS
  u16_t i;
  for(i = 0; i < HTTPD_FS_NUMFILES; i++) {
    count[i] = 0;
  }
#endif /* HTTPD_FS_STATISTICS */
}
/*-----------------------------------------------------------------------------------*/
#if HTTPD_FS_STATISTICS
u16_t httpd_fs_count
(char *name)
{
  struct httpd_fsdata_file_noconst *f;
  u16_t i;

  i = 0;
  for(f = (struct httpd_fsdata_file_noconst *)HTTPD_FS_ROOT;
      f != NULL;
      f = (struct httpd_fsdata_file_noconst *)f->next) {

    if(httpd_fs_strcmp(name, f->name) == 0) {
      return count[i];
    }
    ++i;
  }
  return 0;
}
#endif /* HTTPD_FS_STATISTICS */
/*-----------------------------------------------------------------------------------*/
