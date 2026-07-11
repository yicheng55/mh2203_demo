#ifndef ATCMD_WEB_MH2203_COMPAT_H
#define ATCMD_WEB_MH2203_COMPAT_H

#include "mh2203_platform.h"
#include "at_port.h"

#ifdef __cplusplus
extern "C" {
#endif

FLASH_Status atcmd_mh2203_flash_erase_page(uint32_t address);
FLASH_Status atcmd_mh2203_flash_program_word(uint32_t address, uint32_t data);
uint32_t atcmd_mh2203_flash_read_word(uint32_t address);
void atcmd_mh2203_flash_unlock(void);
void atcmd_mh2203_flash_lock(void);
void atcmd_mh2203_flash_clear_flag(uint32_t flags);

#ifdef __cplusplus
}
#endif

#endif /* ATCMD_WEB_MH2203_COMPAT_H */
