#ifndef __DEVELOP_CONF_H__
#define __DEVELOP_CONF_H__

#include <stdint.h>

//#define	MQTT_CLIENT_SUPPORT	0

//#if MQTT_CLIENT_SUPPORT
//#include "mqtt_client.h"
//#endif

typedef int8_t    s8_t;
#ifndef UIP_U16_T_DEFINED
typedef uint16_t  u16_t;
#define UIP_U16_T_DEFINED
#endif
typedef int16_t   s16_t;
typedef uint32_t  u32_t;
typedef int32_t   s32_t;

#define SYSTEMTICK_PERIOD_MS10             10

/** Definitions for error constants. */
typedef enum {
/** No error, everything OK. */
  ERR_OK         = 0,
/** Out of memory error.     */
  ERR_MEM        = -1,
/** Buffer error.            */
  ERR_BUF        = -2,
/** Timeout.                 */
  ERR_TIMEOUT    = -3,
/** Routing problem.         */
  ERR_RTE        = -4,
/** Operation in progress    */
  ERR_INPROGRESS = -5,
/** Illegal value.           */
  ERR_VAL        = -6,
/** Operation would block.   */
  ERR_WOULDBLOCK = -7,
/** Address in use.          */
  ERR_USE        = -8,
/** Already connecting.      */
  ERR_ALREADY    = -9,
/** Conn already established.*/
  ERR_ISCONN     = -10,
/** Not connected.           */
  ERR_CONN       = -11,
/** Low-level netif error    */
  ERR_IF         = -12,

/** Connection aborted.      */
  ERR_ABRT       = -13,
/** Connection reset.        */
  ERR_RST        = -14,
/** Connection closed.       */
  ERR_CLSD       = -15,
/** Illegal argument.        */
  ERR_ARG        = -16
} err_enum_t;

typedef s8_t err_t;

#define PBUF_POOL_BUFSIZE                (1514+4) //.2000	//.2000(tested) //dm,jj=1514

#endif //__DEVELOP_CONF_H__
