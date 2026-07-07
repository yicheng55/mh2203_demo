#ifndef ATCMD_WEB_MH2203_AT32_BOARD_UART_COMPAT_H
#define ATCMD_WEB_MH2203_AT32_BOARD_UART_COMPAT_H

#include "at32f4xx.h"
#include "at_port.h"

#define AT32_CMD_USART USART2

void UART_CMD_Init_Update(uint32_t baud, uint8_t wordlen, uint8_t parity, uint8_t stop);
void AT_USART_DMA_RxToggle(uint16_t length);
void DMA_Init_AT(void);

#endif /* ATCMD_WEB_MH2203_AT32_BOARD_UART_COMPAT_H */
