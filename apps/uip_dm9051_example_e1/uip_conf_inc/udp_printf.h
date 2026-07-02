#ifndef UDP_PRINTF_H
#define UDP_PRINTF_H

#include <stdint.h>

struct uip_udp_conn;

#define UDP_PRINTF_PORT 1600

void udp_printf_init(void);
void udp_printf_appcall(void);
int udp_printf_putchar(int ch);
void udp_printf_set_link(int link_up);
int udp_printf_is_link_up(void);
int udp_printf_is_ready(void);
void udp_printf_status_uart(const char *msg);
int udp_printf_has_pending(void);
struct uip_udp_conn *udp_printf_get_conn(void);
int udp_printf_is_reserved_port(uint16_t port);
const char *udp_printf_reserved_port_name(uint16_t port);

#endif /* UDP_PRINTF_H */
