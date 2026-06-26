#ifndef __UDPDEMO_H__
#define __UDPDEMO_H__

#include "stdint.h"


struct udp_appstate
{
    uint8_t state;
    uint8_t *textptr;
    int textlen;
};

void udp_recv_appcall(void);
void udp_recv_connect(void);
void udp_send_appcall(void);
void udp_send_connect(void);

void myudp_appcall(void);

void myudp_init(void);
#endif




