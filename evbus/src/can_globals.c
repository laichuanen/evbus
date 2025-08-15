// can_globals.c
#include <stdint.h>
volatile uint8_t can_tx_last = 0;
volatile uint8_t can_rx_last = 0;
volatile uint32_t can_iflag_snapshot = 0;
