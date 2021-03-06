/*
 * src_adc.c
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#include "src_adc.h"
#include "config.h"


#define MPPT_LIST_SIZE                  4
#define MPPT_ONE_VOLTAGE_LIST_INDEX     0
#define MPPT_ONE_CURRENT_LIST_INDEX     1
#define MPPT_TWO_VOLTAGE_LIST_INDEX     2
#define MPPT_TWO_CURRENT_LIST_INDEX     3

#define BUCK_LIST_SIZE                  2
#define BUCK_5V_VOLTAGE_LIST_INDEX      0
#define BUCK_3V3_VOLTAGE_LIST_INDEX     1

#define BATTERY_LIST_SIZE               2
#define BATTERY_VOLTAGE_LIST_INDEX      0
#define BATTERY_CURRENT_LIST_INDEX      1



bool mppt_adc_evts[MPPT_ADC_EVT_COUNT] = {0};

typedef enum {
    Voltage_Component,
    Current_Component
} eAdcComponentType;

typedef struct {
    uint32_t            base;
    uint32_t            resultBase;
    uint16_t            adcResult;
    uint32_t            millivolts;
    float               stepped_down_volts;
    float               volts;
    float               current;
    ADC_SOCNumber       socNumber;
    eAdcComponentType   component_type;
    uint32_t            r_one;
    uint32_t            r_two;
} adcListComponent_t;

static adcListComponent_t mppt_one_voltage = {
                                              ADCA_BASE, ADCARESULT_BASE,
                                              0, 0, 0.0, 0.0, 0.0,
                                              ADC_SOC_NUMBER0, Voltage_Component,
                                              V_PV_SENSE_R1, V_PV_SENSE_R2
                                             };

static adcListComponent_t mppt_one_current = {
                                              ADCA_BASE, ADCARESULT_BASE,
                                              0, 0, 0.0, 0.0, 0.0,
                                              ADC_SOC_NUMBER1, Current_Component,
                                              0, 0
                                             };


static adcListComponent_t mppt_two_voltage = {
                                              ADCA_BASE, ADCARESULT_BASE,
                                              0, 0, 0.0, 0.0, 0.0,
                                              ADC_SOC_NUMBER2, Voltage_Component,
                                              V_PV_SENSE_R1, V_PV_SENSE_R2
                                             };

static adcListComponent_t mppt_two_current = {
                                              ADCA_BASE, ADCARESULT_BASE,
                                              0, 0, 0.0, 0.0, 0.0,
                                              ADC_SOC_NUMBER3, Current_Component,
                                              0, 0
                                             };


static adcListComponent_t buck_5V_voltage  = {
                                              ADCA_BASE, ADCARESULT_BASE,
                                              0, 0, 0.0, 0.0, 0.0,
                                              ADC_SOC_NUMBER4, Voltage_Component,
                                              BUCK_5V_OUTPUT_R1, BUCK_5V_OUTPUT_R2
                                             };

static adcListComponent_t buck_3V3_voltage = {
                                              ADCA_BASE, ADCARESULT_BASE,
                                              0, 0, 0.0, 0.0, 0.0,
                                              ADC_SOC_NUMBER5, Voltage_Component,
                                              BUCK_3V3_OUTPUT_R1, BUCK_3V3_OUTPUT_R2
                                             };

static adcListComponent_t battery_voltage = {
                                             ADCA_BASE, ADCARESULT_BASE,
                                             0, 0, 0.0, 0.0, 0.0,
                                             ADC_SOC_NUMBER6, Voltage_Component,
                                             V_BATT_SENSE_R1, V_BATT_SENSE_R2
                                            };

static adcListComponent_t battery_current = {
                                             ADCA_BASE, ADCARESULT_BASE,
                                             0, 0, 0.0, 0.0, 0.0,
                                             ADC_SOC_NUMBER7, Current_Component,
                                             0, 0
                                            };


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
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN0, 15);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN1, 15);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN4, 15);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN3, 15);

    // Configure SOCs of ADCB
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN0, 15);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN1, 15);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN4, 15);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN3, 15);

    DEVICE_DELAY_US(1000);
}


/**********************************************************
 *                      G E T S
 **********************************************************/

float get_buck_v(uint32_t buck_id) {
    switch(buck_id) {
    case(BUCK_5V_ID): return buck_5V_voltage.volts;
    case(BUCK_3V3_ID): return buck_3V3_voltage.volts;
    }
    return -1.0;
}

float get_buck_stepped_down_v(uint32_t buck_id) {
    switch(buck_id) {
    case(BUCK_5V_ID): return buck_5V_voltage.stepped_down_volts;
    case(BUCK_3V3_ID): return buck_3V3_voltage.stepped_down_volts;
    }
    return -1.0;
}

float get_mppt_v(uint32_t mppt_id) {
    switch(mppt_id) {
    case(MPPT_ONE_ID): return mppt_one_voltage.volts;
    case(MPPT_TWO_ID): return mppt_two_voltage.volts;
    }
    return -1.0;
}

float get_mppt_stepped_down_v(uint32_t mppt_id) {
    switch(mppt_id) {
    case(MPPT_ONE_ID): return mppt_one_voltage.stepped_down_volts;
    case(MPPT_TWO_ID): return mppt_two_voltage.stepped_down_volts;
    }
    return -1.0;
}

float get_mppt_i(uint32_t mppt_id) {
    switch(mppt_id) {
    case(MPPT_ONE_ID): return mppt_one_current.current;
    case(MPPT_TWO_ID): return mppt_two_current.current;
    }
    return -1.0;
}

float get_battery_v(void) {
    return battery_voltage.volts;
}

float get_battery_stepped_down_v(void) {
    return battery_voltage.stepped_down_volts;
}

float get_battery_i(void) {
    return battery_current.current;
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


/*
 * @brief Updates the ADC component
 */
void update_conversion(adcListComponent_t * adcComponent) {
    ADC_forceSOC(adcComponent->base, adcComponent->socNumber);
    while(ADC_isBusy(adcComponent->base));
    adcComponent->adcResult = ADC_readResult(adcComponent->resultBase, adcComponent->socNumber);
    adcComponent->millivolts = adc_convert_to_mv(adcComponent->adcResult);
    adcComponent->stepped_down_volts = adc_convert_to_v(adcComponent->adcResult);

    if(adcComponent->component_type == Voltage_Component) {
        adcComponent->volts = VOLTAGE_UNDIVIDER(adcComponent->stepped_down_volts, adcComponent->r_one, adcComponent->r_two);
    }
    else if(adcComponent->component_type == Current_Component) {
        adcComponent->current = I_SENSED(adcComponent->stepped_down_volts);
    }
}


/**
 * @brief Updates the buck ADC list based on the latest ADC results available
 */
void update_output_buck_conversions(void) {
    update_conversion(&buck_5V_voltage);
    update_conversion(&buck_3V3_voltage);
}

/**
 * @brief Updates the MPPT ADC list based on the latest ADC results available
 */
void update_mppt_conversions(void) {
    // MPPT 1
    update_conversion(&mppt_one_voltage);
    update_conversion(&mppt_one_current);

    // MPPT 2
    update_conversion(&mppt_two_voltage);
    update_conversion(&mppt_two_current);
}

/**
 * @brief Updates the Battery ADC list based on the latest ADC results available
 */
void update_battery_conversions(void) {
    update_conversion(&battery_voltage);
    update_conversion(&battery_current);
}


/**********************************************************
 *                  I N T E R R U P T S
 **********************************************************/


///** 5V Buck **/
//__interrupt void adc_buck_5V_irq(void) {
//    buck_5V_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
//    buck_5V_stepped_down_v = adc_convert_to_v(buck_5V_mv);
//    buck_5V_v = VOLTAGE_UNDIVIDER(buck_5V_stepped_down_v, BUCK_5V_OUTPUT_R1, BUCK_5V_OUTPUT_R2);
//    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
//}
//
///** 3.3V Buck **/
//__interrupt void adc_buck_3V3_irq(void) {
//    buck_3V3_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);
//    buck_3V3_stepped_down_v = adc_convert_to_v(buck_3V3_mv);
//    buck_3V3_v = VOLTAGE_UNDIVIDER(buck_3V3_stepped_down_v, BUCK_3V3_OUTPUT_R1, BUCK_3V3_OUTPUT_R2);
//    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER2);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP10);
//}
//
///** MPPT 1 **/
//__interrupt void adc_mppt_one_v_irq(void) {
//    mppt_one_mv = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
//    mppt_one_stepped_down_v = adc_convert_to_v(mppt_one_mv);
//    mppt_one_v = VOLTAGE_UNDIVIDER(mppt_one_stepped_down_v, V_PV_SENSE_R1, V_PV_SENSE_R2);
//
//    mppt_adc_evts[MPPT_ONE_V_ADC_EVT] = true;
//
//    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER3);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP10);
//}
//
//__interrupt void adc_mppt_one_i_irq(void) {
//    uint16_t adc_result = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
//    float adc_v = adc_convert_to_v(adc_result);
//    mppt_one_i = I_SENSED(adc_v);
//
//    mppt_adc_evts[MPPT_ONE_I_ADC_EVT] = true;
//
//    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER4);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP10);
//}
//
///** MPPT 2 **/
//__interrupt void adc_mppt_two_v_irq(void) {
//    mppt_two_mv = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER0);
//    mppt_two_stepped_down_v = adc_convert_to_v(mppt_two_mv);
//    mppt_two_v = VOLTAGE_UNDIVIDER(mppt_two_stepped_down_v, V_PV_SENSE_R1, V_PV_SENSE_R2);
//
//    mppt_adc_evts[MPPT_TWO_V_ADC_EVT] = true;
//
//    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER1);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
//}
//
//__interrupt void adc_mppt_two_i_irq(void) {
//    uint16_t adc_result = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);
//    float adc_v = adc_convert_to_v(adc_result);
//    mppt_two_i = I_SENSED(adc_v);
//
//    mppt_adc_evts[MPPT_TWO_I_ADC_EVT] = true;
//
//    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER2);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP10);
//}
//
//
///** Battery **/
//__interrupt void adc_battery_v_irq(void) {
//    battery_result_mv = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER2);
//
//    // Apply voltage divider conversion
//    battery_stepped_down_v = adc_convert_to_v(battery_result_mv);
//    battery_v = VOLTAGE_UNDIVIDER(battery_stepped_down_v, V_BATT_SENSE_R1, V_BATT_SENSE_R2);
//
//    mppt_adc_evts[MPPT_BATT_V_ADC_EVT] = true;
//
//    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER3);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP10);
//}
//
//__interrupt void adc_battery_i_irq(void) {
//    uint16_t adc_result = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER3);
//    float adc_v = adc_convert_to_v(adc_result);
//    battery_i = I_SENSED(adc_v);
//
//    mppt_adc_evts[MPPT_BATT_I_ADC_EVT] = true;
//
//    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER4);
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP10);
//}
