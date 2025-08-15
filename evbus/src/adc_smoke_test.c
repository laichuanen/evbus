/**
 * adc_smoke_test.c
 * Minimal ADC smoke test (polling, one-shot) for MPC5744P using old-style bitfield headers.
 *
 * Usage:
 *  - Add this file to a fresh S32DS project that uses the old Freescale/NXP headers (with .R/.B access).
 *  - Make sure your system/clock init enables the ADC peripheral clock in RUN mode.
 *  - Build & Debug. Watch the variable `last` in Expressions, or inspect ADC_0.CDR[ADC_CH].
 *
 * Notes:
 *  - This test reads a single ADC channel repeatedly. Change ADC_CH to match your board's routed pin.
 *  - If you already have a system clock init (MC_ME / RUN_PC / PLL), keep it; this file focuses on ADC only.
 *  - If your board uses a different reference or alignment, adjust MCR fields accordingly.
 */
#include "MPC5744P.h"
#include <stdint.h>

#ifndef ADC_CH
#define ADC_CH   (0u)      /* Set to the physical ADC channel you want to measure */
#endif

/* Optional: crude delay to make single-stepping friendlier (no reliance on PIT/STM). */
static void small_delay(volatile uint32_t n)
{
    while (n--) { __asm__("nop"); }
}

/* Initialize ADC_0 for one-shot conversion on ADC_CH. */
static void adc0_init_one_shot(void)
{
    /* 1) Bring ADC out of power-down and use default config */
    ADC_0.MCR.R = 0u;           /* reset-ish defaults */
    ADC_0.MCR.B.PWDN = 0u;      /* exit power-down */

    /* 2) Sampling/clock control: conservative default values (tune as needed) */
    ADC_0.CTR0.R = 0x00000008u;
    ADC_0.CTR1.R = 0x00000008u;

    /* 3) Enable the desired channel in Normal mode mask (NCMR0 covers channels 0..15) */
    ADC_0.NCMR0.R = (1u << ADC_CH);

    /* 4) Clear any stale status flags */
    ADC_0.ISR.R = 0xFFFFFFFFu;  /* write-1-to-clear */

    /* 5) Kick the first conversion */
    ADC_0.MCR.B.NSTART = 1u;    /* Normal conversion start */
}

/* Perform one conversion and return the 12-bit result of channel `ch`. */
static uint16_t adc0_read_once(uint8_t ch)
{
	/* Re-trigger for the next sample */
	ADC_0.MCR.B.NSTART = 1u;
    /* Wait until at least one channel end-of-conversion occurs (ISR.EOC set). */
    while ((ADC_0.ISR.R & (1u << 1)) == 0u) {
        /* spin */
    }

    /* Read result register for channel `ch`.
       In old headers CDR[ch].R contains VALID at bit19 and result in low bits (commonly 12-bit). */
    uint32_t raw = ADC_0.CDR[ch].R;
    while ((raw & (1u << 19)) == 0u) {     /* wait for VALID */
        raw = ADC_0.CDR[ch].R;
    }
    return (uint16_t)(raw & 0x0FFFu);      /* extract 12-bit data (adjust if you use different alignment) */
}

static uint16_t call_adc0_read_once()
{
    return adc0_read_once((uint8_t)ADC_CH);
}

//int main(void)
//{
//    volatile uint16_t last = 0u;
//
//    /* TODO: Ensure system/peripheral clocks are enabled for ADC_0 in RUN mode (MC_ME / RUN_PC). */
//    /* This file focuses on ADC usage; keep your existing clock init if you already have one. */
//
//    adc0_init_one_shot();
//
//    for (;;)
//    {
//        last = adc0_read_once((uint8_t)ADC_CH);
//
//        /* Re-trigger for the next sample */
//        ADC_0.MCR.B.NSTART = 1u;
//
//        /* Optional tiny delay to avoid hammering the peripheral when single-stepping */
//        small_delay(1000u);
//    }
//
//    /* not reached */
//    /* return 0; */
//}
