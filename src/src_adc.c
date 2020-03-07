/*
 * src_adc.c
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#include "src_adc.h"
#include "src_epwm.h"
#include "driverlib.h"
#include "device.h"

/** Bucks */
static float buck_one_result_v;
static float buck_two_result_v;
static uint16_t buck_one_result_mv;
static uint16_t buck_two_result_mv;

/** MPPT */
static float mppt_one_result_v;
static float mppt_two_result_v;
static float mppt_one_result_i;
static float mppt_two_result_i;
static uint16_t mppt_one_result_mv;
static uint16_t mppt_two_result_mv;

/** Battery */
static float battery_result_v;
static float battery_result_i;
static uint16_t battery_result_mv;


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
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN1, 15);    // Battery V
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN4, 15);    // MPPT V
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN3, 15);    // MPPT I


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


/**********************************************************
 *                      G E T S
 **********************************************************/

/**
 * @brief Converts the ADC result to millivolts
 *
 * @details Multiplies the ADC results by the number of mV
 *      in VREF and then divides by the maximum value of the ADC.
 *
 * @return buck_one_result_v variable [V]
 */
float get_buck_v(uint32_t buck_base) {
    switch(buck_base) {
    case(BUCK3V3_BASE): return buck_one_result_v;
    case(BUCK5V0_BASE): return buck_two_result_v;
    }
    return -1.0;
}

float get_mppt_v(uint32_t mppt_base) {
    switch(mppt_base) {
    case(MPPT1_BASE): return mppt_one_result_v;
    case(MPPT2_BASE): return mppt_two_result_v;
    }
    return -1.0;
}

float get_mppt_i(uint32_t mppt_base) {
    switch(mppt_base) {
    case(MPPT1_BASE): return mppt_one_result_i;
    case(MPPT2_BASE): return mppt_two_result_i;
    }
    return -1.0;
}

float get_battery_v(void) {
    return battery_result_v;
}

float get_battery_i(void) {
    return battery_result_i;
}

/**********************************************************
 *                  C O N V E R S I O N S
 **********************************************************/

/**
 * @brief Converts the ADC result to millivolts
 *
 * @details Multiplies the ADC results by the number of mV
 *      in VREF and then divides by the maximum value of the ADC.
 *
 * @return Value in mV
 */
uint32_t adc_convert_to_mv(uint32_t adc_result) {
    return ((VREF_MV * adc_result) / ADC_MAX_VALUE);
}

/**
 * @brief Converts the ADC result to volts
 *
 * @details Multiplies the ADC results by the value of VREF
 *      and then divides by the maximum value of the ADC.
 *
 * @return Value in Volts
 */
float adc_convert_to_v(uint32_t adc_result) {
    return (((float)adc_result * VREF_MV_F) / ADC_MAX_VALUE_F);
}



/**********************************************************
 *                  I N T E R R U P T S
 **********************************************************/

/** 3.3V Buck **/
__interrupt void adc_buck_one_irq(void) {
    buck_one_result_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
    buck_one_result_v = adc_convert_to_v(buck_one_result_mv);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

/** 5V Buck **/
__interrupt void adc_battery_v_irq(void) {
    battery_result_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);
    battery_result_v = adc_convert_to_v(battery_result_mv);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER2);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
    Interrupt_clearACKGroup(0xFFF);
}

__interrupt void adc_mppt_one_v_irq(void) {
    mppt_one_result_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    mppt_one_result_v = adc_convert_to_v(mppt_one_result_mv);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER3);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP10);
    Interrupt_clearACKGroup(0xFFF);
}

__interrupt void adc_mppt_one_i_irq(void) {
    uint16_t adc_result = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
    float adc_i = adc_convert_to_v(adc_result);
    mppt_one_result_i = (adc_i - mppt_one_result_v); //   / MPPT_SHUNT_R;
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
    Interrupt_clearACKGroup(0xFFF);
}

