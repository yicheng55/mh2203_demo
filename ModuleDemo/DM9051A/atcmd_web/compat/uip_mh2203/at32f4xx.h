#ifndef ATCMD_WEB_MH2203_AT32F4XX_COMPAT_H
#define ATCMD_WEB_MH2203_AT32F4XX_COMPAT_H

#include "mh2203_platform.h"
#include "atcmd_mh2203_compat.h"

#ifndef _printf
#define _printf(...) ((void)0)
#endif

#ifndef __IO
#define __IO volatile
#endif

#ifndef FLASH_PRC_DONE
#define FLASH_PRC_DONE FLASH_COMPLETE
#endif

#ifndef FLASH_FLAG_PRGMFLR
#define FLASH_FLAG_PRGMFLR FLASH_FLAG_PGERR
#endif

#ifndef FLASH_FLAG_PRCDN
#define FLASH_FLAG_PRCDN FLASH_FLAG_EOP
#endif

#ifndef USART_FLAG_TRAC
#define USART_FLAG_TRAC USART_FLAG_TC
#endif

#ifndef USART_INT_IDLEF
#define USART_INT_IDLEF USART_IT_IDLE
#endif
#ifndef ATCMD_WEB_MH2203_FLASH_COMPAT_IMPL
#define FLASH_Unlock() atcmd_mh2203_flash_unlock()
#define FLASH_Lock() atcmd_mh2203_flash_lock()
#define FLASH_ClearFlag(flags) atcmd_mh2203_flash_clear_flag((flags))
#define FLASH_ErasePage(address) atcmd_mh2203_flash_erase_page((address))
#define FLASH_ProgramWord(address, data) atcmd_mh2203_flash_program_word((address), (data))
#endif
#endif /* ATCMD_WEB_MH2203_AT32F4XX_COMPAT_H */
