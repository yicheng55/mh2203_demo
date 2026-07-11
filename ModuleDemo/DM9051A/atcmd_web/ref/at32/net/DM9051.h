/*********************************************************************************************************//**
* @file    DM9051_stm.h
* @version V1.01
* @date    2014/6/10
* @brief   The header file of DM9051 register.
*************************************************************************************************************
*
* <h2><center>Copyright (C) 2014 DAVICOM Semiconductor Inc. All rights reserved</center></h2>
*
************************************************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------------------------------------*/
#ifndef __DM9051_REG_H
#define __DM9051_REG_H

/* Includes ------------------------------------------------------------------------------------------------*/
#include "includes.h"

//#define LwIP_EN             			// Use LwIP, This define mask use uIP
#ifdef LwIP_EN
#include "ethernetif.h"
#endif	//uIP_NOOS

/* Settings ------------------------------------------------------------------------------------------------*/
/************************* DM9051 Driver Private define **************************************/
//#define uCOS_EN           				// RTOS Switch If use uC/OS-II unmask
//#define SPI_DMA                     	// DMA transfer SPI data
//#define DMA_INT                   // SPI DMA interrupt
//#define FifoPointCheck            // Enable DM9051 FIFO Pointer Check
//#define FORCE_10M                 // This define mask use Auto mode, else use force 10M mode
//#define DM9051_INT                // Enable DM9051 RX interrupt machine
//#define DM9051_Fiber              // Enable DM9051 Fiber Setting
//#define EEPROM_MAC_ADDR           // Enbale Use EEPROM MAC Address
//#define FLOW_CONTRORL							// Enable Flow control function

/* DM9051 MAC Address */
#define emacETHADDR0   0x00
#define emacETHADDR1   0x60
#define emacETHADDR2   0x6E
#define emacETHADDR3   0xB4
#define emacETHADDR4   0x7E
#define emacETHADDR5   0x34

/************************* MCU SPI & DMA Private define *************************************/
/* Configure DM9051 SPI channel */
#define DM9051_SPI                                      ((SPI_Type*)SPI1_BASE)
/* DM9051 SPI Chip Select Group and PIN*/
#define DM9051_SPI_CS_GROUP                             GPIOA
#define DM9051_SPI_CS_PIN                               GPIO_Pins_4
/* DM9051 SPI SCK & MISO & MOSI Group and PIN */
#define DM9051_SPI_PIN_GROUP                            GPIOA
#define DM9051_SPI_SCK_PIN                              GPIO_Pins_5
#define DM9051_SPI_MISO_PIN                             GPIO_Pins_6
#define DM9051_SPI_MOSI_PIN                             GPIO_Pins_7
/* DM9051 Interrupt  */
#define DM9051_RX_INT_GROUP                             GPIOC
#define DM9051_RX_INT_PIN                               GPIO_Pins_7
#define DM9051_RX_INT_PortSource                        GPIO_PortSourceGPIOC
#define DM9051_RX_INT_PINSource                         GPIO_PinSource7
#define DM9051_RX_INT_EXTI                              EXTI_Line7
#define DM9051_IRQHandler                               EXTI9_5_IRQHandler

#define DM9051_RX_Final 																GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)

/* DM9051 DMA Configure */
#ifdef SPI_DMA
/* Define DMA RX Channel */
#define DM9051_SPI_DMA_RX_Channel                       DMA1_Channel2
#define DM9051_SPI_DMA_RX_TC_FLAG                       DMA1_FLAG_TC2
#define DM9051_SPI_DMA_RX_GL_FLAG                       DMA1_FLAG_GL2
/* Define DMA TX Channel */
#define DM9051_SPI_DMA_TX_Channel                       DMA1_Channel3
#define DM9051_SPI_DMA_TX_TC_FLAG                       DMA1_FLAG_TC3 
#define DM9051_SPI_DMA_TX_GL_FLAG                       DMA1_FLAG_GL3 
/* Define SPI Data Register Address */
#define DM9051_SPI_DMA_DR_BaseAddr                      SPI1_BASE + 0xC  // DMA Channel + SPI_DR Address = DMA_Channel + 0x0c
#endif //SPI_DMA

/* Private typedef -----------------------------------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------------------------------------*/
#define MII_BMCR            1
#define MII_PHY_DSCR        16
#define MII_PHY_SPEC_CFG 		20

#define DM9051_PHY          (0x40)    			/* PHY address 0x01                                             */

/* Exported typedef ----------------------------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------------------------------------*/ 
#define DM9051_ID           (0x90510A46)		/* DM9051A ID                                                   */
#define DM9051_PKT_MAX      (1536)          /* Received packet max size                                     */
#define DM9051_PKT_RDY      (0x01)          /* Packet ready to receive                                      */

#define DM9051_NCR          (0x00)
#define DM9051_NSR          (0x01)
#define DM9051_TCR          (0x02)
#define DM9051_TSR1         (0x03)
#define DM9051_TSR2         (0x04)
#define DM9051_RCR          (0x05)
#define DM9051_RSR          (0x06)
#define DM9051_ROCR         (0x07)
#define DM9051_BPTR         (0x08)
#define DM9051_FCTR         (0x09)
#define DM9051_FCR          (0x0A)
#define DM9051_EPCR         (0x0B)
#define DM9051_EPAR         (0x0C)
#define DM9051_EPDRL        (0x0D)
#define DM9051_EPDRH        (0x0E)
#define DM9051_WCR          (0x0F)

#define DM9051_PAR          (0x10)
#define DM9051_MAR          (0x16)

#define DM9051_GPCR         (0x1e)
#define DM9051_GPR          (0x1f)
#define DM9051_TRPAL        (0x22)
#define DM9051_TRPAH        (0x23)
#define DM9051_RWPAL        (0x24)
#define DM9051_RWPAH        (0x25)

#define DM9051_VIDL         (0x28)
#define DM9051_VIDH         (0x29)
#define DM9051_PIDL         (0x2A)
#define DM9051_PIDH         (0x2B)

#define DM9051_CHIPR        (0x2C)
#define DM9051_TCR2         (0x2D)
#define DM9051_OTCR         (0x2E)
#define DM9051_SMCR         (0x2F)

#define DM9051_ETCR         (0x30)    /* early transmit control/status register                             */
#define DM9051_CSCR         (0x31)    /* check sum control register                                         */
#define DM9051_RCSSR        (0x32)    /* receive check sum status register                                  */

#define DM9051_PBCR					(0x38)
#define DM9051_INTR					(0x39)
#define DM9051_MPCR					(0x55)
#define DM9051_MRCMDX       (0x70)
#define DM9051_MRCMD        (0x72)
#define DM9051_MRRL         (0x74)
#define DM9051_MRRH         (0x75)
#define DM9051_MWCMDX       (0x76)
#define DM9051_MWCMD        (0x78)
#define DM9051_MWRL         (0x7A)
#define DM9051_MWRH         (0x7B)
#define DM9051_TXPLL        (0x7C)
#define DM9051_TXPLH        (0x7D)
#define DM9051_ISR          (0x7E)
#define DM9051_IMR          (0x7F)

#define CHIPR_DM9051A       (0x19)
#define CHIPR_DM9051B       (0x1B)

#define DM9051_REG_RESET    (0x01)
#define DM9051_IMR_OFF      (0x80)
#define DM9051_TCR2_SET     (0x90)	/* set one packet */
#define DM9051_RCR_SET      (0x31)
#define DM9051_BPTR_SET     (0x37)
#define DM9051_FCTR_SET     (0x38)
#define DM9051_FCR_SET      (0x28)
#define DM9051_TCR_SET      (0x01)


#define NCR_EXT_PHY         (1 << 7)
#define NCR_WAKEEN          (1 << 6)
#define NCR_FCOL            (1 << 4)
#define NCR_FDX             (1 << 3)
#define NCR_LBK             (3 << 1)
#define NCR_RST             (1 << 0)
#define NCR_DEFAULT					(0x0)    /* Disable Wakeup */

#define NSR_SPEED           (1 << 7)
#define NSR_LINKST          (1 << 6)
#define NSR_WAKEST          (1 << 5)
#define NSR_TX2END          (1 << 3)
#define NSR_TX1END          (1 << 2)
#define NSR_RXOV            (1 << 1)
#define NSR_CLR_STATUS			(NSR_WAKEST | NSR_TX2END | NSR_TX1END)

#define TCR_TJDIS           (1 << 6)
#define TCR_EXCECM          (1 << 5)
#define TCR_PAD_DIS2        (1 << 4)
#define TCR_CRC_DIS2        (1 << 3)
#define TCR_PAD_DIS1        (1 << 2)
#define TCR_CRC_DIS1        (1 << 1)
#define TCR_TXREQ           (1 << 0) /* Start TX */
#define TCR_DEFAULT					(0x0)

#define TSR_TJTO            (1 << 7)
#define TSR_LC              (1 << 6)
#define TSR_NC              (1 << 5)
#define TSR_LCOL            (1 << 4)
#define TSR_COL             (1 << 3)
#define TSR_EC              (1 << 2)

#define RCR_WTDIS           (1 << 6)
#define RCR_DIS_LONG        (1 << 5)
#define RCR_DIS_CRC         (1 << 4)
#define RCR_ALL             (1 << 3)
#define RCR_RUNT            (1 << 2)
#define RCR_PRMSC           (1 << 1)
#define RCR_RXEN            (1 << 0)
#define RCR_DEFAULT					(RCR_DIS_LONG | RCR_DIS_CRC)

#define RSR_RF              (1 << 7)
#define RSR_MF              (1 << 6)
#define RSR_LCS             (1 << 5)
#define RSR_RWTO            (1 << 4)
#define RSR_PLE             (1 << 3)
#define RSR_AE              (1 << 2)
#define RSR_CE              (1 << 1)
#define RSR_FOE             (1 << 0)

#define BPTR_DEFAULT				(0x3f)
#define FCTR_DEAFULT				(0x38)
#define FCR_DEFAULT					(0xFF)
#define SMCR_DEFAULT				(0x0)
#define PBCR_MAXDRIVE				(0x44)

//#define FCTR_HWOT(ot)       ((ot & 0xF ) << 4 )
//#define FCTR_LWOT(ot)       (ot & 0xF )

#define IMR_PAR             (1 << 7)
#define IMR_LNKCHGI         (1 << 5)
#define IMR_UDRUN						(1 << 4)
#define IMR_ROOM            (1 << 3)
#define IMR_ROM             (1 << 2)
#define IMR_PTM             (1 << 1)
#define IMR_PRM             (1 << 0)
#define IMR_FULL 						(IMR_PAR | IMR_LNKCHGI | IMR_UDRUN | IMR_ROOM | IMR_ROM | IMR_PTM | IMR_PRM)
#define IMR_OFF							(IMR_PAR)
#define IMR_DEFAULT					(IMR_PAR | IMR_PRM | IMR_PTM) 

#define ISR_ROOS            (1 << 3)
#define ISR_ROS             (1 << 2)
#define ISR_PTS             (1 << 1)
#define ISR_PRS             (1 << 0)
#define ISR_CLR_STATUS      (0x80 | 0x3F)

#define EPCR_REEP           (1 << 5)
#define EPCR_WEP            (1 << 4)
#define EPCR_EPOS           (1 << 3)
#define EPCR_ERPRR          (1 << 2)
#define EPCR_ERPRW          (1 << 1)
#define EPCR_ERRE           (1 << 0)

#define GPCR_GEP_CNTL       (1 << 0)

#define SPI_WR_BURST				(0xF8)
#define SPI_RD_BURST				(0x72)

#define SPI_READ						(0x03)
#define SPI_WRITE        		(0x04)
#define SPI_WRITE_BUFFER  	(0x05)		/* Send a series of bytes from the Master to the Slave */
#define SPI_READ_BUFFER   	(0x06)    /* Send a series of bytes from the Slave  to the Master */


/* Exported macro ------------------------------------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------------------------------------*/
extern uint8_t DM9051_RX_INT_TRIGGER;

/* Exported functions --------------------------------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------------------------------------*/
uint8_t DM9051_Read_Reg(uint8_t Reg_Off);
void DM9051_Write_Reg(uint8_t Reg_Off, uint8_t spi_data);

#ifdef LwIP_EN
int32_t DM9051_Init(struct netif *netif)
uint32_t DM9051_TX(struct netif *netif, struct pbuf* p);
struct pbuf *DM9051_RX(struct netif *netif);
#else
int32_t DM9051_Init(void);
uint32_t DM9051_TX(void);
uint16_t DM9051_RX(void);

void etherdev_init(void);
void etherdev_send(void);
uint16_t etherdev_read(void);
#endif	//LwIP_EN

#ifdef DM9051_INT
void DM9051_isr(void);
#endif //DM9051_INT

#endif /* __DM9051_REG_H -----------------------------------------------------------------------------------*/
