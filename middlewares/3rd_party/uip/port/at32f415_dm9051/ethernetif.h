#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

//#include "lwip/err.h"
//#include "lwip/netif.h"

//err_t _ethernetif_input(struct netif *netif);
//err_t _ethernetif_init(struct netif *netif);
u16_t ethernetif_input(void);
err_t ethernetif_output(void);
err_t ethernetif_init(uint8_t* macadd);

#endif
