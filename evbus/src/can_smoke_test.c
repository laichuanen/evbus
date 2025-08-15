/**
 * can_smoke_test.c
 * Minimal FlexCAN0 loopback (internal) smoke test for MPC5744P.
 * - No external transceiver/wiring required.
 * - Uses old-style .R/.B register headers.
 *
 * How to use:
 *   1) Keep your existing SysClk_Init() (provides clocks & RUN_PC mapping).
 *   2) Add this file to the project. It provides:
 *        - can0_loopback_init()
 *        - can0_loopback_smoke_once()
 *      You can call these from your test main() (or replace your main during bring-up).
 *   3) Build & Debug. Watch variables can_tx_last, can_rx_last, can_iflag_snapshot.
 *
 * Notes:
 *   - We enable CTRL1.LPB (internal loopback) and keep self-reception enabled (SRXDIS=0).
 *   - Two message buffers are used on CAN_0:
 *       MB[0] = RX, extended ID 0x12340000
 *       MB[1] = TX, extended ID 0x12340000
 *   - Bit timing set for 500 kbps with 40 MHz OSC (same as your CAN.c).

 */
#include "MPC5744P.h"
#include <stdint.h>
#include "can_globals.h"

/* Extern your existing system clock init (from CAN.c) */
extern void SysClk_Init(void);

/* Exported variables for easy watching in debugger */
// 移除以下這些
// volatile uint8_t can_tx_last = 0;
// volatile uint8_t can_rx_last = 0;
// volatile uint32_t can_iflag_snapshot = 0;

// 在需要用到的地方改成 extern 宣告
extern volatile uint8_t can_tx_last;
extern volatile uint8_t can_rx_last;
extern volatile uint32_t can_iflag_snapshot;

/* Quick helpers for flags */
#define MB_FLAG(n)        (1u << (n))

static void can0_loopback_init(void)
{
    /* ---- Enter freeze to configure ---- */
    CAN_0.MCR.R = (0
        | 0x40000000u   /* FRZ */
        | 0x10000000u   /* HALT */
        | 0x00010000u   /* BCC: individual masking */
        | 0x0000003Fu); /* enable 64 MBs */

    /* Wait until we are frozen & ready */
    while (0 == CAN_0.MCR.B.FRZACK) {}
    while (0 == CAN_0.MCR.B.NOTRDY) {}

    /* Bit timing @ 500 kbps with 40MHz, and enable internal loopback (LPB) */
    CAN_0.CTRL1.R = 0x04E40004u | 0x00001000u; /* LPB bit set */

    /* Make all MBs inactive, set masks to match all bits */
    volatile uint32_t i = 0;
    for (i = 0; i < 64; i++) {
        CAN_0.MB[i].CS.R = 0;
        CAN_0.RXIMR[i].R = 0x1FFFFFFFu;
    }

    /* RX MB0: extended ID 0x12340000, empty & active */
    CAN_0.MB[0].ID.R = 0x12340000u;
    CAN_0.MB[0].CS.R = 0x04200000u;   /* RX active, IDE */

    /* TX MB1: preset INACTIVE, IDE+SRR so we can write ID/DATA before launching */
    CAN_0.MB[1].CS.R = 0x08600000u;   /* INACTIVE + IDE + SRR */
    CAN_0.MB[1].ID.R = 0x12340000u;

    /* Clear all flags */
    CAN_0.IFLAG1.R = 0xFFFFFFFFu;

    /* Leave freeze (enable module) */
    CAN_0.MCR.B.HALT = 0;
    while (1 == CAN_0.MCR.B.FRZACK) {}
    while (1 == CAN_0.MCR.B.NOTRDY) {}
}

static void can0_loopback_smoke_once(void)
{
    /* Prepare one byte payload (toggle) */
    can_tx_last ^= 0x01u;
    CAN_0.MB[1].DATA.B[0] = can_tx_last;

    /* Launch TX on MB1: DLC=1, IDE=1, SRR=1, CODE=0xC (Transmit) */
    CAN_0.MB[1].CS.R = 0x0C610000u;

    /* 以下需要接上硬體測試 */
//    /* Wait TX complete flag for MB1 */
//    while ((CAN_0.IFLAG1.R & MB_FLAG(1)) == 0u) { }
//    CAN_0.IFLAG1.R = MB_FLAG(1);  /* clear MB1 flag */
//
//    /* Wait RX flag for MB0 */
//    while ((CAN_0.IFLAG1.R & MB_FLAG(0)) == 0u) { }
//    /* Lock & read */
//    (void)CAN_0.MB[0].CS.R;
//    (void)CAN_0.MB[0].ID.R;
//    can_rx_last = CAN_0.MB[0].DATA.B[0];
//    (void)CAN_0.TIMER.R;  /* unlock all RX MBs */
//    CAN_0.IFLAG1.R = MB_FLAG(0);  /* clear MB0 flag */
//
//    can_iflag_snapshot = CAN_0.IFLAG1.R;

    /* 以下為沒有硬體的測試方式 */
    /* Wait TX complete flag for MB1 — 無硬體模式就略過等待 */
    if (CAN_0.IFLAG1.R & MB_FLAG(1)) {
        CAN_0.IFLAG1.R = MB_FLAG(1);  /* clear MB1 flag */
    }

    /* Wait RX flag for MB0 — 無硬體模式就略過等待 */
    if (CAN_0.IFLAG1.R & MB_FLAG(0)) {
        /* Lock & read */
        (void)CAN_0.MB[0].CS.R;
        (void)CAN_0.MB[0].ID.R;
        can_rx_last = CAN_0.MB[0].DATA.B[0];
        (void)CAN_0.TIMER.R;  /* unlock all RX MBs */
        CAN_0.IFLAG1.R = MB_FLAG(0);  /* clear MB0 flag */
    }

    can_iflag_snapshot = CAN_0.IFLAG1.R;
}

/* Optional standalone main for direct bring-up.
   Define USE_CAN_SMOKE_MAIN to compile this main(). */
#ifdef USE_CAN_SMOKE_MAIN
//int main(void)
//{
//    SysClk_Init();              /* reuse user's clock init */
//    can0_loopback_init();
//
//    for(;;){
//        can0_loopback_smoke_once();
//        for(volatile uint32_t d=0; d<10000; ++d){ __asm__("nop"); }
//    }
//}
#endif
