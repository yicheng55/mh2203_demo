#ifndef __UIP_CONF_H__
#define __UIP_CONF_H__

#include <inttypes.h>

typedef uint8_t u8_t;

#ifndef UIP_U16_T_DEFINED
typedef uint16_t u16_t;
#define UIP_U16_T_DEFINED
#endif

typedef unsigned short uip_stats_t;

#define UIP_CONF_MAX_CONNECTIONS 10
#define UIP_CONF_MAX_LISTENPORTS 40
#define UIP_CONF_BUFFER_SIZE     1200
#define UIP_CONF_BYTE_ORDER      LITTLE_ENDIAN
#define UIP_CONF_LOGGING         1
#define UIP_CONF_UDP             1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_STATISTICS      1

#include "app_call.h"

#endif
