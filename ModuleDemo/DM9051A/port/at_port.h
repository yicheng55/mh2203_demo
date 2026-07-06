#ifndef __AT_PORT_H__
#define __AT_PORT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UART */
void         atp_uart_init(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop);
void         atp_uart_putc(char c);
char        *atp_uart_rx_buf(void);
unsigned int atp_uart_rx_len(void);
void         atp_uart_rx_clear(void);
int          atp_uart_rx_ready(void);

/* Non-volatile storage (word-based) */
void         atp_nv_erase(void);
void         atp_nv_write_word(uint32_t idx, uint32_t word);
uint32_t     atp_nv_read_word(uint32_t idx);
void         atp_nv_commit(void);

/* System control */
void         atp_system_reset(void);

/* Timer */
uint32_t     atp_mstick(void);

#ifdef __cplusplus
}
#endif

#endif /* __AT_PORT_H__ */
