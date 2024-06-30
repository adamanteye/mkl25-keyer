#ifndef MYGPIO_H_
#define MYGPIO_H_

#include "derivative.h"
#include "speaker.h"

void gpio_init()
{
    SIM_SCGC5 |= (SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTA_MASK);

    GPIOC_PDDR |= 1 << 12; /* LED 1 Output */
    PORTC_PCR12 = 0x0100;
    GPIOC_PDDR |= 1 << 13; /* LED 2 Output */
    PORTC_PCR13 = 0x0100;
    GPIOC_PDDR |= 1 << 9; /* LED Common */
    PORTC_PCR9 = 0x0100;
    GPIOB_PDDR |= 1 << 19; /* LED Common */
    PORTB_PCR19 = 0x0100;
    GPIOE_PDDR |= 1 << 3; /* 继电器 Output */
    PORTE_PCR3 = 0x0100;

    NVIC_ISER |= 0x40000000; /* 开启PORTA中断 */

    GPIOA_PDDR &= ~0x0002;   /* PTA1 Input */
    PORTA_PCR1 = 0x0A0102;   /* 下降沿触发中断 */
    GPIOA_PDDR &= ~(1 << 5); /* PTA5 Input */
    PORTA_PCR5 = 0x0A0102;
    GPIOA_PDDR &= ~(1 << 12); /* PTA12 Input */
    PORTA_PCR12 = 0x0A0102;
    GPIOA_PDDR &= ~(1 << 14); /* PTA14 Input */
    PORTA_PCR14 = 0x0A0102;

    GPIOE_PDOR &= ~(1 << 3); /* 高电平将会触发继电器动作 */
    GPIOC_PDOR |= 1 << 12;   /* LED 1 默认熄灭 */
    GPIOC_PDOR |= 1 << 13;   /* LED 2 默认熄灭 */
    GPIOC_PDOR |= 1 << 9;    /* LED Common 保持上电, 始终通 */
    GPIOB_PDOR |= 1 << 19;
}

#endif /* MYGPIO_H_ */
