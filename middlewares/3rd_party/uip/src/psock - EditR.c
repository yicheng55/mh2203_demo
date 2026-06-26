/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * This file is part of the uIP TCP/IP stack
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: psock.c,v 1.2 2006/06/12 08:00:30 adam Exp $
 */

#include <stdio.h>
#include <string.h>

#include "uipopt.h"
#include "psock.h"
#include "uip.h"

#define STATE_NONE 0
#define STATE_ACKED 1
#define STATE_READ 2
#define STATE_BLOCKED_NEWDATA 3
#define STATE_BLOCKED_CLOSE 4
#define STATE_BLOCKED_SEND 5
#define STATE_DATA_SENT 6

/*
 * Return value of the buffering functions that indicates that a
 * buffer was not filled by incoming data.
 *
 */
#define BUF_NOT_FULL 0
#define BUF_NOT_FOUND 0

/*
 * Return value of the buffering functions that indicates that a
 * buffer was completely filled by incoming data.
 *
 */
#define BUF_FULL 1

/*
 * Return value of the buffering functions that indicates that an
 * end-marker byte was found.
 *
 */
#define BUF_FOUND 2

/*---------------------------------------------------------------------------*/
