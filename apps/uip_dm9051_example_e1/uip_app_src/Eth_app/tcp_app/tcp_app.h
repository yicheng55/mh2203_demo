#ifndef __TCP_SERVICE_H_
#define __TCP_SERVICE_H_

//.#include "includes.h"

/* Set TCP Client Connect Remote IP and Port */
#define TCP_Remote_IP0 		192 //192
#define TCP_Remote_IP1 		168 //168
#define TCP_Remote_IP2 		6   //1
#define TCP_Remote_IP3 		10  //19
#define TCP_Remote_PORT 	5002

/* Set TCP Server Listen Port */
#define TCP_LISTEN_PORT   5001

struct tcp_state
{
    uint8_t state;
    uint8_t *textptr;
    int textlen;
};

//typedef struct tcp_state uip_tcp_appstate_t;

/* tcp server function */
void tcp_server_init(void);

/* tcp client function */
void tcp_client_init(void);

/* TCP Client/Server appcall */
void tcp_client_server_appcall(void);
#endif //__TCP_SERVICE_H_
