#ifndef BBBIO_MSCPI_H
#define BBBIO_MSCPI_H
//----------------------------------------------------------------------------
#define BBBIO_McSPI_SPI0	0
#define BBBIO_McSPI_SPI1	1
#define SPI0 0
#define SPI1 1

#define BBBIO_McSPI_CH0		0
#define BBBIO_McSPI_CH1		1
#define BBBIO_McSPI_CH2		2	/* CH2 & CH3 not pin out , useless */
#define BBBIO_McSPI_CH3		3

#define SPI_CH0 0
#define SPI_CH1 1
#define SPI_CH2 2
#define SPI_CH3 3

/* Rerence AM335x Techinal Reference Manual .
 *	Session 24.3.1.1 , 24.3.1.2.1 , 24.3.1.2.2
 */
#define BBBIO_McSPI_Master	0x0
#define BBBIO_McSPI_Slave	0x1

#define SPI_MASTER 0x0
#define SPI_SLAVE 0x1

#define BBBIO_McSPI_TxRx	0x0
#define BBBIO_McSPI_Rx_Only	0x1
#define BBBIO_McSPI_Tx_Only	0x2

#define SPI_TXRX 0x0
#define SPI_RXTX 0x0
#define SPI_RX 0x1
#define SPI_TX 0x2

/* Reference AM335x Techinal Reference Manual .
 * 	Session 24.3.1.3.6 Polarity and Phase
 */
#define BBBIO_McSPI_CLK_MODE0	0x0
#define BBBIO_McSPI_CLK_MODE1	0x1
#define BBBIO_McSPI_CLK_MODE2	0x2
#define BBBIO_McSPI_CLK_MODE3	0x3

#define SPI_CLOCKMODE0 0x0
#define SPI_CLOCKMODE1 0x1
#define SPI_CLOCKMODE2 0x2
#define SPI_CLOCKMODE3 0x3

/* Reference AM335x Techinal Reference Manual .
 *	page :4594 , bit 16~18 , Field : IS , DPE1 , DPE0
 */
#define BBBIO_McSPI_D0O_D1I	0x6
#define BBBIO_McSPI_D0I_D1O	0x1

#define SPI_OUTIN 0x6
#define SPI_INOUT 0x1

#define BBBIO_McSPI_EN_ACT_HIGH	0x0
#define BBBIO_McSPI_EN_ACT_LOW	0x1

#define SPI_CE_ACT_HIGH 0x0
#define SPI_CE_ACT_LOW 0x1

#define SPI_DIV1 0x0
#define SPI_DIV2 0x1
#define SPI_DIV4 0x2
#define SPI_DIV8 0x3
#define SPI_DIV16 0x4
#define SPI_DIV32 0x5
#define SPI_DIV64 0x6
#define SPI_DIV128 0x7
#define SPI_DIV256 0x8
#define SPI_DIV512 0x9
#define SPI_DIV1K 0xA
#define SPI_DIV2K 0xB
#define SPI_DIV4K 0xC
#define SPI_DIV8K 0xD
#define SPI_DIV16K 0xE
#define SPI_DIV32K 0xF

#define BBBIO_McSPI_CLDIV_1	0x0
#define BBBIO_McSPI_CLDIV_2	0x1
#define BBBIO_McSPI_CLDIV_4	0x2
#define BBBIO_McSPI_CLDIV_8	0x3
#define BBBIO_McSPI_CLDIV_16	0x4
#define BBBIO_McSPI_CLDIV_32	0x5
#define BBBIO_McSPI_CLDIV_64	0x6
#define BBBIO_McSPI_CLDIV_128	0x7
#define BBBIO_McSPI_CLDIV_256	0x8
#define BBBIO_McSPI_CLDIV_512	0x9
#define BBBIO_McSPI_CLDIV_1024	0xA
#define BBBIO_McSPI_CLDIV_2048	0xB
#define BBBIO_McSPI_CLDIV_4096	0xC
#define BBBIO_McSPI_CLDIV_8192	0xD
#define BBBIO_McSPI_CLDIV_16384	0xE
#define BBBIO_McSPI_CLDIV_32768	0xF

//-------------------------------------------------------------------------

void configure_spi_pins(char cs_pin, char d1_pin, char d0_pin, char clk_pin);
void configure_spi_pins_p1(void);
void configure_spi_pins_p2(void);

int spi_transact(unsigned int McSPI_ID, unsigned int chn, unsigned int Tx_data, unsigned int *Rx_data);

int BBBIO_McSPI_Init() ;

#define BBBIO_McSPI_Enable(a) BBBIO_McSPI_CLK_set(a,1,0)
#define BBBIO_McSPI_Disable(a) BBBIO_McSPI_CLK_set(a,0,0)
#define spi_enable(a) BBBIO_McSPI_CLK_set(a,1,0)
#define spi_disable(a) BBBIO_McSPI_CLK_set(a,0,0)
#define spi_enable_pb(a) BBBIO_McSPI_CLK_set(a,1,1)
#define spi_disable_pb(a) BBBIO_McSPI_CLK_set(a,0,1)
int BBBIO_McSPI_CLK_set(unsigned int McSPI_ID ,int enable , int idle) ;

int spi_ctrl(unsigned int McSPI_ID ,
                        unsigned int channel,
                        unsigned int MS,		/* MS		 , maset or slave */
                        unsigned int TRM ,		/* TRM		 , Tx only / Rx only , TxRx */
                        unsigned int CLK_div ,		/* Clock Divider , default clock : 48M Hz */
                        unsigned int CLKmode ,		/* POL/PHA ,     , clock polarity */
                        unsigned int EPOL ,
                        unsigned int DataDir ,		/* IS/DPE1/DPE0	 ,data0 Outpu    t data1 Input ,or data0 Input data1 Output */
                        unsigned int WL);		// WL		 ,word length */

void BBBIO_McSPI_Reset(unsigned int McSPI_ID);

//-------------------------------------------------------------------------
#endif
