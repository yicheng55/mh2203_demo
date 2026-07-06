#include "at_port.h"

/* UART wrappers for AT_Command / etherbridge */
void uart_send_byte(char c)
{
    atp_uart_putc(c);
}

void uart_send_string(const char *s)
{
    while (*s)
        atp_uart_putc(*s++);
}
