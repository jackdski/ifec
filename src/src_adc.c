/*
 * src_adc.c
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#include "src_adc.h"
#include "driverlib.h"
#include "device.h"
#include "src_epwm.h"

/**********************************************************
 *          P R I V A T E   V A R I A B L E S
 **********************************************************/

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


/**********************************************************
 *                      I N I T S
 **********************************************************/

// TODO: find out how many ADCs are needed and add them in
/**
 * @brief Initializes ADCA and ADCB for the various converters
 *
 * @details Sets the ADC Reference Voltage to 3.3V. The interrupt will go off
 *  after the ADC conversion is finished.
 */
void init_voltage_adc(void) {
    /* VOLTAGE ADC - ADCA */

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
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN0, 15);    // 3.3V Buck - LaunchPad Pin 70
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN1, 15);    // 5.0V Buck
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN2, 15);    // MPPT1
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN3, 15);    // MPPT2
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER4, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN4, 15);    // Battery


    // Set SOC interrupts. Enable the interrupts and make sure flags are cleared.
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);    // 3.3V Buck
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER1);    // 5.0V Buck
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER2);    // MPPT1
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER4, ADC_SOC_NUMBER3);    // MPPT2

    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER2);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER3);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER4);

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER2);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER3);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
}

void init_current_adc(void) {
    ADC_setVREF(ADCB_BASE, ADC_REFERENCE_EXTERNAL, ADC_REFERENCE_3_3V); // 3.3V Vref has no affect
    ADC_setPrescaler(ADCB_BASE, ADC_CLK_DIV_4_0);
    ADC_setInterruptPulseMode(ADCB_BASE, ADC_PULSE_END_OF_CONV);

    ADC_enableConverter(ADCB_BASE);
    DEVICE_DELAY_US(1000);

    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN0, 15);    // MPPT1 Current
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN1, 15);    // MPPT2 Current
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN2, 15);    // Battery Current

    // Set SOC interrupts. Enable the interrupts and make sure flags are cleared.
    ADC_setInterruptSource(ADCB_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);    // MPPT1 Current
    ADC_setInterruptSource(ADCB_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER1);    // MPPT2 Current
    ADC_setInterruptSource(ADCB_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER2);    // Battery Voltage and Input Current

    ADC_enableInterrupt(ADCB_BASE, ADC_INT_NUMBER1);
    ADC_enableInterrupt(ADCB_BASE, ADC_INT_NUMBER2);
    ADC_enableInterrupt(ADCB_BASE, ADC_INT_NUMBER3);

    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER2);
    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER3);
    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER4);
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

__interrupt void adc_mppt_one_v_irq(void) {
    mppt_one_result_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    mppt_one_result_v = adc_convert_to_v(mppt_one_result_mv);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER3);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

__interrupt void adc_mppt_one_i_irq(void) {
    uint16_t adc_result = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER0);
    mppt_one_result_i = adc_convert_to_v(adc_result) / MPPT_SHUNT_R;
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

__interrupt void adc_mppt_two_v_irq(void) {
    mppt_two_result_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
    mppt_two_result_v = adc_convert_to_v(mppt_two_result_mv);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

__interrupt void adc_mppt_two_i_irq(void) {
    uint16_t adc_result = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);
    mppt_two_result_i = adc_convert_to_v(adc_result) / MPPT_SHUNT_R;
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

/**
 * Will update both battery voltage and input current after conversion trigger
 *  from CPUTIMER1
 */
__interrupt void adc_battery_irq(void) {
    // battery voltage
    battery_result_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4);
    battery_result_v = adc_convert_to_v(battery_result_mv);

    // battery current
    uint16_t adc_result = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER3);
    mppt_two_result_i = adc_convert_to_v(adc_result) / BATTERY_IN_SHUNT_R;

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}
