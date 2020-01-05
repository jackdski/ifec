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

void init_adc(void);
float get_buck_one_v(void);

void adc_buck_one_irq(void);
void adc_buck_two_irq(void);
void adc_mppt_one_irq(void);
void adc_mppt_two_irq(void);


#endif /* INCLUDE_SRC_ADC_H_ */
