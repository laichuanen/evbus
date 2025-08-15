/* main_smoke.c �X Selectable smoke-test main for MPC5744P
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
extern void SUIL2_Init(void);   /* �u�� SPI / PWM �ݭn�}��ƥ� */

// ---- guards: ������n��@�� ----
#if !defined(USE_ADC_SMOKE) && !defined(USE_SPI_SMOKE) && \
    !defined(USE_CAN_SMOKE) && !defined(USE_PWM_SMOKE)
#error "�Цb�sĶ�ﶵ�w�q�䤤�@�ӡGUSE_ADC_SMOKE / USE_SPI_SMOKE / USE_CAN_SMOKE / USE_PWM_SMOKE"
#endif
#if (defined(USE_ADC_SMOKE) && (defined(USE_SPI_SMOKE)||defined(USE_CAN_SMOKE)||defined(USE_PWM_SMOKE))) || \
    (defined(USE_SPI_SMOKE) && (defined(USE_CAN_SMOKE)||defined(USE_PWM_SMOKE))) || \
    (defined(USE_CAN_SMOKE) && defined(USE_PWM_SMOKE))
#error "�@���u��ҥΤ@�� smoke test"
#endif

static inline void tiny_delay(volatile uint32_t n){ while(n--){ __asm__("nop"); } }

/* ---------- ADC SMOKE ---------- */
#ifdef USE_ADC_SMOKE
#ifndef ADC_SMOKE_CH
#define ADC_SMOKE_CH  (0u)     /* �Q��������q�D */
#endif
extern void     adc0_init_one_shot(void);           /* �Ӧ� adc_smoke_test.c */
extern uint16_t adc0_read_once(uint8_t ch);

volatile uint16_t sm_adc_last = 0;

int main(void){
    SysClk_Init();
    adc0_init_one_shot();
    for(;;){
        sm_adc_last = adc0_read_once(ADC_SMOKE_CH);
        ADC_0.MCR.B.NSTART = 1u;        /* Ĳ�o�U�@�� */
        tiny_delay(5000);
    }
}
#endif /* USE_ADC_SMOKE */

/* ---------- SPI SMOKE ---------- */
#ifdef USE_SPI_SMOKE
/* ��ĳ�A�w�g�� pads_init_spi0_pullup_miso() �� MISO ���԰��A�L�~���w��]��Ū��í�w�� */
extern void pads_init_spi0_pullup_miso(void);
extern void spi0_init_master(void);
extern uint16_t spi0_txrx_once(uint16_t tx);        /* �A�{�� SPI.c �� API */

volatile uint16_t sm_spi_last = 0;
volatile uint32_t sm_spi_sr   = 0;

int main(void){
    SysClk_Init();
    SUIL2_Init();                 /* SOUT/SIN/SCK/PCS0 �ƥ� */
    pads_init_spi0_pullup_miso(); /* �L�w��۴� */
    spi0_init_master();

    for(;;){
        sm_spi_last = spi0_txrx_once(0xA5A5);
        sm_spi_sr   = SPI_0.SR.R;          /* �b Registers/Expressions �[��X�� */
        tiny_delay(20000);
    }
}
#endif /* USE_SPI_SMOKE */

/* ---------- CAN SMOKE (internal loopback) ---------- */
#ifdef USE_CAN_SMOKE
extern void can0_loopback_init(void);       /* �Ӧ� can_smoke_test.c */
extern void can0_loopback_smoke_once(void); /* �e1 byte �æۦ� */

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
extern void pwm_smoke_init(void);  /* �Ӧ� pwm_smoke_test.c */
extern void pwm_smoke_step(void);

//int main(void){
//    SysClk_Init();
//    SUIL2_Init();      /* C[4]/C[6]/C[7] �� FlexPWM0 X/A/B (OBE=1, SSS=2) */
//    pwm_smoke_init();
//    for(;;){
//        pwm_smoke_step();          /* �g���ʧ��ܥe�Ť�ALDOK �۲M�i�� */
//        tiny_delay(80000);
//    }
//}
#endif /* USE_PWM_SMOKE */
