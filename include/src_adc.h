/*
 * src_adc.h
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#ifndef INCLUDE_SRC_ADC_H_
#define INCLUDE_SRC_ADC_H_

#include <stdint.h>
#include <stdbool.h>

#define MPPT_ADC_EVT_COUNT  6
#define MPPT_ONE_V_ADC_EVT  5
#define MPPT_ONE_I_ADC_EVT  4
#define MPPT_TWO_V_ADC_EVT  3
#define MPPT_TWO_I_ADC_EVT  2
#define MPPT_BATT_V_ADC_EVT 1
#define MPPT_BATT_I_ADC_EVT 0

#define ADC_MAX_VALUE       4095U   // 12-bit
#define ADC_MAX_VALUE_F     4095.0f // 12-bit floating point
#define VREF_MV             3300U   // 3.3V in millivolts
#define VREF_MV_F           3.3f    // 3.3V

#define VREFHI_V            3.3f    // 3.3V
#define MPPT_SHUNT_R        0.1f    // 100mOhms
#define BATTERY_IN_SHUNT_R  0.1f    // 100mOhms

void init_adc();

/***    G E T S    ***/
float get_buck_v(uint32_t buck_base);
float get_mppt_v(uint32_t mppt_base);
float get_mppt_i(uint32_t mppt_base);
float get_battery_v(void);
float get_battery_i(void);
bool is_mppt_adc_done(void);

/***    C O N V E R S I O N S   ***/
uint32_t adc_convert_to_mv(uint32_t adc_result);
float adc_convert_to_v(uint32_t adc_result);

/***    G E T S    ***/
float get_buck_v(uint32_t buck_base);
float get_mppt_v(uint32_t mppt_base);
float get_mppt_i(uint32_t mppt_base);
float get_battery_v(void);
float get_battery_i(void);

/***    I N T E R R U P T S    ***/

/*
 * ADCA1 - INT1.1           ADCB1 - INT1.2
 * ADCA2 - INT10.2          ADCB2 - INT10.6
 * ADCA3 - INT10.3          ADCB3 - INT10.7
 * ADCA4 - INT10.4          ADCB4 - INT10.8
 */
__interrupt void adc_buck_5V_irq(void);
__interrupt void adc_buck_3V3_irq(void);
__interrupt void adc_mppt_one_v_irq(void);
__interrupt void adc_mppt_one_i_irq(void);
__interrupt void adc_mppt_two_v_irq(void);
__interrupt void adc_mppt_two_i_irq(void);
__interrupt void adc_battery_v_irq(void);
__interrupt void adc_battery_i_irq(void);

#endif /* INCLUDE_SRC_ADC_H_ */
