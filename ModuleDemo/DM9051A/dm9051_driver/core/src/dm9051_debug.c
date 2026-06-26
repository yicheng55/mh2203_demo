/*
 * Future DM9051 diagnostics and optional debug helpers.
 *
 * Current diagnostic/helper sources are still in:
 *   drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051_beta.c
 *
 * Keep this file out of existing Keil builds until the staged core is ready.
 */

#include "dm9051_core.h"

/* -------------------------------------------------------------------------
 * Staging rules
 * -------------------------------------------------------------------------
 *
 * - Keep hot-path RX/TX behavior in dm9051_core.c.
 * - Put optional register dumps, pointer diagnostics, and log helpers here.
 * - Do not include uIP, lwIP, or MH2030A platform headers here.
 */

/* -------------------------------------------------------------------------
 * Register snapshots
 * ---------------------------------------------------------------------- */

/* Future owner for:
 *   dm9051_status
 *   dm9051_read_regs_info
 */

/* -------------------------------------------------------------------------
 * RX diagnostics
 * ---------------------------------------------------------------------- */

/* Future owner for:
 *   dm9051_read_rx_pointers
 *   env_evaluate_rxb
 *   env_evaluate_rxb_zero
 *   dm9051_show_rxbstatistic
 *   ret_fire_time
 */

/* -------------------------------------------------------------------------
 * Timing / error diagnostics
 * ---------------------------------------------------------------------- */

/* Future owner for:
 *   davicom_haltickcount, if retained as diagnostic only
 *   env_err_rsthdlr3
 */
