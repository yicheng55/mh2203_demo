#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include "httpd.h"

typedef struct httpd_state uip_tcp_appstate_t;

#ifndef UIP_APPCALL
#define UIP_APPCALL     httpd_appcall
#endif

#endif
