//////////////////////////////////////////////////////////////////////////////
// * File name: ezdsp5535_spi.c
//////////////////////////////////////////////////////////////////////////////

#include "ezdsp5535_spi.h"
#include "csl_spi.h"
#include "Si446x_api.h"
/*
 *  EZDSP5535_SPI_init( )
 *
 *      Enables and configures SPI for the SPIFLASH
 *      ( CS0, EBSR Mode 1, 8-bit, 100KHz clock )
 */
void EZDSP5535_SPI_init(void)
{
    /* Configuration for SPI
    SPI_Config      hwConfig;

    hwConfig.wLen       = SPI_WORD_LENGTH_8;    // 8-bit
    hwConfig.spiClkDiv  = 0x0041;               // 100KHz clock (12MHz / 120)
    hwConfig.csNum      = SPI_CS_NUM_2;         // Select CS2
    hwConfig.frLen      = 1;
    hwConfig.dataDelay  = SPI_DATA_DLY_0;
    hwConfig.clkPol     = SPI_CLKP_LOW_AT_IDLE;
*/
	/* Disable the serial Data clock */
	CSL_FSET(CSL_SPI_REGS->SPICCR, CSL_SPI_SPICCR_CLKEN_SHIFT, 
		CSL_SPI_SPICCR_CLKEN_SHIFT, CSL_SPI_SPICCR_CLKEN_DISABLED);
	//CSL_FINST(CSL_SPI_REGS->SPICCR, SPI_SPICCR_CLKEN, DISABLED);
	CSL_FINST(CSL_SPI_REGS->SPICCR, SPI_SPICCR_RST, RELEASE);

	CSL_FINS(CSL_SPI_REGS->SPICDR, SPI_SPICDR_CLKDV, 0x0011);	// 100KHz clock (12MHz / 120)

	EZDSP5535_waitusec(1);
	/* Enable the serial Data clock */
	CSL_SPI_REGS->SPICCR =
	      (Uint16)(CSL_SPI_SPICCR_CLKEN_ENABLED << CSL_SPI_SPICCR_CLKEN_SHIFT);

	/* Set Data delay, cs pol, clk pol and clpck pkase bit as per chip select */
	CSL_FINS(CSL_SPI_REGS->SPIDCR2, SPI_SPIDCR2_DD2, SPI_DATA_DLY_0);
	CSL_FINS(CSL_SPI_REGS->SPIDCR2, SPI_SPIDCR2_CSP2, 0);
	CSL_FINS(CSL_SPI_REGS->SPIDCR2, SPI_SPIDCR2_CKP2, SPI_CLKP_LOW_AT_IDLE);
	CSL_FINS(CSL_SPI_REGS->SPIDCR2, SPI_SPIDCR2_CKPH2, 0);

	CSL_CPU_REGS->IER1 |= 0x0008;		// SPI interrupt
}

Uint8 SpiReadByte(Uint8 reg)
{
	Uint16 	spiStatusReg;
	volatile Uint16 	spiBusyStatus;
	volatile Uint16 	spiWcStaus;
	Uint8 temp8;

	CSL_SPI_REGS->SPICMD1 = 0x0000 | 0;
	CSL_SPI_REGS->SPIDR2 = (Uint16)(reg << 8);
	CSL_SPI_REGS->SPICMD2 = (2 << 12)|(15 << 3)|SPI_READ_CMD;

	while(!(CSL_SPI_REGS->SPISTAT1 & CSL_SPI_SPISTAT1_BSY_MASK));
	do {
		spiStatusReg = CSL_SPI_REGS->SPISTAT1;
		spiBusyStatus = (spiStatusReg & CSL_SPI_SPISTAT1_BSY_MASK);
		spiWcStaus = (spiStatusReg & CSL_SPI_SPISTAT1_CC_MASK);
	}while(spiBusyStatus && (!spiWcStaus));

	temp8 = (CSL_SPI_REGS->SPIDR1 & 0xFF);
	return temp8;
}

void SpiWriteByte(Uint8 reg, Uint8 val)
{
	Uint16 	spiStatusReg;
	volatile Uint16 	spiBusyStatus;
	volatile Uint16 	spiWcStaus;

    CSL_SPI_REGS->SPICMD1 = 0x0000 | 0;

	CSL_SPI_REGS->SPIDR2 = ((Uint16)reg << 8) | val;
	CSL_SPI_REGS->SPICMD2 = (2 << 12)|(15 << 3)|SPI_WRITE_CMD;

	while(!(CSL_SPI_REGS->SPISTAT1 & CSL_SPI_SPISTAT1_BSY_MASK));
	do {
		spiStatusReg = CSL_SPI_REGS->SPISTAT1;
		spiBusyStatus = (spiStatusReg & CSL_SPI_SPISTAT1_BSY_MASK);
		spiWcStaus = (spiStatusReg & CSL_SPI_SPISTAT1_CC_MASK);
	}while(spiBusyStatus && (!spiWcStaus));
	EZDSP5535_waitusec(100);
}

void SpiSendCommand(int length, Uint8 *str)
{
	int i;
	Uint16 	spiStatusReg;
	volatile Uint16 	spiBusyStatus;
	volatile Uint16 	spiWcStaus;

    CSL_SPI_REGS->SPICMD1 = 0x0000 | (length - 1);		// bytes

	for(i = 0; i < length; i++)
	{
		CSL_SPI_REGS->SPIDR2 = (Uint16)(str[i] << 8);
		CSL_SPI_REGS->SPICMD2 = (2 << 12)|(7 << 3)|SPI_WRITE_CMD;

		while(!(CSL_SPI_REGS->SPISTAT1 & CSL_SPI_SPISTAT1_BSY_MASK));
		do {
			spiStatusReg = CSL_SPI_REGS->SPISTAT1;
			spiBusyStatus = (spiStatusReg & CSL_SPI_SPISTAT1_BSY_MASK);
			spiWcStaus = (spiStatusReg & CSL_SPI_SPISTAT1_CC_MASK);
		}while(spiBusyStatus && (!spiWcStaus));

		EZDSP5535_waitusec(100);
	}
}

void SpiGetResponse(Uint8 reg, int length, Uint8 *str)
{
	int i;
	Uint16 	spiStatusReg;
	volatile Uint16 	spiBusyStatus;
	volatile Uint16 	spiWcStaus;

	CSL_SPI_REGS->SPICMD1 = 0x0000 | length;
	CSL_SPI_REGS->SPIDR2 = (Uint16)reg << 8;
	CSL_SPI_REGS->SPICMD2 = (2 << 12)|(7 << 3)|SPI_WRITE_CMD;

	while(!(CSL_SPI_REGS->SPISTAT1 & CSL_SPI_SPISTAT1_BSY_MASK));
	do {
		spiStatusReg = CSL_SPI_REGS->SPISTAT1;
		spiBusyStatus = (spiStatusReg & CSL_SPI_SPISTAT1_BSY_MASK);
		spiWcStaus = (spiStatusReg & CSL_SPI_SPISTAT1_CC_MASK);
	}while(spiBusyStatus && (!spiWcStaus));

	EZDSP5535_waitusec(100);
	for(i = 0; i < length; i++)
	{
		CSL_SPI_REGS->SPICMD2 = (2 << 12)|(7 << 3)|SPI_READ_CMD;
		while(!(CSL_SPI_REGS->SPISTAT1 & CSL_SPI_SPISTAT1_BSY_MASK));
		do {
			spiStatusReg = CSL_SPI_REGS->SPISTAT1;
			spiBusyStatus = (spiStatusReg & CSL_SPI_SPISTAT1_BSY_MASK);
			spiWcStaus = (spiStatusReg & CSL_SPI_SPISTAT1_CC_MASK);
		}while(spiBusyStatus && (!spiWcStaus));
		str[i] = (CSL_SPI_REGS->SPIDR1 & 0xFF);
	}
}

int SpiWaitforCTS(void)
{
	Uint8 ctsValue = 0;
	Uint16 errCnt = 0;

	while (ctsValue != 0xFF) {		// Wait until radioIC is ready
		ctsValue = SpiReadByte(CMD_READ_CMD_BUFF);
		if (++errCnt > MAX_CTS_RETRY)
			return -1;     // Error handling; CTS read exceeds a limit
	}
	return 0;
}

void SpiWriteTxDataBuffer(int length, Uint8 *str)
{
	int i;
	Uint16 	spiStatusReg;
	volatile Uint16 	spiBusyStatus;
	volatile Uint16 	spiWcStaus;

    CSL_SPI_REGS->SPICMD1 = 0x0000 | length;
	CSL_SPI_REGS->SPIDR2 = CMD_WRITE_TX_FIFO << 8;
	CSL_SPI_REGS->SPICMD2 = (2 << 12)|(7 << 3)|SPI_WRITE_CMD;

	while(!(CSL_SPI_REGS->SPISTAT1 & CSL_SPI_SPISTAT1_BSY_MASK));
	do {
		spiStatusReg = CSL_SPI_REGS->SPISTAT1;
		spiBusyStatus = (spiStatusReg & CSL_SPI_SPISTAT1_BSY_MASK);
		spiWcStaus = (spiStatusReg & CSL_SPI_SPISTAT1_CC_MASK);
	}while(spiBusyStatus && (!spiWcStaus));

	EZDSP5535_waitusec(100);

	for(i = 0; i < length; i++)
	{
		CSL_SPI_REGS->SPIDR2 = (Uint16)str[i] << 8;
		CSL_SPI_REGS->SPICMD2 = (2 << 12)|(7 << 3)|SPI_WRITE_CMD;

		while(!(CSL_SPI_REGS->SPISTAT1 & CSL_SPI_SPISTAT1_BSY_MASK));
		do {
			spiStatusReg = CSL_SPI_REGS->SPISTAT1;
			spiBusyStatus = (spiStatusReg & CSL_SPI_SPISTAT1_BSY_MASK);
			spiWcStaus = (spiStatusReg & CSL_SPI_SPISTAT1_CC_MASK);
		}while(spiBusyStatus && (!spiWcStaus));

		EZDSP5535_waitusec(100);
	}
}

void SpiReadRxDataBuffer(int length, Uint8 *str)
{
	SpiGetResponse(CMD_READ_RX_FIFO, length, str);
}

Uint8 spibuffer[128];
Uint8 *recv_buffer = spibuffer;
Int16 recv_cnt;

extern Int16 spiIsrStatus;
interrupt void spi_isr(void)
{
	if(--recv_cnt > 0) {
		*recv_buffer++ = CSL_SPI_REGS->SPIDR1 & 0xff;
		CSL_SPI_REGS->SPIDR2 = CMD_READ_RX_FIFO << 8;
		CSL_SPI_REGS->SPICMD2 = (2 << 12)|(15 << 3)|SPI_READ_CMD;
	} else {
		SpiWriteByte(CMD_FIFO_INFO, 0);
		SpiWriteByte(CMD_GET_INT_STATUS, 0);
//		CSL_SPI_REGS->SPICMD1 = 0x0000 | 0;
		spiIsrStatus = 1;
	}
}

Uint16 SPI_recvData(Uint8 *buf)
{
	Uint8 info[4];
	recv_buffer = buf;
	SpiWriteByte(CMD_FIFO_INFO, 0);
	SpiGetResponse(CMD_READ_CMD_BUFF, 3, info);
	recv_cnt = info[1];
	CSL_SPI_REGS->SPICMD1 = 0x4000 | 0;
	CSL_SPI_REGS->SPIDR2 = CMD_READ_RX_FIFO << 8;

	CSL_SPI_REGS->SPICMD2 = (2 << 12)|(15 << 3)|SPI_READ_CMD;	// read first
	return recv_cnt;
}

