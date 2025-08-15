#include "derivative.h"   // MPC5744P �H�s���w�q

//static inline void small_delay(volatile unsigned n){ while(n--); }

static void clocks_init(void) {
    /* ���Ҧ��Ҳզb DRUN �i�ΡA�ä��� PLL0 ���t�ή��� */
    MC_ME.RUN_PC[0].B.DRUN = 1u;
    MC_ME.DRUN_MC.B.XOSCON = 1u;
    MC_CGM.AC3_SC.B.SELCTL = 1u;             // PLL0 �ӷ� = XOSC
    PLLDIG.PLL0DV.R = 0x50000000 | 0x00020000 | 0x00002000 | 0x0014; // 200 MHz
    MC_ME.DRUN_MC.B.PLL0ON = 1u;

    /* PBRIDGEx �����]SPI �Ҧb�׬y�ơ^���W�� ~50MHz */
    MC_CGM.SC_DC0.R = 0x80030000;

    /* ���� DSPI_0 �� RUN_PC0�A�T�O�i�J DRUN �ᦳ���� */
//    MC_ME.PCTLn[MC_ME_PCTL_DSPI_0_INDEX] = 0u;   // -> RUN_PC[0]�i:contentReference[oaicite:3]{index=3}�j
    MC_ME.PCTL99.R = 0x0u;

    /* �i�J DRUN */
    MC_ME.MCTL.R = 0x30005AF0;
    MC_ME.MCTL.R = 0x3000A50F;
    while(!MC_ME.GS.B.S_PLL0) { }
    while(MC_ME.GS.B.S_MTRANS) { }
    while(MC_ME.GS.B.S_CURRENT_MODE != 0x3) { }  // DRUN
}

static void pads_init_spi0_pullup_miso(void) {
    /* C[4]=PCS0, C[5]=SCK, C[6]=SOUT, C[7]=SIN */
    SIUL2.MSCR[36].B.SSS = 1; SIUL2.MSCR[36].B.OBE = 1; SIUL2.MSCR[36].B.SRC = 3;
    SIUL2.MSCR[37].B.SSS = 1; SIUL2.MSCR[37].B.OBE = 1; SIUL2.MSCR[37].B.SRC = 3;
    SIUL2.MSCR[38].B.SSS = 1; SIUL2.MSCR[38].B.OBE = 1; SIUL2.MSCR[38].B.SRC = 3;
    SIUL2.MSCR[39].B.IBE = 1; SIUL2.IMCR[41].B.SSS = 1;

    /* �� MISO �}�]C[7]�^���ؤW�ԡA�S���~���u���]��Ū�� 1 */
    SIUL2.MSCR[39].B.PUE = 1;   // enable pull
    SIUL2.MSCR[39].B.PUS = 1;   // select pull-up
}

static void spi0_init(void) {
    /* ������A�]�w�A�D�Ҧ��B�M�� FIFO */
    SPI_0.MCR.R = 0;
    SPI_0.MCR.B.MSTR    = 1;   // �D�Ҧ��i:contentReference[oaicite:4]{index=4}�j
    SPI_0.MCR.B.CLR_TXF = 1;
    SPI_0.MCR.B.CLR_RXF = 1;
    SPI_0.MCR.B.HALT    = 1;

    /* CTAR0�G16 ���ơBMode0�B�A���ɧ�/���W�]�i�̻ݭn�վ�^ */
    SPI_0.MODE.CTAR[0].R = 0x78021004;

    /* �Ѱ� HALT �}�l�ʧ@ */
    SPI_0.MCR.B.HALT = 0;
}

static uint16_t spi0_txrx_once(uint16_t tx)
{
    /* ���ݥi���e */
    while (SPI_0.SR.B.TFFF == 0) { }
    /* PCS0 ���ġ]�줸 16�^�A��Ʃ�C 16 �� */
    SPI_0.PUSHR.PUSHR.R = (1u << 16) | tx;   // �ϥ� PUSHR�i:contentReference[oaicite:5]{index=5}�j

    /* ���ݦ���@���A�A���X */
    while (SPI_0.SR.B.RFDF == 0) { }
    uint16_t rx = (uint16_t)SPI_0.POPR.R;    // Ū POPR �M RFDF�i:contentReference[oaicite:6]{index=6}�j

    /* �M��L���A�X�С]�g 1 �M���^ */
    SPI_0.SR.R = 0xFCFE0000;

    return rx;
}

//int main(void)
//{
//    volatile uint16_t last = 0;
//    volatile uint32_t sr_snapshot = 0;
//
//    clocks_init();
//    pads_init_spi0_pullup_miso();
//    spi0_init();
//
//    for(;;) {
//        last = spi0_txrx_once(0xA5A5);
//        sr_snapshot = SPI_0.SR.R;   // ��K�b Debug �����ݺX��
//        small_delay(1000);
//    }
//}
