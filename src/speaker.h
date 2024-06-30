#ifndef SPEAKER_H_
#define SPEAKER_H_

#include "derivative.h"

#define PWM0_CLK_FREQ 164 * 1000
#define SPEAKER_C0    (unsigned short)(PWM0_CLK_FREQ / 523)
#define SPEAKER_D0    (unsigned short)(PWM0_CLK_FREQ / 587)
#define SPEAKER_E0    (unsigned short)(PWM0_CLK_FREQ / 659)
#define SPEAKER_F0    (unsigned short)(PWM0_CLK_FREQ / 698)
#define SPEAKER_G0    (unsigned short)(PWM0_CLK_FREQ / 784)
#define SPEAKER_A0    (unsigned short)(PWM0_CLK_FREQ / 880)
#define SPEAKER_B0    (unsigned short)(PWM0_CLK_FREQ / 988)
#define SPEAKER_N0    0

void speaker_init(void)
{
    SIM_SOPT2 |= SIM_SOPT2_TPMSRC(1);
    SIM_SOPT2 &= ~SIM_SOPT2_PLLFLLSEL_MASK;
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
    SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK;
    PORTC_PCR8 = 0x0300;
    TPM0_CNT = 0;
    TPM0_MOD = 0x00;
    TPM0_SC = (0x0008 | 0x0007);
    TPM0_C4SC = (0x0020 | 0x0008);
    TPM0_C4V = 0x00;
}

void speaker_set_note(unsigned short period)
{
    TPM0_MOD = period;
    TPM0_C4V = period / 2;
}

#endif /* SPEAKER_H_ */
