#ifndef DM9051_REGS_H
#define DM9051_REGS_H

/* Staging subset of DM9051 register and bit definitions.
 * Current source of truth:
 *   drivers/dm9051_edriver_v1.6.1a_beta/core/dm9051_internal.h */

#define DM9051_NCR                0x00u
#define DM9051_NSR                0x01u
#define DM9051_TCR                0x02u
#define DM9051_TSR1               0x03u
#define DM9051_TSR2               0x04u
#define DM9051_RCR                0x05u
#define DM9051_RSR                0x06u
#define DM9051_ROCR               0x07u
#define DM9051_BPTR               0x08u
#define DM9051_FCTR               0x09u
#define DM9051_FCR                0x0Au
#define DM9051_EPCR               0x0Bu
#define DM9051_EPAR               0x0Cu
#define DM9051_EPDRL              0x0Du
#define DM9051_EPDRH              0x0Eu
#define DM9051_WCR                0x0Fu
#define DM9051_PAR                0x10u
#define DM9051_MAR                0x16u
#define DM9051_GPCR               0x1Eu
#define DM9051_GPR                0x1Fu
#define DM9051_TRPAL              0x22u
#define DM9051_TRPAH              0x23u
#define DM9051_RWPAL              0x24u
#define DM9051_RWPAH              0x25u
#define DM9051_VIDL               0x28u
#define DM9051_VIDH               0x29u
#define DM9051_PIDL               0x2Au
#define DM9051_PIDH               0x2Bu
#define DM9051_CHIPR              0x2Cu
#define DM9051_TCR2               0x2Du
#define DM9051_OTCR               0x2Eu
#define DM9051_SMCR               0x2Fu
#define DM9051_ATCR               0x30u
#define DM9051_CSCR               0x31u
#define DM9051_RCSSR              0x32u
#define DM9051_PBCR               0x38u
#define DM9051_INTR               0x39u
#define DM9051_TXFSSR             0x3Bu
#define DM9051_PPCR               0x3Du
#define DM9051_IPCOCR             0x54u
#define DM9051_MPCR               0x55u
#define DM9051_LMCR               0x57u
#define DM9051_MBNDRY             0x5Eu
#define DM9051_MRCMDX             0x70u
#define DM9051_MRCMD              0x72u
#define DM9051_MRRL               0x74u
#define DM9051_MRRH               0x75u
#define DM9051_MWCMDX             0x76u
#define DM9051_MWCMD              0x78u
#define DM9051_MWRL               0x7Au
#define DM9051_MWRH               0x7Bu
#define DM9051_TXPLL              0x7Cu
#define DM9051_TXPLH              0x7Du
#define DM9051_ISR                0x7Eu
#define DM9051_IMR                0x7Fu

#define DM9051_VENDOR_ID          0x0A46u
#define DM9051_PRODUCT_ID         0x9051u

#define DM9051_PHY                0x40u
#define DM9051_PKT_RDY            0x01u

#define DM9051_CHIPR_A            0x19u
#define DM9051_CHIPR_B            0x1Bu

#define DM9051_NCR_RESET          0x01u
#define DM9051_IMR_OFF            0x80u
#define DM9051_TCR2_SET           0x90u
#define DM9051_RCR_SET            0x31u
#define DM9051_BPTR_SET           0x37u
#define DM9051_FCTR_SET           0x38u
#define DM9051_FCR_SET            0x28u

#define DM9051_OPC_REG_R          0x00u
#define DM9051_OPC_REG_W          0x80u

#define DM9051_TCR_TXREQ          (1u << 0)
#define DM9051_NSR_LINKST         (1u << 6)
#define DM9051_EPCR_BUSY          (1u << 0)
#define DM9051_EPCR_PHY_READ      0x0Cu
#define DM9051_EPCR_PHY_WRITE     0x0Au
#define DM9051_INTR_ACTIVE_LOW    (1u << 0)
#define DM9051_FCR_TXPEN          (1u << 5)
#define DM9051_FCR_BKPA           (1u << 4)
#define DM9051_FCR_BKPM           (1u << 3)
#define DM9051_FCR_FLCE           (1u << 0)
#define DM9051_FCR_DEFAULT        (DM9051_FCR_TXPEN | DM9051_FCR_BKPA | \
                                   DM9051_FCR_BKPM | DM9051_FCR_FLCE)
#define DM9051_TCSCR_UDPCS_ENABLE (1u << 2)
#define DM9051_TCSCR_TCPCS_ENABLE (1u << 1)
#define DM9051_TCSCR_IPCS_ENABLE  (1u << 0)
#define DM9051_TCSCR_ALL_ENABLE   (DM9051_TCSCR_IPCS_ENABLE | \
                                   DM9051_TCSCR_UDPCS_ENABLE | \
                                   DM9051_TCSCR_TCPCS_ENABLE)
#define DM9051_RCSSR_RCSEN        (1u << 1)
#define DM9051_RCSSR_DCSE         (1u << 0)
#define DM9051_RCSSR_RX_ENABLE    (DM9051_RCSSR_RCSEN | DM9051_RCSSR_DCSE)
#define DM9051_PPCR_PAUSE_COUNT   0x0Fu
#define DM9051_IPCOCR_CLKOUT      (1u << 7)
#define DM9051_IPCOCR_DUTY_LEN    1u
#define DM9051_LMCR_NEWMOD        (1u << 7)
#define DM9051_LMCR_TYPED0        (1u << 0)
#define DM9051_LMCR_MODE1         (DM9051_LMCR_NEWMOD | DM9051_LMCR_TYPED0)
#define DM9051_MBNDRY_BYTE        (1u << 7)
#define DM9051_BOUND_CONF_BIT     DM9051_MBNDRY_BYTE
#define DM9051_RCR_DIS_LONG       (1u << 5)
#define DM9051_RCR_DIS_CRC        (1u << 4)
#define DM9051_RCR_ALL            (1u << 3)
#define DM9051_RCR_PRMSC          (1u << 1)
#define DM9051_RCR_RXEN           (1u << 0)
#define DM9051_RCR_DEFAULT        (DM9051_RCR_DIS_LONG | DM9051_RCR_DIS_CRC)
#define DM9051_RSR_RF             (1u << 7)
#define DM9051_RSR_LCS            (1u << 5)
#define DM9051_RSR_RWTO           (1u << 4)
#define DM9051_RSR_AE             (1u << 2)
#define DM9051_RSR_CE             (1u << 1)
#define DM9051_RSR_FOE            (1u << 0)
#define DM9051_RSR_ERR_BITS       (DM9051_RSR_RF | DM9051_RSR_LCS | \
                                   DM9051_RSR_RWTO | DM9051_RSR_AE | \
                                   DM9051_RSR_CE | DM9051_RSR_FOE)
#define DM9051_ISR_PR             (1u << 0)
#define DM9051_ISR_CLEAR_RX       (1u << 7)
#define DM9051_IMR_PAR            (1u << 7)
#define DM9051_IMR_PRM            (1u << 0)
#define DM9051_IMR_INT_DEFAULT    (DM9051_IMR_PAR | DM9051_IMR_PRM)
#define DM9051_IMR_POL_DEFAULT    DM9051_IMR_PAR

#define DM9051_PHY_ADV_REG        0x04u
#define DM9051_PHY_ADV_FLOW_CTRL  0x05E1u

#endif /* DM9051_REGS_H */
