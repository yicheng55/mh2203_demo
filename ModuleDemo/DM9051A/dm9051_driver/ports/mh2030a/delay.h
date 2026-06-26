#ifndef DM9051_MH2030A_DELAY_H
#define DM9051_MH2030A_DELAY_H

#include <stdint.h>
#include "../../core/inc/dm9051_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void Delay_Ms(uint16_t ms);
void Delay_Us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_MH2030A_DELAY_H */
