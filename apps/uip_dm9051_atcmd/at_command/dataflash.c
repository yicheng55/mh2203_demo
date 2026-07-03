#include <stdio.h>
#include "at_port.h"
#include "atcommand.h"
#include "dataflash.h"

extern struct at_funcation at_show;
extern struct eeprom_funcation eeprom_show;

void Copy_AT_Type_to_Show(void)
{
		at_show = at_type;
		eeprom_show = eeprom_type;
}

#ifdef TL_FLASH_NOT_WRITE_DBG
uint32_t u32Addr_d[41]; //programmer count and get 41 words.
#endif

/* u32Idx: 相對 word 索引 (0 起算)，取代原本的絕對 flash 位址。
 * 底層寫入透過 at_port 的 atp_nv_write_word()，版面順序與 AT32 原版一致。 */
uint32_t Write_AT_Data(uint32_t u32Idx, uint32_t u32Data)
{
		atp_nv_write_word(u32Idx, u32Data);
	#ifdef TL_FLASH_NOT_WRITE_DBG
		u32Addr_d[u32Idx] = u32Data;
	#endif
		return u32Idx + 1;
}

void Write_AT_Show_DataFlash(void)
{
#ifdef TL_FLASH_NOT_WRITE_DBG
		uint8_t n;
#endif
		uint32_t u32Data, u32Addr;
		uint8_t i;

		atp_nv_erase();

		{
			//word(-12) - Write TEST_PATTERN to Flash, Check Flash not empty
			u32Addr = Write_AT_Data(0, TEST_PATTERN);

			//word(-11) - Role mode and DHCP Switch
			if(at_show.dhcpc_mode)
				u32Data = at_show.role | 0x8;
			else
				u32Data = at_show.role & 0x7;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
					
			//word(-10) - IP Address
			u32Data = (at_show.hostip[1] << 16) + at_show.hostip[0];
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//word(-9) - Netmask Address
			u32Data = (at_show.hostmask[1] << 16) + at_show.hostmask[0];
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//word(-8) - Getway Address
			u32Data = (at_show.hostgw[1] << 16) + at_show.hostgw[0];
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//word(-7) - TCP Listen Port
			u32Data = at_show.t_lport;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//word(-6) - TCP Client Remote Address
			u32Data = (at_show.tcp_raddr[1] << 16) + at_show.tcp_raddr[0];
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//TCP Client Remote Port
			u32Data = at_show.tcp_rport;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//word(-4) - UDP Listen Port
			u32Data = at_show.u_lport;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			//word(-3) - Static UDP Client Remote Address
			u32Data = (at_show.udp_raddr[1] << 16) + at_show.udp_raddr[0];
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			//UDP Client Remote Port
			u32Data = at_show.udp_rport;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//12.word(-1) - DNS mode
			u32Data = at_show.dns_mode;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			//13.word0 - DNS Server IP
			u32Data = (at_show.dns_saddr[1] << 16) + (at_show.dns_saddr[0]);
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			//14.word1 - DNS Server IP
			u32Data = at_show.dns_srvport;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//15-22.word2 - DNS Server Name ( x 8 )
			for(i = 0; i < 32 ; i += 4){
				u32Data = (at_show.dns_srvname[i + 3] << 24) + (at_show.dns_srvname[i + 2] << 16) +
										(at_show.dns_srvname[i + 1] << 8) + (at_show.dns_srvname[i]);
				u32Addr = Write_AT_Data(u32Addr, u32Data);
			}
			//23-30.word3 - DNS Server Path ( x 8 )
			for(i = 0; i < 32 ; i += 4){
				u32Data = (at_show.dns_srvpath[i + 3] << 24) + (at_show.dns_srvpath[i + 2] << 16) +
										(at_show.dns_srvpath[i + 1] << 8) + (at_show.dns_srvpath[i]);
				u32Addr = Write_AT_Data(u32Addr, u32Data);
//				display_flash_data8SrvPath(u32Addr_s, u32Addr, u32Data); //printf(".HINT ..
			}
			//31.word4 - DNS Server Port
			u32Data = (uint16_t)at_show.dns_srvport;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//32.wort5 - keepalive mode
			u32Data = (at_show.keepalive_send_count << 24 ) + (at_show.keepalive_send_period << 16) 
									+ (at_show.keepalive_no_data_period << 8) + (at_show.keepalive_mode);
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//33.word6 - Baudrate value
			u32Data = at_show.baudrate;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			//34.
			u32Data = (at_show.stop << 16) + (at_show.parity << 8) + (at_show.wordlen);
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			//35.
			u32Data = (at_show.rst_count << 16) + (at_show.trans_len);
			u32Addr = Write_AT_Data(u32Addr, u32Data);
		
			//EEPROM Data******************************************************************************************
			//36.MAC address to DataFlash
			u32Data = (eeprom_show.macaddr[1] << 24) + (eeprom_show.macaddr[0] << 16);
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			//37.
			u32Data = (eeprom_show.macaddr[5] << 24) + (eeprom_show.macaddr[4] << 16) + 
								(eeprom_show.macaddr[3] << 8) + (eeprom_show.macaddr[2]);
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//38.word11
			u32Data = eeprom_show.autoload;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
		
			//39.word12 - VID/PID
			u32Data = eeprom_show.pid;
			u32Data = (u32Data << 16) + eeprom_show.vid;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//40.word13 - 
			u32Data = eeprom_show.wakeupcontrol2;
			u32Data = (u32Data << 16) + eeprom_show.pincontrol;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//41.word14 - 
			u32Data = eeprom_show.control3;
			u32Addr = Write_AT_Data(u32Addr, u32Data);
			
			//42.word15 - 
//			.u32Data = eeprom_show.ee_lastport;
//			u32Addr = Write_AT_Data(u32Addr, u32Data);
		}

		atp_nv_commit();
#ifdef TL_FLASH_NOT_WRITE_DBG
		printf(".wrFlash");
		n = u32Addr;
		for(i = 0; i < n ; i ++){
			if (!(i%8))
				printf("\r\n");
			display_flash_data(i, u32Addr_d[i]); //printf(".HINT ..
		}
#endif
}

/**
* @brief  This function handles Write AT DATA to Flash request.
* @param  None
* @retval None
*/
void Write_AT_DataFlash(void)
{
		/* Don't printf() while under FLASH_Inlock() - i.e. writing flash stage! (jos)
		 */
//#ifdef TL_FLASH_NOT_WRITE_DBG
//		uint8_t i, n;
//#endif
		Copy_AT_Type_to_Show();
		Write_AT_Show_DataFlash();
//#ifdef TL_FLASH_NOT_WRITE_DBG
//		printf(".wrFlash");
//		n = ((u32Addr- StartAddress) >> 2) + 1;
//		for(i = 0; i < n ; i ++){
//			if (!(i%8))
//				printf("\r\n");
//			display_flash_data(StartAddress + (i << 2), u32Addr_d[i]); //printf(".HINT ..
//		}
//#endif

#if 0
//		u32Addr = StartAddress;
//			// Write TEST_PATTERN to Flash, Check Flash not empty
//			display_flash_data(u32Addr, test_addr); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, test_addr); //FLASHStatus = FLASH_ProgramWord(u32Addr, test_addr);
//			
//			//Ethernet App Mode & DHCP Client State
//			u32Addr += 4;
//			u32Data = at_type.role;
//			if(at_type.dhcpc_mode){
//				u32Data |= 0x8;
//			}else {
//				u32Data &= 0x7;
//			}
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramHalfWord(u32Addr, u32Data);
//					
//			//IP Address
//			u32Addr += 4;
//			u32Data = (at_type.hostip[1] << 16) + at_type.hostip[0];
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//Netmask Address
//			u32Addr += 4;
//			u32Data = (at_type.hostmask[1] << 16) + at_type.hostmask[0];
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//Getway Address
//			u32Addr += 4;
//			u32Data = (at_type.hostgw[1] << 16) + at_type.hostgw[0];
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//TCP Listen Port
//			u32Addr += 4;
//			u32Data = at_type.t_lport;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//TCP Client Remote Address
//			u32Addr += 4;
//			u32Data = (at_type.tcp_raddr[1] << 16) + at_type.tcp_raddr[0];
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			u32Addr += 4;//TCP Client Remote Port
//			u32Data = at_type.tcp_rport;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//UDP Listen Port
//			u32Addr += 4;
//			u32Data = at_type.u_lport;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//Static UDP Client Remote Address
//			u32Addr += 4;
//			u32Data = (at_type.udp_raddr[1] << 16) + at_type.udp_raddr[0];
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			u32Addr += 4;//UDP Client Remote Port
//			u32Data = at_type.udp_rport;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//DNS mode
//			u32Addr += 4;
//			u32Data = at_type.dns_mode;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//DNS Server IP
//			u32Addr += 4;
//			u32Data = (at_type.dns_saddr[1] << 16) + (at_type.dns_saddr[0]);
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			u32Addr += 4;
//			u32Data = at_type.dns_srvport;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//DNS Server Name
//			for(i = 0; i < 32 ; i += 4){
//				u32Addr += 4;
//				u32Data = (at_type.dns_srvname[i + 3] << 24) + (at_type.dns_srvname[i + 2] << 16) +
//										(at_type.dns_srvname[i + 1] << 8) + (at_type.dns_srvname[i]);
//				display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			}
//			//DNS Server Path
//			for(i = 0; i < 32 ; i += 4){
//				u32Addr += 4;
//				u32Data = (at_type.dns_srvpath[i + 3] << 24) + (at_type.dns_srvpath[i + 2] << 16) +
//										(at_type.dns_srvpath[i + 1] << 8) + (at_type.dns_srvpath[i]);
//				display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			}
//			//DNS Server Port
//			u32Addr += 4;
//			u32Data = (uint16_t)at_type.dns_srvport;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//keepalive mode
//			u32Addr += 4;
//			u32Data = (at_type.keepalive_send_count << 24 ) + (at_type.keepalive_send_period << 16) 
//									+ (at_type.keepalive_no_data_period << 8) + (at_type.keepalive_mode);
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//Baudrate value
//			u32Addr += 4;
//			u32Data = at_type.baudrate;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			u32Addr += 4;
//			u32Data = (at_type.stop << 16) + (at_type.parity << 8) + (at_type.wordlen);
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			u32Addr += 4;
//			u32Data = (at_type.rst_count << 16) + (at_type.trans_len);
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//		
//			//EEPROM Data******************************************************************************************
//			//MAC address to DataFlash
//			u32Addr += 4;
//			u32Data = (eeprom_type.macaddr[1] << 24) + (eeprom_type.macaddr[0] << 16);
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			u32Addr += 4;
//			u32Data = (eeprom_type.macaddr[5] << 24) + (eeprom_type.macaddr[4] << 16) + 
//								(eeprom_type.macaddr[3] << 8) + (eeprom_type.macaddr[2]);
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//word11
//			u32Addr += 4;
//			u32Data = eeprom_type.autoload;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//		
//			//word12 - VID/PID
//			u32Addr += 4;
//			u32Data = eeprom_type.pid;
//			u32Data = (u32Data << 16) + eeprom_type.vid;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//word13 - 
//			u32Addr += 4;
//			u32Data = eeprom_type.wakeupcontrol2;
//			u32Data = (u32Data << 16) + eeprom_type.pincontrol;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//word14 - 
//			u32Addr += 4;
//			u32Data = eeprom_type.control3;
//			display_flash_data(u32Addr, u32Data); //printf(".HINT flash %2u, %8x\r\n", u32Addr >> 2, u32Data); //FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
#endif

		//.before +OK
//		atcmd_resp_cmd("Note: _Write_AT_DataFlash");
}

//uint8_t cvblank(uint8_t b) {
//	if (b == ' ') {
//		printf("dbg-name has blank\r\n");
//		return 0;
//	}
//	return b;
//}

//uint32_t DataCheckLog(uint32_t u32Data)
//{
//	uint32_t dw;
//	uint8_t b;
//	
//	b = (uint8_t)(u32Data);
//	dw = cvblank(b);
//	
//	b = (uint8_t)(u32Data >> 8);
//	dw = cvblank(b) << 8;
//	
//	b = (uint8_t)(u32Data >> 16);
//	dw = cvblank(b) << 16;
//	
//	b = (uint8_t)(u32Data >> 24);
//	dw = cvblank(b) << 24;
//	
//	return dw;
//}

int AT_Pass_Custom_Mode(void)
{
	if(at_type.rst_count < 200){
		Write_AT_DataFlash();	//init
	}
	
	if(at_type.rst_count >= 200)
	{
		_printf("Erase FullChip\n");
		return 0;
	}

	return 1;
}

uint32_t Read_AT_Data(uint32_t *p32Idx)
{
	uint32_t u32Data = atp_nv_read_word(*p32Idx);
#ifdef TL_FLASH_NOT_WRITE_DBG
	u32Addr_d[*p32Idx] = u32Data;
#endif
	*p32Idx += 1;
	return u32Data;
}

uint8_t Read_AT_Show_DataFlash(uint8_t inc)
{
#ifdef TL_FLASH_NOT_WRITE_DBG
#if 0
		uint8_t n;
#endif
#endif
		uint8_t i;
		uint8_t	mflag = 0;
		uint32_t u32Addr;
		uint32_t u32Data;

		u32Addr = 0;

	  //Check data flash was programmed?
		//u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		if(u32Data != TEST_PATTERN) {
			_printf("DataEmp\r\n");
			mflag = 2;
			goto lexit;
		}
	
		//Role mode and DHCP Switch
		//u32Addr += 4;
		//u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.role = (u32Data & 0x7);
		if(u32Data & 0x8) {
			at_show.dhcpc_mode = 1;
		}else {
			at_show.dhcpc_mode = 0;
		}
		
		//Host IP Addess
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.hostip[0] = (uint16_t)(u32Data);
		at_show.hostip[1] = (uint16_t)(u32Data >> 16);
		
		//Host Netmask Addess
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.hostmask[0] = (uint16_t)(u32Data);
		at_show.hostmask[1] = (uint16_t)(u32Data >> 16);
		
		//Host Getway Addess
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.hostgw[0] = (uint16_t)(u32Data);
		at_show.hostgw[1] = (uint16_t)(u32Data >> 16);
		
		//TCP listen Port
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.t_lport = (uint32_t)(u32Data);
		
		//Static TCP Client Remote Address
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.tcp_raddr[0] = (uint16_t)(u32Data);
		at_show.tcp_raddr[1] = (uint16_t)(u32Data >> 16);
//		u32Addr += 4;//Static TCP Client Remote Port
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.tcp_rport = (uint32_t)u32Data;
		
		//UDP listen Port
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.u_lport = (uint32_t)(u32Data);
		
		//Static UDP Client Remote Address
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.udp_raddr[0] = (uint16_t)(u32Data);
		at_show.udp_raddr[1] = (uint16_t)(u32Data >> 16);		
//		u32Addr += 4;//Static UDP Client Remote Port
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.udp_rport = (uint32_t)(u32Data);
		
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.dns_mode = (uint8_t)(u32Data);
		//DNS SERVER　IP
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.dns_saddr[0] = (uint16_t)(u32Data);
		at_show.dns_saddr[1] = (uint16_t)(u32Data >> 16);
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.dns_srvport = (uint32_t)(u32Data);

		//DNS Server Name
		for(i = 0; i < 32; i += 4){
//			u32Addr += 4;
//			u32Data =  *(volatile uint32_t *)u32Addr;
			//u32Data = DataCheckLog(u32Data);
			u32Data = Read_AT_Data(&u32Addr);
			at_show.dns_srvname[i] = (uint8_t)(u32Data);
			at_show.dns_srvname[i + 1] = (uint8_t)(u32Data >> 8);
			at_show.dns_srvname[i + 2] = (uint8_t)(u32Data >> 16);
			at_show.dns_srvname[i + 3] = (uint8_t)(u32Data >> 24);
		}
		//DNS Server Path
		for(i = 0; i < 32; i += 4){
//			u32Addr += 4;
//			u32Data =  *(volatile uint32_t *)u32Addr;
			u32Data = Read_AT_Data(&u32Addr);
			at_show.dns_srvpath[i] = (uint8_t)(u32Data);
			at_show.dns_srvpath[i + 1] = (uint8_t)(u32Data >> 8);
			at_show.dns_srvpath[i + 2] = (uint8_t)(u32Data >> 16);
			at_show.dns_srvpath[i + 3] = (uint8_t)(u32Data >> 24);
		}
		//keep alive
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.dns_srvport = (uint16_t)(u32Data);
		
		//keep alive
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.keepalive_mode = (uint8_t)(u32Data);
		at_show.keepalive_no_data_period = (uint8_t)(u32Data >> 8);
		at_show.keepalive_send_period = (uint8_t)(u32Data >> 16);
		at_show.keepalive_send_count = (uint8_t)(u32Data >> 24);
		
		//Buadrate value 
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.baudrate = (uint32_t)(u32Data);
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.wordlen = (uint8_t)(u32Data);
		at_show.parity = (uint8_t)(u32Data >> 8);
		at_show.stop = (uint8_t)(u32Data >> 16);
		
		//transparent lenght and reset counter 
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		at_show.trans_len = (uint16_t)(u32Data);
		at_show.rst_count = (uint16_t)(u32Data >> 16);
#ifdef CUS_TEST
		if(showp_read_flag != 1) {
			at_show.rst_count = at_type.rst_count; //in xx case.
			at_show.rst_count++;
		}
#endif //CUS_TEST
		
		//EEPROM Data***********************************************************
		//MAC Address
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		eeprom_show.macaddr[0] = (uint8_t) (u32Data >> 16);
		eeprom_show.macaddr[1] = (uint8_t) (u32Data >> 24);
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		eeprom_show.macaddr[2] = (uint8_t) (u32Data);
		eeprom_show.macaddr[3] = (uint8_t) (u32Data >> 8);
		eeprom_show.macaddr[4] = (uint8_t) (u32Data >> 16);
		eeprom_show.macaddr[5] = (uint8_t) (u32Data >> 24);
	
		//word11 
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		eeprom_show.autoload = (uint16_t)(u32Data);

		//word12 
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		eeprom_show.vid = (uint16_t)u32Data;
		eeprom_show.pid = (uint16_t)(u32Data >> 16);
		
		//word13 
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		eeprom_show.pincontrol = (uint16_t)u32Data;
		eeprom_show.wakeupcontrol2 = (uint16_t)(u32Data >> 16);
		
		//word14 
//		u32Addr += 4;
//		u32Data =  *(volatile uint32_t *)u32Addr;
		u32Data = Read_AT_Data(&u32Addr);
		eeprom_show.control3 = (uint16_t)u32Data;

#if JOSRANDOM
		if (inc) {
			//word.new.page
			u32Addr = StartAddress2;
			u32Data = Read_AT_Data(&u32Addr);
			eeprom_show.ee_lastport = u32Data;
			
//			printf("RD.1 [ee_lastport]= .flash %x, %u (next %x)\r\n", StartAddress2, eeprom_show.ee_lastport, u32Addr);

			//word.new.page-write
			FLASH_Unlock();
			FLASH_ClearFlag(FLASH_FLAG_PRGMFLR | FLASH_FLAG_PRCDN);
			FLASHStatus = FLASH_ErasePage(StartAddress2);
			if(FLASHStatus == FLASH_PRC_DONE){
				u32Addr = StartAddress2;
				Write_AT_Data(u32Addr, eeprom_show.ee_lastport + 1);
			}
			FLASH_Lock();
			
			u32Addr = StartAddress2;
			u32Data = Read_AT_Data(&u32Addr);
//			printf("RD.2 [ee_lastport]= .flash %x, %u (next %x)\r\n", StartAddress2, u32Data, u32Addr);
		}
#endif
		
#ifdef TL_FLASH_NOT_WRITE_DBG
 #if 0
		printf(".rdFlash");
		n = ((u32Addr- StartAddress) >> 2) + 1;
		for(i = 0; i < n ; i ++){
			if (!(i%8))
				printf("\r\n");
			display_flash_data(StartAddress + (i << 2), u32Addr_d[i]); //printf(".HINT ..
		}
 #endif
#endif
		
lexit:  	
		return mflag;
}

/**
* @brief  This function handles Read AT DATA for Flash request.
* @param  None
* @retval None
*/
uint8_t Read_AT_DataFlash(void)
{
		/* Don't printf() while under FLASH_Inlock() - i.e. writing flash stage! (jos)
		 */
		uint8_t	mflag = Read_AT_Show_DataFlash(1);

		at_type = at_show;
		eeprom_type = eeprom_show;

		return mflag;
}

//#define FDATE_00START				0
//#define FDATE_01ROLE_DHCPC			1
//#define FDATE_02IP					2
//#define FDATE_03MASK				3
//#define FDATE_04GW					4
//#define FDATE_05TCP_LP				5
//#define FDATE_06TCP_SCONN_R_IP		6
//#define FDATE_07TCP_SCONN_R_PORT	7
//#define FDATE_08UDP_LP				8
//#define FDATE_09UDP_SCONN_R_IP		9
//#define FDATE_10UDP_SCONN_R_PORT	10
//#define FDATE_11DNS_MODE			11
//#define FDATE_12DNS_Server_IP		12
//#define FDATE_13DNS_Server_Port		13
//#define FDATE_14DNS_Server_Name_x8	14
////#define FDATE_15
////#define FDATE_16
////#define FDATE_17
////#define FDATE_18
////#define FDATE_19
////#define FDATE_20
////#define FDATE_21
//#define FDATE_22DNS_Server_Path_x8	22
////#define FDATE_23
////#define FDATE_24
////#define FDATE_23
////#define FDATE_26
////#define FDATE_27
////#define FDATE_28
////#define FDATE_29
//#define FDATE_30DNS_Server_Port		30
//#define FDATE_31keepalive_mode		31
//#define FDATE_32Baudrate_value		32
//#define FDATE_33n81_value			33
//#define FDATE_34trans_len_rst_count 34
//#define FDATE_35MAC_AddrW			35
//#define FDATE_36MAC_AddrDW			36
//#define FDATE_37eeprom_autoload		37
//#define FDATE_38eeprom_pid			38
//#define FDATE_39eeprom_wakeu_pinctl	39
//#define FDATE_40eeprom_control3		40
//#define FDATE_TOTAL_NUM				41

//void Write_SYSCFG_DataFlash(void)
//{
//		uint32_t u32Data;
//		uint32_t u32Addr;
//		//uint32_t test_addr = TEST_PATTERN;
//		uint8_t i;
//		FLASH_Unlock();
//	  FLASH_ClearFlag(FLASH_FLAG_PRGMFLR | FLASH_FLAG_PRCDN);
//		FLASHStatus = FLASH_ErasePage(StartAddress);

//	  if(FLASHStatus == FLASH_PRC_DONE){
//		u32Addr = StartAddress;
//			//CFG Data******************************************************************************************
//			//BE changed by webpage, 
//			//.word0
//			//TEST.PATTERN
//			FLASHStatus = FLASH_ProgramWord(StartAddress, TEST_PATTERN);
//			//.word1
//			//"tnmode" and "staticip" (oppsite value!)
//			if(at_show.dhcpc_mode){
//				u32Data = at_show.role | 0x8;
//			}else {
//				u32Data = at_show.role & 0x7;
//			}
//			printf("flh role %u, dhcpc %u\r\n", at_show.role, at_show.dhcpc_mode);
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramHalfWord(u32Addr, u32Data);
//			//word2
//			//sip1/sip2/sip3/sip4
//			u32Data = (at_show.hostip[1] << 16) + at_show.hostip[0];
//			printf("flh ip %u.%u.%u.%u\r\n", uip_ipaddr1(at_show.hostip), uip_ipaddr2(at_show.hostip),
//					uip_ipaddr3(at_show.hostip), uip_ipaddr4(at_show.hostip));
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word3
//			//mip1/mip2/mip3/mip4
//			u32Data = (at_show.hostmask[1] << 16) + at_show.hostmask[0];
//			printf("flh mask %u.%u.%u.%u\r\n", uip_ipaddr1(at_show.hostmask), uip_ipaddr2(at_show.hostmask),
//					uip_ipaddr3(at_show.hostmask), uip_ipaddr4(at_show.hostmask));
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word4
//			//gip1/gip2/gip3/gip4
//			u32Data = (at_show.hostgw[1] << 16) + at_show.hostgw[0];
//			printf("flh ip %u.%u.%u.%u\r\n", uip_ipaddr1(at_show.hostgw), uip_ipaddr2(at_show.hostgw),
//					uip_ipaddr3(at_show.hostgw), uip_ipaddr4(at_show.hostgw));
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word5
//			//"tlp"1/2
//			u32Data = at_show.t_lport;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word6
//			//trip1/trip2/trip3/trip4 1/2
//			u32Data = (at_show.tcp_raddr[1] << 16) + at_show.tcp_raddr[0];
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word7 (TCP Client Remote Port)
//			//"trp"1/2
//			u32Data = at_show.tcp_rport;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word8 (UDP Listen Port)
//			//"tlp"2/2
//			u32Data = at_show.u_lport;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word9 (Static UDP Client Remote Address)
//			//trip1/trip2/trip3/trip4 2/2
//			u32Data = (at_show.udp_raddr[1] << 16) + at_show.udp_raddr[0];
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word10 (UDP Client Remote Port)
//			//"trp"2/2
//			u32Data = at_show.udp_rport;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//word11 [DNS mode]
//			//"dnsmode"
//			u32Data = at_show.dns_mode;
//			printf("flh dns_mode %u\r\n", at_show.dns_mode);
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word12
//			//dip1/dip2/dip3/dip4
//			u32Data = (at_show.dns_saddr[1] << 16) + (at_show.dns_saddr[0]);
//			printf("flh dnssrv,ip %u.%u.%u.%u\r\n", uip_ipaddr1(at_show.dns_saddr), uip_ipaddr2(at_show.dns_saddr),
//					uip_ipaddr3(at_show.dns_saddr), uip_ipaddr4(at_show.dns_saddr));
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word13
//			//[NOUSED.]
//			//CANNOT change by webpage, at_type. ok
//			u32Data = at_type.dns_srvport;
////			u32Data = at_show.dns_srvport;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word14~21
//			//TO change by webpage, at_show. ok
//			printf("flh dns_srvname: ");
////			if (srvname_valid_ok(at_show.dns_srvname)) {
//				for(i = 0; i < 32 ; i += 4){
////					u32Data = (at_type.dns_srvname[i + 3] << 24) + (at_type.dns_srvname[i + 2] << 16) +
////											(at_type.dns_srvname[i + 1] << 8) + (at_type.dns_srvname[i]);
//					u32Data = (at_show.dns_srvname[i + 3] << 24) + (at_show.dns_srvname[i + 2] << 16) +
//											(at_show.dns_srvname[i + 1] << 8) + (at_show.dns_srvname[i]);
//					
//				u32Addr += 4;
//				FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//				}
////			} else
////				printf("(invalid name)");
//			printf("\r\n");
//			//word22~29
//			//[NOUSED.], at_type. ok
//			for(i = 0; i < 32 ; i += 4){
//				u32Data = (at_type.dns_srvpath[i + 3] << 24) + (at_type.dns_srvpath[i + 2] << 16) +
//										(at_type.dns_srvpath[i + 1] << 8) + (at_type.dns_srvpath[i]);
////				u32Data = (at_show.dns_srvpath[i + 3] << 24) + (at_show.dns_srvpath[i + 2] << 16) +
////										(at_show.dns_srvpath[i + 1] << 8) + (at_show.dns_srvpath[i]);
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			}
//			//word30 [DNS Server Port]
//			//[NOUSED.]
//			//CANNOT change by webpage, at_type. ok
//			u32Data = (uint16_t)at_type.dns_srvport;
////			u32Data = (uint16_t)at_show.dns_srvport;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word31 [keepalive mode]
//			//"ka"
//			//ONLY at_show.keepalive_mode
//			u32Data = (at_type.keepalive_send_count << 24 ) + (at_type.keepalive_send_period << 16) 
//									+ (at_type.keepalive_no_data_period << 8) + (at_show.keepalive_mode);
////			u32Data = (xxxxxxx.keepalive_send_count << 24 ) + (xxxxxxx.keepalive_send_period << 16) 
////									+ (xxxxxxx.keepalive_no_data_period << 8) + (at_show.keepalive_mode);
//			printf("flh keepalive %u\r\n", at_show.keepalive_mode);
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word32
//			//"bd"
//			u32Data = at_show.baudrate;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word33
//			//"wln"+"pty"+"stop"
//			u32Data = (at_show.stop << 16) + (at_show.parity << 8) + (at_show.wordlen);
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word34 [_RST_COUNT + _TRANS_LEN]
//			//[NOUSED.]
//			//CANNOT change by webpage, at_type. ok
//			u32Data = (at_type.rst_count << 16) + (at_type.trans_len);
////			u32Data = (at_show.rst_count << 16) + (at_show.trans_len);
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			
//			//EEPROM Data******************************************************************************************
//		#if 1
//			//CANNOT change by webpage, ONLY RE-WRITE AFTER ERAGE PAGE!
//			//word35
//			u32Data = (eeprom_type.macaddr[1] << 24) + (eeprom_type.macaddr[0] << 16);
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word36
//			u32Data = (eeprom_type.macaddr[5] << 24) + (eeprom_type.macaddr[4] << 16) + 
//					  (eeprom_type.macaddr[3] << 8) + (eeprom_type.macaddr[2]);
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word37
//			u32Data = eeprom_type.autoload;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word38
//			u32Data = (eeprom_type.pid << 16) + eeprom_type.vid;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word39
//			u32Data = (eeprom_type.wakeupcontrol2 << 16) + eeprom_type.pincontrol;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//			//word40
//			u32Data = eeprom_type.control3;
//			u32Addr += 4;
//			FLASHStatus = FLASH_ProgramWord(u32Addr, u32Data);
//		#endif
//	  }
//		FLASH_Lock();
//}

//static uint32_t Read_AT_FlashDWord(uint32_t u32WordIndex)
//{
//		return *(volatile uint32_t *)(StartAddress + (u32WordIndex << 2));
//}

//void Read_AT_DataFlash_to_Show(void)
//{
//		uint8_t u8Data; //role, dhcpc_mod,
//		uint16_t u16Data;
//		uint32_t u32Data, u32Data1;
//		uint8_t i;

//	  //Check data flash was programmed, first?
//		u32Data = Read_AT_FlashDWord(FDATE_00START);
//		if(u32Data == TEST_PATTERN) {
//				u32Data = Read_AT_FlashDWord(FDATE_01ROLE_DHCPC);
//				at_show.role= (u32Data & 0x7);
//				at_show.dhcpc_mode= (u32Data & 0x8) ? 1 : 0;;
//				u8Data = Read_AT_FlashDWord(FDATE_31keepalive_mode);
//				at_show.keepalive_mode= u8Data;
//				u8Data = Read_AT_FlashDWord(FDATE_11DNS_MODE);
//				at_show.dns_mode= u8Data;
//				u16Data = (uint16_t) Read_AT_FlashDWord(FDATE_34trans_len_rst_count);
//				at_show.trans_len= u16Data;
//				// mac (eeprom_type)
//				 //
//				u32Data = Read_AT_FlashDWord(FDATE_35MAC_AddrW);
//				eeprom_show.macaddr[0] = (uint8_t)(u32Data >> 16);
//				eeprom_show.macaddr[1] = (uint8_t)(u32Data >> 24);
//				u32Data1 = Read_AT_FlashDWord(FDATE_36MAC_AddrDW);
//				eeprom_show.macaddr[2]= (uint8_t)(u32Data1);
//				eeprom_show.macaddr[3]= (uint8_t)(u32Data1 >> 8);
//				eeprom_show.macaddr[4]= (uint8_t)(u32Data1 >> 16);
//				eeprom_show.macaddr[5]= (uint8_t)(u32Data1 >> 24);
//				u32Data = Read_AT_FlashDWord(FDATE_02IP);
//				at_show.hostip[0] = (uint16_t)(u32Data);
//				at_show.hostip[1] = (uint16_t)(u32Data >> 16);
//				u32Data = Read_AT_FlashDWord(FDATE_03MASK);
//				at_show.hostmask[0] = (uint16_t)(u32Data);
//				at_show.hostmask[1] = (uint16_t)(u32Data >> 16);
//				u32Data = Read_AT_FlashDWord(FDATE_04GW);
//				at_show.hostgw[0] = (uint16_t)(u32Data);
//				at_show.hostgw[1] = (uint16_t)(u32Data >> 16);
//				u16Data = Read_AT_FlashDWord(FDATE_05TCP_LP);
//				at_show.t_lport= u16Data;
//				u32Data = Read_AT_FlashDWord(FDATE_06TCP_SCONN_R_IP);
//				at_show.tcp_raddr[0] = (uint16_t)(u32Data);
//				at_show.tcp_raddr[1] = (uint16_t)(u32Data >> 16);
//				u16Data = Read_AT_FlashDWord(FDATE_07TCP_SCONN_R_PORT);
//				at_show.tcp_rport= u16Data;
//				// UDP Listen Port
//				u16Data = Read_AT_FlashDWord(FDATE_08UDP_LP);
//				at_show.u_lport= u16Data;
//				// UDP Client Remote Address & Remote Port 
//				u32Data = Read_AT_FlashDWord(FDATE_09UDP_SCONN_R_IP);
//				at_show.udp_raddr[0] = (uint16_t)(u32Data);
//				at_show.udp_raddr[1] = (uint16_t)(u32Data >> 16);
//				u16Data = Read_AT_FlashDWord(FDATE_10UDP_SCONN_R_PORT);
//				at_show.udp_rport= u16Data;
//				// DNS Server IP 
//				u32Data = Read_AT_FlashDWord(FDATE_12DNS_Server_IP);
//				at_show.dns_saddr[0] = (uint16_t)(u32Data);    //DNS Server IP
//				at_show.dns_saddr[1] = (uint16_t)(u32Data >> 16);
//				
//				for(i = 0; i < 8; i++){
//					u32Data = Read_AT_FlashDWord(FDATE_14DNS_Server_Name_x8 + i);
//					at_show.dns_srvname[(i << 2)] = (uint8_t)(u32Data);
//					at_show.dns_srvname[(i << 2) + 1] = (uint8_t)(u32Data >> 8);
//					at_show.dns_srvname[(i << 2) + 2] = (uint8_t)(u32Data >> 16);
//					at_show.dns_srvname[(i << 2) + 3] = (uint8_t)(u32Data >> 24);
//				}

//				u16Data = Read_AT_FlashDWord(FDATE_13DNS_Server_Port);
//				at_show.dns_srvport= u16Data;        //manual setup _server port
//				// Setup Baudrate 
//				u32Data = Read_AT_FlashDWord(FDATE_32Baudrate_value);
//				at_show.baudrate= u32Data;
//				u32Data = Read_AT_FlashDWord(FDATE_33n81_value);
//				at_show.wordlen= (uint8_t)(u32Data);
//				at_show.parity= (uint8_t)(u32Data >> 8);
//				at_show.stop= (uint8_t)(u32Data >> 16);
//				// autoload.., etc (eeprom_type)
//				 //
//				eeprom_show.autoload = (uint16_t) Read_AT_FlashDWord(FDATE_37eeprom_autoload);
//				u32Data = Read_AT_FlashDWord(FDATE_39eeprom_wakeu_pinctl);
//				eeprom_show.pincontrol= (uint16_t)u32Data;
//				eeprom_show.wakeupcontrol2= (uint16_t)(u32Data >> 16);
//				eeprom_show.control3 = (uint8_t) Read_AT_FlashDWord(FDATE_40eeprom_control3);
//				u32Data = Read_AT_FlashDWord(FDATE_38eeprom_pid);
//				eeprom_show.vid= (uint16_t)u32Data;
//				eeprom_show.pid= (uint16_t)(u32Data >> 16);
//		}
//		else {
//			atcmd_resp_cmd("DataEmp");
//			_printf("DataEmp\r\n");
//		} 
//}
