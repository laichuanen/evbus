/* main_integration.c — run ADC + SPI + CAN + PWM together (non-blocking) */

#include "MPC5744P.h"
#include <stdint.h>
#include "adc_smoke_test.c"
#include "spi_smoke_test.c"
#include "can_smoke_test.c"

/* 你的既有初始化（沿用你專案內的實作） */
extern void SysClk_Init(void);
extern void SUIL2_Init(void);

/* 來自各自 smoke 檔/驅動的初始化 API（你已經有） */
extern void adc0_init_one_shot(void);
extern void can0_loopback_init(void);          /* 內部 loopback, 不需實體總線 */
extern void pwm_smoke_init(void);
extern void pads_init_spi0_pullup_miso(void);  /* 無硬體時讓 MISO 有固定電位 */
extern void spi0_init_master(void);

/* 觀察用變數（Expressions） */
static volatile uint16_t sm_adc_last = 0;
static volatile uint16_t sm_spi_last = 0;
static volatile uint8_t  sm_can_tx = 0, sm_can_rx = 0;
static volatile uint32_t sm_can_iflag = 0;
static volatile uint16_t sm_pwm_duty = 300;
static volatile uint16_t sm_pwm_cnt  = 0;

static inline void tiny_delay(volatile uint32_t n){ while(n--){ __asm__("nop"); } }

/* --------- 非阻塞 step 函式們 --------- */

/* ADC：有完成旗標才讀，讀完清旗標並觸發下一次 */
static inline void adc_step_nonblock(uint8_t ch){
    const uint32_t ADC_EOC_MASK = (1u << 1);
    if (ADC_0.ISR.R & ADC_EOC_MASK) {
        uint32_t raw = ADC_0.CDR[ch].R;
        if (raw & (1u << 19)) sm_adc_last = (uint16_t)(raw & 0x0FFFu);
        ADC_0.ISR.R = ADC_EOC_MASK;     /* 清 EOC */
        ADC_0.MCR.B.NSTART = 1u;        /* 下一次 */
    }
}

/* SPI：TX FIFO 可填就送一筆；有資料就收一筆（MOSI↔MISO 未接時請用 MISO 內拉高） */
static inline void spi_step_nonblock(void){
    if (SPI_0.SR.B.TFFF) {
        SPI_0.PUSHR.PUSHR.R = (1u << 16) | 0xA5A5u;  /* PCS0 + TXDATA */
        /* 清 TFFF (bit 25) */
        SPI_0.SR.R = (1u << 25);
    }
    if (SPI_0.SR.B.RFDF) {
        sm_spi_last = (uint16_t)SPI_0.POPR.R;
        /* 清 RFDF (bit 17) */
        SPI_0.SR.R = (1u << 17);
    }
}

/* CAN：內部 loopback，偶爾送一筆；用 IFLAG 非阻塞處理 */
static inline void can_step_nonblock(uint32_t tick){
    const uint32_t MB0 = (1u<<0), MB1 = (1u<<1);
    /* 每隔一段時間送 1 byte */
    if ((tick & 0x3FFFu) == 0u) {
        sm_can_tx ^= 1u;
        CAN_0.MB[1].DATA.B[0] = sm_can_tx;
        CAN_0.MB[1].CS.R = 0x0C610000u;   /* DLC=1, IDE=1, SRR=1, CODE=0xC(TX) */
    }
    /* 清 TX 完成 */
    if (CAN_0.IFLAG1.R & MB1) { CAN_0.IFLAG1.R = MB1; }
    /* 處理 RX */
    if (CAN_0.IFLAG1.R & MB0) {
        (void)CAN_0.MB[0].CS.R; (void)CAN_0.MB[0].ID.R;
        sm_can_rx = CAN_0.MB[0].DATA.B[0];
        (void)CAN_0.TIMER.R;            /* 解鎖 */
        CAN_0.IFLAG1.R = MB0;
    }
    sm_can_iflag = CAN_0.IFLAG1.R;
}

/* PWM：週期性調 duty；只寫 LDOK 不 busy-wait（RUN 已在 init 中打開） */
static inline void pwm_step_nonblock(uint32_t tick){
    if ((tick % 50000u) == 0u) {
        sm_pwm_duty += 50u; if (sm_pwm_duty > 1200u) sm_pwm_duty = 300u;
        FlexPWM_0.SUB[1].VAL4.R = sm_pwm_duty;  /* 上升緣移動 */
        FlexPWM_0.SUB[1].VAL5.R = 1200u;        /* 下降緣固定 */
        FlexPWM_0.MCTRL.B.LDOK = (1u << 1);     /* 下一次 reload 套用 */
    }
    sm_pwm_cnt = (uint16_t)FlexPWM_0.SUB[1].CNT.R;
}

/* -------------------- main：同時跑四個周邊 -------------------- */
//int main(void){
//    SysClk_Init();
//
//    /* 腳位多半由你原本的 SUIL2_Init 來設；SPI/PWM 要用到 */
//    SUIL2_Init();
//
//    /* 個別初始化 */
//    adc0_init_one_shot();
//    pads_init_spi0_pullup_miso();   /* 無硬體 SPI 建議開這個 */
//    spi0_init();
//    can0_loopback_init();           /* 內部 loopback，不需要外部總線 */
//    pwm_smoke_init();               /* 會啟動 SM1 RUN */
//
//    /* 先啟動一次 ADC 轉換 */
//    ADC_0.MCR.B.NSTART = 1u;
//
//    uint32_t tick = 0;
//    for(;;){
//        ++tick;
//        adc_step_nonblock(0u);     /* 你要的 ADC channel */
//        spi_step_nonblock();
//        can_step_nonblock(tick);
//        pwm_step_nonblock(tick);
//        tiny_delay(2000);
//    }
//}
