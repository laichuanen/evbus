/**
 * pwm_smoke_test.c
 * Minimal FlexPWM0 smoke test for MPC5744P (no scope needed).
 * - Old-style .R/.B register headers (MPC5744P.h)
 * - Uses FlexPWM_0 Submodule 1 (SM1), periodically changes duty
 * - Observe in debugger: CNT counting, LDOK auto-clear, duty change
 *
 * How to use:
 *   (A) �����I�s API�]��ĳ�^
 *       extern void pwm_smoke_init(void);
 *       extern void pwm_smoke_step(void);
 *       SysClk_Init();     // �A�쥻��������l��
 *       SUIL2_Init();      // �A�쥻���}���l�ơ]C[4]/C[6]/C[7] �� FlexPWM X/A/B�^
 *       pwm_smoke_init();
 *       for(;;){ pwm_smoke_step(); }
 *
 *   (B) �ϥΤ��� main�]�b�sĶ�ﶵ�[�J -DUSE_PWM_SMOKE_MAIN�^
 */

#include "MPC5744P.h"
#include <stdint.h>

/* �ѧA���M�״��ѡ]�ЫO�d�A���J����@�^ */
extern void PWM_SysClk_Init(void);
extern void PWM_SUIL2_Init(void);

/* ��K�b Debug/Expressions �[�� */
volatile uint16_t pwm_smoke_duty_dbg = 300;  /* �ثe�]�w�� B �q�D�W�ɽt */
volatile uint16_t pwm_cnt_snapshot   = 0;    /* ���� SM1 �p�ƾ� CNT �� */

/* ��l�ơG�Ұ� FlexPWM_0 �� SM1�A�]�w�g���P��X�}��]�� SUIL2_Init �t�n�^ */
void pwm_smoke_init(void)
{
    /* ����Ҧ��l�ҲեH�K�]�w */
    FlexPWM_0.MCTRL.B.RUN = 0u;

    /* Submodule 1 �򥻭p�Ƴ]�w�GINIT=0�AVAL1=�g���F���B�H 1250 ���� */
    FlexPWM_0.SUB[1].INIT.R = 0u;
    FlexPWM_0.SUB[1].VAL0.R = 0u;          /* reload point */
    FlexPWM_0.SUB[1].VAL1.R = 1250u;       /* �g�� (modulo)�F�W�v�̧A�������өw */

    /* ��l�e�Ť�GB �q�D�AVAL4 �W�ɽt�BVAL5 �U���t */
    FlexPWM_0.SUB[1].VAL4.R = pwm_smoke_duty_dbg;  /* rising */
    FlexPWM_0.SUB[1].VAL5.R = 1200u;               /* falling */

    /* �}�� SM1 �� A/B/X ��X�F�A���M�ױ`�� 0x0222�]SM1 A/B/X�^ */
    FlexPWM_0.OUTEN.R = 0x0222u;
    FlexPWM_0.MASK.R  = 0x0000u;

    /* �N shadow �g�J active�]LDOK �|�۲M�^ */
    /* �N shadow �g�J�n�D�]LDOK �|�b�Ĥ@�� reload �ɦ۲M�^ */
    FlexPWM_0.MCTRL.B.LDOK = (1u << 1);

    /* ���Ұ� SM1 �X�X �@�w�n�b�� LDOK �e�N���p�ƾ��]�_�� */
    FlexPWM_0.MCTRL.B.RUN |= (1u << 1);

    /* �]�i��^�{�b�A�� LDOK �۲M�F�S���o�q�]�ॿ�`�] */
//    volatile int i = 0;
//    for (i = 0; i < 10; ++i) { __asm__("nop"); }  // �p���@�U
    while (FlexPWM_0.MCTRL.B.LDOK != 0u) { /* wait auto-clear */ }
}

/* �C���I�s���e�Ť񩹤W���A�ç�s shadow��active�C��b�D�j��I�s�Y�i�C */
void pwm_smoke_step(void)
{
    /* �b 300��1200 �������ʡA�e�Ť��ܤƩ����[�� */
    pwm_smoke_duty_dbg += 50u;
    if (pwm_smoke_duty_dbg > 1200u) {
        pwm_smoke_duty_dbg = 300u;
    }

    /* �g�J�s duty �� shadow */
    FlexPWM_0.SUB[1].VAL4.R = pwm_smoke_duty_dbg;  /* rising edge moves */
    FlexPWM_0.SUB[1].VAL5.R = 1200u;               /* fixed falling edge */

    /* shadow �� active */
    FlexPWM_0.MCTRL.B.LDOK = (1u << 1);
    /* ���Ұ� SM1 �X�X �@�w�n�b�� LDOK �e�N���p�ƾ��]�_�� */
	FlexPWM_0.MCTRL.B.RUN |= (1u << 1);
    while (FlexPWM_0.MCTRL.B.LDOK != 0u) { }

    /* ���� CNT ���A�b Expressions �ݱo��p�ƾ��b�] */
    pwm_cnt_snapshot = (uint16_t)FlexPWM_0.SUB[1].CNT.R;
    volatile uint16_t pwm_cnt_snapshot_result = pwm_cnt_snapshot;
}

/* ��ΡG���� main�]�[�W -DUSE_PWM_SMOKE_MAIN �ҥΡ^ */
#ifdef USE_PWM_SMOKE_MAIN
//int main(void)
//{
//    SysClk_Init();   /* �A���J�������]�w�]�ݽT�O AC0/MOTC_CLK �w�ҥΡ^ */
//    SUIL2_Init();    /* �A���J���}��]�w�GC[4]/C[6]/C[7] �� FlexPWM0 X/A/B�AOBE=1, SSS=2 */
//
//    pwm_smoke_init();
//
//    for (;;)
//    {
//        pwm_smoke_step();
//
//        /* �p����A���ܤƦb Debug ������e���[�� */
//        for (volatile uint32_t d = 0; d < 80000u; ++d) { __asm__("nop"); }
//    }
//}
#endif
