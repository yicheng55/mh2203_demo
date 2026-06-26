#ifndef DM9051_HAL_MH2030A_INT_H
#define DM9051_HAL_MH2030A_INT_H

#include "dm9051_hal_mh2030a_spi1.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Compile this file with DM9051_MH2030A_ENABLE_IRQ=1. */
void dm9051_mh2030a_irq_attach_device(dm9051_device_t *dev);
void dm9051_mh2030a_irq_detach_device(void);
uint32_t dm9051_mh2030a_irq_line(void);
uint32_t dm9051_mh2030a_irq_count(void);
void dm9051_mh2030a_irq_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* DM9051_HAL_MH2030A_INT_H */
