#ifndef DM9051_MH2030A_BOARD_H
#define DM9051_MH2030A_BOARD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void mh2030a_uip_board_init(uint32_t baudrate);
void mh2030a_uip_clock_init(void);
void mh2030a_uip_tick_isr(void);

typedef int (*mh2030a_board_putchar_fn)(int ch);
typedef int (*mh2030a_board_ready_fn)(void);

/* 供上層(main)在網路就緒後把 console 輸出改接到其他 sink(如 udp_printf)；
 * ready_fn 每次 fputc() 都會查詢，一旦回傳 0 就自動退回 UART，board 層本身
 * 不認識也不依賴 sink 背後的協定棧實作。 */
void mh2030a_board_set_console_sink(mh2030a_board_putchar_fn putchar_fn,
                                     mh2030a_board_ready_fn ready_fn);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_MH2030A_BOARD_H */
