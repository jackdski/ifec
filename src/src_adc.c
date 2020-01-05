/*
 * src_adc.c
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#include "src_adc.h"
#include "driverlib.h"
#include "device.h"

static uint16_t buck_one_result_mv;
static uint16_t buck_two_result_mv;
static uint16_t mppt_one_result_mv;
static uint16_t mppt_two_result_mv;

static float buck_one_result_v;
static float buck_two_result_v;
static float mppt_one_result_v;
static float mppt_two_result_v;


// TODO: find out how many ADCs are needed and add them in
void init_adc(void) {
    /* ADCA */

    // Setup VREF
    ADC_setVREF(ADCA_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);

    // Set ADCCLK Divider to /4
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);

    // set pulse positions to late
    ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV);

    // enable ADC's
    ADC_enableConverter(ADCA_BASE);
    DEVICE_DELAY_US(1000);

    // Configure SOCs of ADCA
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN0, 15);    // LaunchPad Pin 70
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN1, 15);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN2, 15);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN2, 15);


    // Set SOC1 to set the interrupt 1 flag. Enable the interrupt and make sure its flag is cleared.
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER1);
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER2);
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER4, ADC_SOC_NUMBER3);

    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER2);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER3);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER4);

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER2);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER3);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
}

float get_buck_one_v(void) {
    return buck_one_result_v;
}

uint32_t adc_convert_to_mv(uint32_t adc_result) {
    return ((VREF_MV * adc_result) / ADC_MAX_VALUE);
}

float adc_convert_to_v(uint32_t adc_result) {
    return (((float)adc_result * VREF_MV_F) / ADC_MAX_VALUE_F);
}

__interrupt void adc_buck_one_irq(void) {
    buck_one_result_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
    buck_one_result_v = adc_convert_to_v(buck_one_result_mv);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

__interrupt void adc_buck_two_irq(void) {
    buck_two_result_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);
    buck_two_result_v = adc_convert_to_v(buck_two_result_mv);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER2);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

__interrupt void adc_mppt_one_irq(void) {
    mppt_one_result_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    mppt_one_result_v = adc_convert_to_v(mppt_one_result_mv);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER3);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

__interrupt void adc_mppt_two_irq(void) {
    mppt_two_result_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
    mppt_two_result_v = adc_convert_to_v(mppt_two_result_mv);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}
