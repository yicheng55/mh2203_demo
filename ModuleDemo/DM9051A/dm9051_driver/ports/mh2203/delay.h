#ifndef DM9051_MH2203_DELAY_H
#define DM9051_MH2203_DELAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Delay_Ms(uint16_t ms);
void Delay_Us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_MH2203_DELAY_H */
