#include "includes.h"
#include "stdio.h"
#include "atcommand.h"
#include "dataflash.h"
#include "at_port.h"

extern struct at_funcation at_show;
extern struct eeprom_funcation eeprom_show;

void Copy_AT_Type_to_Show(void)
{
		at_show = at_type;
		eeprom_show = eeprom_type;
}

#define AT_FLASH_BASE   (0x0801F800)

static uint32_t s_nv_cache[64];
static uint8_t  s_nv_dirty = 0;

static uint32_t read_nv_word(uint32_t idx)
{
    return atp_nv_read_word(idx);
}

static void write_nv_word(uint32_t idx, uint32_t data)
{
    s_nv_cache[idx] = data;
    s_nv_dirty = 1;
}

static void flush_nv(void)
{
    uint32_t i;
    if (!s_nv_dirty)
        return;
    atp_nv_erase();
    for (i = 0; i < 64; i++)
        atp_nv_write_word(i, s_nv_cache[i]);
    atp_nv_commit();
    s_nv_dirty = 0;
}

void Write_AT_Show_DataFlash(void)
{
    uint32_t u32Data, u32Addr;
    uint8_t i;

    /* Cache current flash contents first */
    for (i = 0; i < 64; i++)
        s_nv_cache[i] = atp_nv_read_word(i);

    u32Addr = 0;

    s_nv_cache[u32Addr++] = TEST_PATTERN;

    if (at_show.dhcpc_mode)
        u32Data = at_show.role | 0x8;
    else
        u32Data = at_show.role & 0x7;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (at_show.hostip[1] << 16) + at_show.hostip[0];
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (at_show.hostmask[1] << 16) + at_show.hostmask[0];
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (at_show.hostgw[1] << 16) + at_show.hostgw[0];
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = at_show.t_lport;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (at_show.tcp_raddr[1] << 16) + at_show.tcp_raddr[0];
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = at_show.tcp_rport;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = at_show.u_lport;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (at_show.udp_raddr[1] << 16) + at_show.udp_raddr[0];
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = at_show.udp_rport;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = at_show.dns_mode;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (at_show.dns_saddr[1] << 16) + (at_show.dns_saddr[0]);
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = at_show.dns_srvport;
    s_nv_cache[u32Addr++] = u32Data;

    for (i = 0; i < 32; i += 4) {
        u32Data = (at_show.dns_srvname[i + 3] << 24) + (at_show.dns_srvname[i + 2] << 16) +
                  (at_show.dns_srvname[i + 1] << 8) + (at_show.dns_srvname[i]);
        s_nv_cache[u32Addr++] = u32Data;
    }
    for (i = 0; i < 32; i += 4) {
        u32Data = (at_show.dns_srvpath[i + 3] << 24) + (at_show.dns_srvpath[i + 2] << 16) +
                  (at_show.dns_srvpath[i + 1] << 8) + (at_show.dns_srvpath[i]);
        s_nv_cache[u32Addr++] = u32Data;
    }

    u32Data = at_show.dns_srvport;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (at_show.keepalive_send_count << 24) + (at_show.keepalive_send_period << 16)
              + (at_show.keepalive_no_data_period << 8) + (at_show.keepalive_mode);
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = at_show.baudrate;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (at_show.stop << 16) + (at_show.parity << 8) + (at_show.wordlen);
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (at_show.rst_count << 16) + (at_show.trans_len);
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (eeprom_show.macaddr[1] << 24) + (eeprom_show.macaddr[0] << 16);
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = (eeprom_show.macaddr[5] << 24) + (eeprom_show.macaddr[4] << 16) +
              (eeprom_show.macaddr[3] << 8) + (eeprom_show.macaddr[2]);
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = eeprom_show.autoload;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = eeprom_show.pid;
    u32Data = (u32Data << 16) + eeprom_show.vid;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = eeprom_show.wakeupcontrol2;
    u32Data = (u32Data << 16) + eeprom_show.pincontrol;
    s_nv_cache[u32Addr++] = u32Data;

    u32Data = eeprom_show.control3;
    s_nv_cache[u32Addr++] = u32Data;

    s_nv_dirty = 1;
    flush_nv();
}

void Write_AT_DataFlash(void)
{
    Copy_AT_Type_to_Show();
    Write_AT_Show_DataFlash();
}

int AT_Pass_Custom_Mode(void)
{
    if (at_type.rst_count < 200) {
        Write_AT_DataFlash();
    }
    if (at_type.rst_count >= 200) {
        printf("Erase FullChip\n");
        return 0;
    }
    return 1;
}

uint8_t Read_AT_Show_DataFlash(uint8_t inc)
{
    uint8_t i;
    uint8_t mflag = 0;
    uint32_t u32Addr;
    uint32_t u32Data;

    (void)inc;
    u32Addr = 0;

    u32Data = atp_nv_read_word(u32Addr++);
    if (u32Data != TEST_PATTERN) {
        printf("DataEmp\r\n");
        mflag = 2;
        return mflag;
    }

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.role = (u32Data & 0x7);
    if (u32Data & 0x8)
        at_show.dhcpc_mode = 1;
    else
        at_show.dhcpc_mode = 0;

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.hostip[0] = (uint16_t)(u32Data);
    at_show.hostip[1] = (uint16_t)(u32Data >> 16);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.hostmask[0] = (uint16_t)(u32Data);
    at_show.hostmask[1] = (uint16_t)(u32Data >> 16);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.hostgw[0] = (uint16_t)(u32Data);
    at_show.hostgw[1] = (uint16_t)(u32Data >> 16);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.t_lport = (uint32_t)(u32Data);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.tcp_raddr[0] = (uint16_t)(u32Data);
    at_show.tcp_raddr[1] = (uint16_t)(u32Data >> 16);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.tcp_rport = (uint32_t)u32Data;

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.u_lport = (uint32_t)(u32Data);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.udp_raddr[0] = (uint16_t)(u32Data);
    at_show.udp_raddr[1] = (uint16_t)(u32Data >> 16);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.udp_rport = (uint32_t)(u32Data);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.dns_mode = (uint8_t)(u32Data);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.dns_saddr[0] = (uint16_t)(u32Data);
    at_show.dns_saddr[1] = (uint16_t)(u32Data >> 16);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.dns_srvport = (uint32_t)(u32Data);

    for (i = 0; i < 32; i += 4) {
        u32Data = atp_nv_read_word(u32Addr++);
        at_show.dns_srvname[i] = (uint8_t)(u32Data);
        at_show.dns_srvname[i + 1] = (uint8_t)(u32Data >> 8);
        at_show.dns_srvname[i + 2] = (uint8_t)(u32Data >> 16);
        at_show.dns_srvname[i + 3] = (uint8_t)(u32Data >> 24);
    }
    for (i = 0; i < 32; i += 4) {
        u32Data = atp_nv_read_word(u32Addr++);
        at_show.dns_srvpath[i] = (uint8_t)(u32Data);
        at_show.dns_srvpath[i + 1] = (uint8_t)(u32Data >> 8);
        at_show.dns_srvpath[i + 2] = (uint8_t)(u32Data >> 16);
        at_show.dns_srvpath[i + 3] = (uint8_t)(u32Data >> 24);
    }

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.dns_srvport = (uint16_t)(u32Data);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.keepalive_mode = (uint8_t)(u32Data);
    at_show.keepalive_no_data_period = (uint8_t)(u32Data >> 8);
    at_show.keepalive_send_period = (uint8_t)(u32Data >> 16);
    at_show.keepalive_send_count = (uint8_t)(u32Data >> 24);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.baudrate = (uint32_t)(u32Data);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.wordlen = (uint8_t)(u32Data);
    at_show.parity = (uint8_t)(u32Data >> 8);
    at_show.stop = (uint8_t)(u32Data >> 16);

    u32Data = atp_nv_read_word(u32Addr++);
    at_show.trans_len = (uint16_t)(u32Data);
    at_show.rst_count = (uint16_t)(u32Data >> 16);

    u32Data = atp_nv_read_word(u32Addr++);
    eeprom_show.macaddr[0] = (uint8_t)(u32Data >> 16);
    eeprom_show.macaddr[1] = (uint8_t)(u32Data >> 24);

    u32Data = atp_nv_read_word(u32Addr++);
    eeprom_show.macaddr[2] = (uint8_t)(u32Data);
    eeprom_show.macaddr[3] = (uint8_t)(u32Data >> 8);
    eeprom_show.macaddr[4] = (uint8_t)(u32Data >> 16);
    eeprom_show.macaddr[5] = (uint8_t)(u32Data >> 24);

    u32Data = atp_nv_read_word(u32Addr++);
    eeprom_show.autoload = (uint16_t)(u32Data);

    u32Data = atp_nv_read_word(u32Addr++);
    eeprom_show.vid = (uint16_t)u32Data;
    eeprom_show.pid = (uint16_t)(u32Data >> 16);

    u32Data = atp_nv_read_word(u32Addr++);
    eeprom_show.pincontrol = (uint16_t)u32Data;
    eeprom_show.wakeupcontrol2 = (uint16_t)(u32Data >> 16);

    u32Data = atp_nv_read_word(u32Addr++);
    eeprom_show.control3 = (uint16_t)u32Data;

    return mflag;
}

uint8_t Read_AT_DataFlash(void)
{
    uint8_t mflag = Read_AT_Show_DataFlash(1);
    at_type = at_show;
    eeprom_type = eeprom_show;
    return mflag;
}
