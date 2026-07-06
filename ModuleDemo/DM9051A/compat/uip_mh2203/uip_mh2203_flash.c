#include "at_port.h"

/* Flash read/write wrappers for dataflash.c */
void nv_erase(void)
{
    atp_nv_erase();
}

void nv_write_word(uint32_t idx, uint32_t word)
{
    atp_nv_write_word(idx, word);
}

uint32_t nv_read_word(uint32_t idx)
{
    return atp_nv_read_word(idx);
}

void nv_commit(void)
{
    atp_nv_commit();
}
