#ifndef DM9051_MH2203_BOARD_H
#define DM9051_MH2203_BOARD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void mh2203_uip_board_init(uint32_t baudrate);
void mh2203_uip_clock_init(void);
void mh2203_uip_tick_isr(void);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_MH2203_BOARD_H */
