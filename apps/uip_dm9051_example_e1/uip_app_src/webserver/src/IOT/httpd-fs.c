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
#include <stdio.h>
#include "uip.h" //added for "app_call.h" compiled!(httpd.c did include uip.h, firstly.)
#include "httpd.h"
#include "httpd-fs.h"
#include "httpd-fsdata.h"
#include "httpd-debug.h"

#ifndef NULL
#define NULL 0
#endif /* NULL */

#include "httpd-fsdata.c"

#if HTTPD_FS_STATISTICS
static u16_t count[HTTPD_FS_NUMFILES];
#endif /* HTTPD_FS_STATISTICS */

void httpd_report_root_tree(void)
{
	fs_ROOT_HTML = HTTPD_FS_ROOT; //'static' issue!
}

/*-----------------------------------------------------------------------------------*/
static u8_t
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
/*-----------------------------------------------------------------------------------*/

/* if "expression" is true, then execute "handler" expression */
#define DBG_FS_OPEN(expression, fname, handler) do { if (!httpd_fs_strcmp((expression), fname)) { \
  handler;}} while(0)

static void result_true_print(char *name, int len)
{
	printf("result true, %s len= %d\r\n", name, len);
}

int
httpd_fs_open(const char *name, struct httpd_fs_file *file)
{
#if HTTPD_FS_STATISTICS
  u16_t i = 0;
#endif /* HTTPD_FS_STATISTICS */
  struct httpd_fsdata_file_noconst *f;
int opennum = 0;
  for(f = (struct httpd_fsdata_file_noconst *)HTTPD_FS_ROOT;
      f != NULL;
      f = (struct httpd_fsdata_file_noconst *)f->next) {

 //.printf("%02d. compare %s, openfile %s\r\n", ++opennum, name, f->name);
    if(httpd_fs_strcmp(name, f->name) == 0) {
      file->data = f->data;
      file->len = f->len;
#if HTTPD_FS_STATISTICS
      ++count[i];
#endif /* HTTPD_FS_STATISTICS */
	
	  DBG_FS_OPEN("/style2.css", f->name, result_true_print(f->name, file->len)); 
	  //DBG_FS_OPEN(f->name, f->name, result_true_print(f->name, file->len)); 
	  //print(f->name, file->len);
      return 1;
    }
#if HTTPD_FS_STATISTICS
    ++i;
#endif /* HTTPD_FS_STATISTICS */

  }
 //.printf("result false, file not found!\r\n");
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
