/*
 * src_adc.h
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#ifndef INCLUDE_SRC_ADC_H_
#define INCLUDE_SRC_ADC_H_

#define ADC_MAX_VALUE       4095    // 12-bit
#define ADC_MAX_VALUE_F     4095.0  // 12-bit floating point
#define VREF_MV             3300U   // 3.3V in millivolts
#define VREF_MV_F           3.3    // 3.3V

#define VREFHI_V            3.3     // 3.3V
#define MPPT_SHUNT_R        0.1     // 100mOhms
#define BATTERY_IN_SHUNT_R  0.1     // 100mOhms

#include <stdint.h>

void init_adc();

/***    G E T S    ***/
float get_buck_v(uint32_t buck_base);
float get_mppt_v(uint32_t mppt_base);
float get_mppt_i(uint32_t mppt_base);
float get_battery_v(void);
float get_battery_i(void);

/***    C O N V E R S I O N S   ***/
uint32_t adc_convert_to_mv(uint32_t adc_result);
float adc_convert_to_v(uint32_t adc_result);

/***    I N T E R R U P T S    ***/
__interrupt void adc_buck_one_irq(void);
__interrupt void adc_buck_two_irq(void);
__interrupt void adc_mppt_one_v_irq(void);
__interrupt void adc_mppt_one_i_irq(void);
__interrupt void adc_mppt_two_v_irq(void);
__interrupt void adc_mppt_two_i_irq(void);
__interrupt void adc_battery_v_irq(void);
__interrupt void adc_battery_i_irq(void);


#endif /* INCLUDE_SRC_ADC_H_ */
