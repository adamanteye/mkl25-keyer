#ifndef MYADC_H_
#define MYADC_H_

#include "derivative.h"

void adc_init(void)
{
    // enable ADC0 clock
    SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK;
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
    // Set pin0 of PORTC as analog function
    PORTC_PCR0 = 0x0000;
    // long sample time single-end 12bit conversion
    ADC0_CFG1 = 0X00000014;
    // ADxxxa channel select
    ADC0_CFG2 = 0X00000000;
    // default voltage reference Vrefh and Vrefl,software trigger
    ADC0_SC2 = 0X00000000;
    // continuous conversions
    ADC0_SC3 = 0X00000008;
    // interrupt disable and select ADC0_SE14 channel as input
    ADC0_SC1A = 0X000000E;
}

unsigned short adc0_data()
{
    unsigned short adc_result;
    // check convert complete flag, 查询直到完成一次转换，有数据可读
    while ((ADC0_SC1A & ADC_SC1_COCO_MASK) != ADC_SC1_COCO_MASK)
        ;
    // 读取12bit数据，或(ADC0_RA>>2)变为1个字节
    adc_result = ADC0_RA;
    // clear flag and start conversion
    ADC0_SC1A &= ~ADC_SC1_COCO_MASK;
    return adc_result;
}

#endif