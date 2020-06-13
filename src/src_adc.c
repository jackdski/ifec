/*
 * src_adc.c
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#include "src_adc.h"
#include "config.h"

/** Bucks */
static float buck_5V_v;
static float buck_5V_stepped_down_v;
static float buck_3V3_v;
static float buck_3V3_stepped_down_v;
static uint16_t buck_5V_mv;
static uint16_t buck_3V3_mv;

/** MPPT */
static float mppt_one_v;
static float mppt_one_stepped_down_v;
static uint16_t mppt_one_mv;

static float mppt_two_v;
static float mppt_two_stepped_down_v;
static uint16_t mppt_two_mv;

static float mppt_one_i;
static float mppt_two_i;

/** Battery */
static float battery_v;
static float battery_stepped_down_v;
static float battery_i;
static uint16_t battery_result_mv;


void init_adc(void) {
    // Setup VREF
    ADC_setVREF(ADCA_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
    ADC_setVREF(ADCB_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);

    // Set ADCCLK Divider to /4
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);
    ADC_setPrescaler(ADCB_BASE, ADC_CLK_DIV_4_0);

    // set pulse positions to late
    ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV);
    ADC_setInterruptPulseMode(ADCB_BASE, ADC_PULSE_END_OF_CONV);

    // enable ADCA
    ADC_enableConverter(ADCA_BASE);
    ADC_enableConverter(ADCB_BASE);
    DEVICE_DELAY_US(1000);

    // Configure SOCs of ADCA
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN0, 15);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN1, 15);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN4, 15);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN3, 15);

    // Configure SOCs of ADCB
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_CPU1_TINT1, ADC_CH_ADCIN0, 15);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN1, 15);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN4, 15);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN3, 15);


    // Set interrupt sources
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER1);
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER2);
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER4, ADC_SOC_NUMBER3);

    ADC_setInterruptSource(ADCB_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
    ADC_setInterruptSource(ADCB_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER1);
    ADC_setInterruptSource(ADCB_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER2);
    ADC_setInterruptSource(ADCB_BASE, ADC_INT_NUMBER4, ADC_SOC_NUMBER3);

    // Enable interrupts
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER2);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER3);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER4);

    ADC_enableInterrupt(ADCB_BASE, ADC_INT_NUMBER1);
    ADC_enableInterrupt(ADCB_BASE, ADC_INT_NUMBER2);
    ADC_enableInterrupt(ADCB_BASE, ADC_INT_NUMBER3);
    ADC_enableInterrupt(ADCB_BASE, ADC_INT_NUMBER4);

    // Clear interrupt flags
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER2);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER3);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);

    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER2);
    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER3);
    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER4);

    // Register Interrupts
    Interrupt_register(INT_ADCA1, &adc_buck_5V_irq);
    Interrupt_register(INT_ADCA2, &adc_buck_3V3_irq);
    Interrupt_register(INT_ADCA3, &adc_mppt_one_v_irq);
    Interrupt_register(INT_ADCA4, &adc_mppt_one_i_irq);

    Interrupt_register(INT_ADCB1, &adc_mppt_two_v_irq);
    Interrupt_register(INT_ADCB2, &adc_mppt_two_i_irq);
    Interrupt_register(INT_ADCB3, &adc_battery_v_irq);
    Interrupt_register(INT_ADCB4, &adc_battery_i_irq);
}


/**********************************************************
 *                      G E T S
 **********************************************************/

float get_buck_v(uint32_t buck_id) {
    switch(buck_id) {
    case(BUCK_5V_ID): return buck_5V_v;
    case(BUCK_3V3_ID): return buck_3V3_v;
    }
    return -1.0;
}

float get_buck_stepped_down_v(uint32_t buck_id) {
    switch(buck_id) {
    case(BUCK_5V_ID): return buck_5V_stepped_down_v;
    case(BUCK_3V3_ID): return buck_3V3_stepped_down_v;
    }
    return -1.0;
}

float get_mppt_v(uint32_t mppt_id) {
    switch(mppt_id) {
    case(MPPT_ONE_ID): return mppt_one_v;
    case(MPPT_TWO_ID): return mppt_two_v;
    }
    return -1.0;
}

float get_mppt_stepped_down_v(uint32_t mppt_id) {
    switch(mppt_id) {
    case(MPPT_ONE_ID): return mppt_one_stepped_down_v;
    case(MPPT_TWO_ID): return mppt_two_stepped_down_v;
    }
    return -1.0;
}

float get_mppt_i(uint32_t mppt_id) {
    switch(mppt_id) {
    case(MPPT_ONE_ID): return mppt_one_i;
    case(MPPT_TWO_ID): return mppt_two_i;
    }
    return -1.0;
}

float get_battery_v(void) {
    return battery_v;
}

float get_battery_stepped_down_v(void) {
    return battery_stepped_down_v;
}

float get_battery_i(void) {
    return battery_i;
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

/** 5V Buck **/
__interrupt void adc_buck_5V_irq(void) {
    buck_5V_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
    buck_5V_stepped_down_v = adc_convert_to_v(buck_5V_mv);
    buck_5V_v = VOLTAGE_UNDIVIDER(buck_5V_stepped_down_v, BUCK_5V_OUTPUT_R1, BUCK_5V_OUTPUT_R2);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

/** 3.3V Buck **/
__interrupt void adc_buck_3V3_irq(void) {
    buck_3V3_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);
    buck_3V3_stepped_down_v = adc_convert_to_v(buck_3V3_mv);
    buck_3V3_v = VOLTAGE_UNDIVIDER(buck_3V3_stepped_down_v, BUCK_3V3_OUTPUT_R1, BUCK_3V3_OUTPUT_R2);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER2);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

/** MPPT 1 **/
__interrupt void adc_mppt_one_v_irq(void) {
    mppt_one_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    mppt_one_stepped_down_v = adc_convert_to_v(mppt_one_mv);
    mppt_one_v = VOLTAGE_UNDIVIDER(mppt_one_stepped_down_v, V_PV_SENSE_R1, V_PV_SENSE_R2);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER3);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP10);
    Interrupt_clearACKGroup(0xFFF);
}

__interrupt void adc_mppt_one_i_irq(void) {
    uint16_t adc_result = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
    float adc_v = adc_convert_to_v(adc_result);
    mppt_one_i = I_SENSED(adc_v);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
    Interrupt_clearACKGroup(0xFFF);
}

/** MPPT 2 **/
__interrupt void adc_mppt_two_v_irq(void) {
    mppt_two_mv = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER0);
    mppt_two_stepped_down_v = adc_convert_to_v(mppt_two_mv);
    mppt_two_v = VOLTAGE_UNDIVIDER(mppt_two_stepped_down_v, V_PV_SENSE_R1, V_PV_SENSE_R2);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP10);
    Interrupt_clearACKGroup(0xFFF);
}

__interrupt void adc_mppt_two_i_irq(void) {
    uint16_t adc_result = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);
    float adc_v = adc_convert_to_v(adc_result);
    mppt_two_i = I_SENSED(adc_v);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER2);
    Interrupt_clearACKGroup(0xFFF);
}


/** Battery **/
__interrupt void adc_battery_v_irq(void) {
    battery_result_mv = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER2);

    // Apply voltage divider conversion
    battery_stepped_down_v = adc_convert_to_v(battery_result_mv);
    battery_v = VOLTAGE_UNDIVIDER(battery_stepped_down_v, V_BATT_SENSE_R1, V_BATT_SENSE_R2);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER3);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
    Interrupt_clearACKGroup(0xFFF);
}

__interrupt void adc_battery_i_irq(void) {
    uint16_t adc_result = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER3);
    float adc_v = adc_convert_to_v(adc_result);
    battery_i = I_SENSED(adc_v);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
    Interrupt_clearACKGroup(0xFFF);
}
