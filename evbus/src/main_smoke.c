/* main_smoke.c — Selectable smoke-test main for MPC5744P
   Define exactly ONE of:
     -DUSE_ADC_SMOKE
     -DUSE_SPI_SMOKE
     -DUSE_CAN_SMOKE
     -DUSE_PWM_SMOKE
*/

#include "MPC5744P.h"
#include <stdint.h>

#define USE_PWM_SMOKE

extern void SysClk_Init(void);
extern void SUIL2_Init(void);   /* 只有 SPI / PWM 需要腳位複用 */

// ---- guards: 必須恰好選一個 ----
#if !defined(USE_ADC_SMOKE) && !defined(USE_SPI_SMOKE) && \
    !defined(USE_CAN_SMOKE) && !defined(USE_PWM_SMOKE)
#error "請在編譯選項定義其中一個：USE_ADC_SMOKE / USE_SPI_SMOKE / USE_CAN_SMOKE / USE_PWM_SMOKE"
#endif
#if (defined(USE_ADC_SMOKE) && (defined(USE_SPI_SMOKE)||defined(USE_CAN_SMOKE)||defined(USE_PWM_SMOKE))) || \
    (defined(USE_SPI_SMOKE) && (defined(USE_CAN_SMOKE)||defined(USE_PWM_SMOKE))) || \
    (defined(USE_CAN_SMOKE) && defined(USE_PWM_SMOKE))
#error "一次只能啟用一種 smoke test"
#endif

static inline void tiny_delay(volatile uint32_t n){ while(n--){ __asm__("nop"); } }

/* ---------- ADC SMOKE ---------- */
#ifdef USE_ADC_SMOKE
#ifndef ADC_SMOKE_CH
#define ADC_SMOKE_CH  (0u)     /* 想測的實體通道 */
#endif
extern void     adc0_init_one_shot(void);           /* 來自 adc_smoke_test.c */
extern uint16_t adc0_read_once(uint8_t ch);

volatile uint16_t sm_adc_last = 0;

int main(void){
    SysClk_Init();
    adc0_init_one_shot();
    for(;;){
        sm_adc_last = adc0_read_once(ADC_SMOKE_CH);
        ADC_0.MCR.B.NSTART = 1u;        /* 觸發下一次 */
        tiny_delay(5000);
    }
}
#endif /* USE_ADC_SMOKE */

/* ---------- SPI SMOKE ---------- */
#ifdef USE_SPI_SMOKE
/* 建議你已經有 pads_init_spi0_pullup_miso() 讓 MISO 內拉高，無外接硬體也能讀到穩定值 */
extern void pads_init_spi0_pullup_miso(void);
extern void spi0_init_master(void);
extern uint16_t spi0_txrx_once(uint16_t tx);        /* 你現有 SPI.c 的 API */

volatile uint16_t sm_spi_last = 0;
volatile uint32_t sm_spi_sr   = 0;

int main(void){
    SysClk_Init();
    SUIL2_Init();                 /* SOUT/SIN/SCK/PCS0 複用 */
    pads_init_spi0_pullup_miso(); /* 無硬體自測 */
    spi0_init_master();

    for(;;){
        sm_spi_last = spi0_txrx_once(0xA5A5);
        sm_spi_sr   = SPI_0.SR.R;          /* 在 Registers/Expressions 觀察旗標 */
        tiny_delay(20000);
    }
}
#endif /* USE_SPI_SMOKE */

/* ---------- CAN SMOKE (internal loopback) ---------- */
#ifdef USE_CAN_SMOKE
extern void can0_loopback_init(void);       /* 來自 can_smoke_test.c */
extern void can0_loopback_smoke_once(void); /* 送1 byte 並自收 */

int main(void){
    SysClk_Init();
    can0_loopback_init();
    for(;;){
        can0_loopback_smoke_once();
        tiny_delay(20000);
    }
}
#endif /* USE_CAN_SMOKE */

/* ---------- PWM SMOKE ---------- */
#ifdef USE_PWM_SMOKE
extern void pwm_smoke_init(void);  /* 來自 pwm_smoke_test.c */
extern void pwm_smoke_step(void);

//int main(void){
//    SysClk_Init();
//    SUIL2_Init();      /* C[4]/C[6]/C[7] → FlexPWM0 X/A/B (OBE=1, SSS=2) */
//    pwm_smoke_init();
//    for(;;){
//        pwm_smoke_step();          /* 週期性改變占空比，LDOK 自清可見 */
//        tiny_delay(80000);
//    }
//}
#endif /* USE_PWM_SMOKE */
