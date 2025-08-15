#include "derivative.h"   // MPC5744P 寄存器定義

//static inline void small_delay(volatile unsigned n){ while(n--); }

static void clocks_init(void) {
    /* 讓所有模組在 DRUN 可用，並切到 PLL0 做系統時鐘 */
    MC_ME.RUN_PC[0].B.DRUN = 1u;
    MC_ME.DRUN_MC.B.XOSCON = 1u;
    MC_CGM.AC3_SC.B.SELCTL = 1u;             // PLL0 來源 = XOSC
    PLLDIG.PLL0DV.R = 0x50000000 | 0x00020000 | 0x00002000 | 0x0014; // 200 MHz
    MC_ME.DRUN_MC.B.PLL0ON = 1u;

    /* PBRIDGEx 時鐘（SPI 所在匯流排）分頻到 ~50MHz */
    MC_CGM.SC_DC0.R = 0x80030000;

    /* 指派 DSPI_0 到 RUN_PC0，確保進入 DRUN 後有時鐘 */
//    MC_ME.PCTLn[MC_ME_PCTL_DSPI_0_INDEX] = 0u;   // -> RUN_PC[0]【:contentReference[oaicite:3]{index=3}】
    MC_ME.PCTL99.R = 0x0u;

    /* 進入 DRUN */
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

    /* 讓 MISO 腳（C[7]）內建上拉，沒有外部線路也能讀到 1 */
    SIUL2.MSCR[39].B.PUE = 1;   // enable pull
    SIUL2.MSCR[39].B.PUS = 1;   // select pull-up
}

static void spi0_init(void) {
    /* 先停住再設定，主模式、清空 FIFO */
    SPI_0.MCR.R = 0;
    SPI_0.MCR.B.MSTR    = 1;   // 主模式【:contentReference[oaicite:4]{index=4}】
    SPI_0.MCR.B.CLR_TXF = 1;
    SPI_0.MCR.B.CLR_RXF = 1;
    SPI_0.MCR.B.HALT    = 1;

    /* CTAR0：16 位資料、Mode0、適中時序/分頻（可依需要調整） */
    SPI_0.MODE.CTAR[0].R = 0x78021004;

    /* 解除 HALT 開始動作 */
    SPI_0.MCR.B.HALT = 0;
}

static uint16_t spi0_txrx_once(uint16_t tx)
{
    /* 等待可推送 */
    while (SPI_0.SR.B.TFFF == 0) { }
    /* PCS0 有效（位元 16），資料放低 16 位 */
    SPI_0.PUSHR.PUSHR.R = (1u << 16) | tx;   // 使用 PUSHR【:contentReference[oaicite:5]{index=5}】

    /* 等待收到一筆，再取出 */
    while (SPI_0.SR.B.RFDF == 0) { }
    uint16_t rx = (uint16_t)SPI_0.POPR.R;    // 讀 POPR 清 RFDF【:contentReference[oaicite:6]{index=6}】

    /* 清其他狀態旗標（寫 1 清除） */
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
//        sr_snapshot = SPI_0.SR.R;   // 方便在 Debug 視窗看旗標
//        small_delay(1000);
//    }
//}
