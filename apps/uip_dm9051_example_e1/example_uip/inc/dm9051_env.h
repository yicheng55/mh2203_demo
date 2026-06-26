/**
  **************************************************************************
  * @file     dm9051_env.h
  * @version  v1.0
  * @date     2023-04-28
  * @brief    header file of dm9051 environment config program.
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#ifndef __DM9051_ENV_H
#define __DM9051_ENV_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "at32f403a_407.h"
//#include "netif.h"
//#include "dm9051_lw.h"
#include "core/dm9051.h"

/** @addtogroup 415_dm9051_env
  * @{
  */

	 
void emac_tmr_init(void);
	 
///*
// * A function env_dm9051f_system_init(void) / "dm9051_env.c",
// *
// * A function env_ethernetif_update_config_cb(struct netif *netif) / "dm9051f.c",
// *
// *   Called by main() / "main.c"
// *   Called by tcpip_stack_init() / "netconf.c" (Called by main()'s init-part / "main.c")
// *   Called by _lwip_periodic_handle() / "netconf.c" (Called by main()'s for-loop / "main.c")
// *
// * More:
// *   A function lwip_rx_loop_handler(void) / "netconf.c" (Called by main()'s for-loop / "main.c")
// *
// */
//error_status env_dm9051f_system_init(void);
////void env_ethernetif_update_config_cb(struct netif *netif);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif //__DM9051_ENV_H
