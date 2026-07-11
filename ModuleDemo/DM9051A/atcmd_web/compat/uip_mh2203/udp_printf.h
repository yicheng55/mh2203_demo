#ifndef UDP_PRINTF_H
#define UDP_PRINTF_H

#include <stdint.h>

struct uip_udp_conn;

#define PRINTF_DEBUG_OUTPUT_UART 0
#define PRINTF_DEBUG_OUTPUT_UDP  1
#define UDP_PRINTF_PORT          1600

#ifndef PRINTF_DEBUG_OUTPUT
#define PRINTF_DEBUG_OUTPUT PRINTF_DEBUG_OUTPUT_UDP
#endif

void udp_printf_init(void);
void udp_printf_appcall(void);
int udp_printf_putchar(int ch);
void udp_printf_set_link(int link_up);
int udp_printf_is_link_up(void);
int udp_printf_is_ready(void);
void udp_printf_status_uart(const char *msg);
int udp_printf_has_pending(void);
struct uip_udp_conn *udp_printf_get_conn(void);
void udp_printf_output_done(void);
void udp_printf_set_output_mode(int enable);
int udp_printf_is_output_mode_enabled(void);
int udp_printf_is_reserved_port(uint16_t port);
const char *udp_printf_reserved_port_name(uint16_t port);

#endif /* UDP_PRINTF_H */
