/*
 * main implementation: use this 'C' sample to create your own application
 *
 */

#include "derivative.h" /* include peripheral declarations */
#include "adc_smoke_test.c"
#include "spi_smoke_test.c"
#include "can_smoke_test.c"
//#include "pwm_smoke_test.c"

extern void xcptn_xmpl(void);

/* CAN external function */
extern void can0_loopback_init(void);
extern void can0_loopback_smoke_once(void);

int main(void)
{
	volatile int counter = 0;
	volatile uint16_t adc_last = 0u;
	volatile uint16_t spi_last = 0;
	volatile uint32_t sr_snapshot = 0;

	xcptn_xmpl ();              /* Configure and Enable Interrupts */

	/* ADC Initialization */
	Sys_Init();
	SUIL2_Init();
	ADC_Init();
	/* SPI Initialization */
	sys_config();
	SPI0_padconfig();
	init_SPI0();
	/* CAN Initialization */
	SysClk_Init();          // �t��/�׬y�Ʈ����BRUN_PC ��
	/* PWM Initialization */
	PWM_SysClk_Init();   /* �A���J�������]�w�]�ݽT�O AC0/MOTC_CLK �w�ҥΡ^ */
	PWM_SUIL2_Init();    /* �A���J���}��]�w�GC[4]/C[6]/C[7] �� FlexPWM0 X/A/B�AOBE=1, SSS=2 */

	/* ADC smoke test Initialization */
	adc0_init_one_shot();
	/* SPI smoke test Initialization */
	clocks_init();
	pads_init_spi0_pullup_miso();
	spi0_init();
	/* CAN smoke test Initialization */
	can0_loopback_init();   // �ҥ� FlexCAN0 + ���� loopback
	/* PWM smoke test Initialization */
	pwm_smoke_init();


	/* Loop forever */
	for(;;) {

		/* ADC smoke test */
		adc_last = call_adc0_read_once();
		/* Re-trigger for the next sample */
		ADC_0.MCR.B.NSTART = 1u;

		/* SPI smoke test */
		SPI_0.PUSHR.PUSHR.R  = 0x8001AAAA;     // Transmit data
		read_data_SPI_0();
    	spi_last = spi0_txrx_once(0xA5A5);
        sr_snapshot = SPI_0.SR.R;   // ��K�b Debug �����ݺX��

        /* CAN smoke test */
        can0_loopback_smoke_once();   // �e1 byte�B���� TX �����B�b�P�Ҳզ��^

        /* PWM smoke test */
        pwm_smoke_step();


		/* Optional tiny delay to avoid hammering the peripheral when single-stepping */
//		small_delay(10u);

	   	counter++;
	}
}
