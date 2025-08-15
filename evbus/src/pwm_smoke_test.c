/**
 * pwm_smoke_test.c
 * Minimal FlexPWM0 smoke test for MPC5744P (no scope needed).
 * - Old-style .R/.B register headers (MPC5744P.h)
 * - Uses FlexPWM_0 Submodule 1 (SM1), periodically changes duty
 * - Observe in debugger: CNT counting, LDOK auto-clear, duty change
 *
 * How to use:
 *   (A) 直接呼叫 API（建議）
 *       extern void pwm_smoke_init(void);
 *       extern void pwm_smoke_step(void);
 *       SysClk_Init();     // 你原本的時鐘初始化
 *       SUIL2_Init();      // 你原本的腳位初始化（C[4]/C[6]/C[7] → FlexPWM X/A/B）
 *       pwm_smoke_init();
 *       for(;;){ pwm_smoke_step(); }
 *
 *   (B) 使用內建 main（在編譯選項加入 -DUSE_PWM_SMOKE_MAIN）
 */

#include "MPC5744P.h"
#include <stdint.h>

/* 由你的專案提供（請保留你的既有實作） */
extern void PWM_SysClk_Init(void);
extern void PWM_SUIL2_Init(void);

/* 方便在 Debug/Expressions 觀察 */
volatile uint16_t pwm_smoke_duty_dbg = 300;  /* 目前設定的 B 通道上升緣 */
volatile uint16_t pwm_cnt_snapshot   = 0;    /* 取樣 SM1 計數器 CNT 值 */

/* 初始化：啟動 FlexPWM_0 的 SM1，設定週期與輸出腳位（由 SUIL2_Init 配好） */
void pwm_smoke_init(void)
{
    /* 停止所有子模組以便設定 */
    FlexPWM_0.MCTRL.B.RUN = 0u;

    /* Submodule 1 基本計數設定：INIT=0，VAL1=週期；此處以 1250 為例 */
    FlexPWM_0.SUB[1].INIT.R = 0u;
    FlexPWM_0.SUB[1].VAL0.R = 0u;          /* reload point */
    FlexPWM_0.SUB[1].VAL1.R = 1250u;       /* 週期 (modulo)；頻率依你的時鐘而定 */

    /* 初始占空比：B 通道，VAL4 上升緣、VAL5 下降緣 */
    FlexPWM_0.SUB[1].VAL4.R = pwm_smoke_duty_dbg;  /* rising */
    FlexPWM_0.SUB[1].VAL5.R = 1200u;               /* falling */

    /* 開啟 SM1 的 A/B/X 輸出；你的專案常用 0x0222（SM1 A/B/X） */
    FlexPWM_0.OUTEN.R = 0x0222u;
    FlexPWM_0.MASK.R  = 0x0000u;

    /* 將 shadow 寫入 active（LDOK 會自清） */
    /* 將 shadow 寫入要求（LDOK 會在第一次 reload 時自清） */
    FlexPWM_0.MCTRL.B.LDOK = (1u << 1);

    /* 先啟動 SM1 —— 一定要在等 LDOK 前就讓計數器跑起來 */
    FlexPWM_0.MCTRL.B.RUN |= (1u << 1);

    /* （可選）現在再等 LDOK 自清；沒有這段也能正常跑 */
//    volatile int i = 0;
//    for (i = 0; i < 10; ++i) { __asm__("nop"); }  // 小等一下
    while (FlexPWM_0.MCTRL.B.LDOK != 0u) { /* wait auto-clear */ }
}

/* 每次呼叫讓占空比往上掃，並更新 shadow→active。放在主迴圈呼叫即可。 */
void pwm_smoke_step(void)
{
    /* 在 300～1200 之間掃動，占空比變化易於觀察 */
    pwm_smoke_duty_dbg += 50u;
    if (pwm_smoke_duty_dbg > 1200u) {
        pwm_smoke_duty_dbg = 300u;
    }

    /* 寫入新 duty 到 shadow */
    FlexPWM_0.SUB[1].VAL4.R = pwm_smoke_duty_dbg;  /* rising edge moves */
    FlexPWM_0.SUB[1].VAL5.R = 1200u;               /* fixed falling edge */

    /* shadow → active */
    FlexPWM_0.MCTRL.B.LDOK = (1u << 1);
    /* 先啟動 SM1 —— 一定要在等 LDOK 前就讓計數器跑起來 */
	FlexPWM_0.MCTRL.B.RUN |= (1u << 1);
    while (FlexPWM_0.MCTRL.B.LDOK != 0u) { }

    /* 取樣 CNT 讓你在 Expressions 看得到計數器在跑 */
    pwm_cnt_snapshot = (uint16_t)FlexPWM_0.SUB[1].CNT.R;
    volatile uint16_t pwm_cnt_snapshot_result = pwm_cnt_snapshot;
}

/* 選用：內建 main（加上 -DUSE_PWM_SMOKE_MAIN 啟用） */
#ifdef USE_PWM_SMOKE_MAIN
//int main(void)
//{
//    SysClk_Init();   /* 你的既有時鐘設定（需確保 AC0/MOTC_CLK 已啟用） */
//    SUIL2_Init();    /* 你的既有腳位設定：C[4]/C[6]/C[7] → FlexPWM0 X/A/B，OBE=1, SSS=2 */
//
//    pwm_smoke_init();
//
//    for (;;)
//    {
//        pwm_smoke_step();
//
//        /* 小延遲，讓變化在 Debug 視窗更容易觀察 */
//        for (volatile uint32_t d = 0; d < 80000u; ++d) { __asm__("nop"); }
//    }
//}
#endif
