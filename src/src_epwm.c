/*
 * src_epwm.c
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#include <stdint.h>

#include "src_epwm.h"
#include "driverlib.h"
#include "device.h"

static float duty_cycle = 0;

// initEPWMGPIO - Configure ePWM GPIO
void initEPWMGPIO(void) {
    // Disable pull up on GPIO 0 and GPIO 2 and configure them as PWM1A and PWM2A output respectively.
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(GPIO_0_EPWM1A);       // LaunchPad pin 80

    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(GPIO_2_EPWM2A);
}

// initEPWM1 - Configure ePWM1
void initEPWM1(void) {
    EALLOW;

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_HRPWM);
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);  // Disable sync(Freeze clock to PWM as well)

    EPWM_setActionQualifierContSWForceShadowMode(EPWM1_BASE, EPWM_AQ_SW_IMMEDIATE_LOAD);
    EPWM_setPhaseShift(EPWM1_BASE, 0);

    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);
    EPWM_disablePhaseShiftLoad(EPWM1_BASE);
    EPWM_setSyncOutPulseMode(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_DISABLED);
    EPWM_setClockPrescaler(EPWM1_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
    EPWM_setEmulationMode(EPWM1_BASE, EPWM_EMULATION_FREE_RUN);

    EPWM_setCounterCompareShadowLoadMode(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);

    EPWM_setActionQualifierAction(EPWM1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
    EPWM_setActionQualifierAction(EPWM1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    HRPWM_setMEPEdgeSelect(EPWM1_BASE, HRPWM_CHANNEL_A, HRPWM_MEP_CTRL_FALLING_EDGE);
    HRPWM_setMEPControlMode(EPWM1_BASE, HRPWM_CHANNEL_A, HRPWM_MEP_DUTY_PERIOD_CTRL);
    HRPWM_setCounterCompareShadowLoadEvent(EPWM1_BASE, HRPWM_CHANNEL_A, HRPWM_LOAD_ON_CNTR_ZERO);
    HRPWM_disableAutoConversion(EPWM1_BASE);
    HRPWM_disablePeriodControl(EPWM1_BASE);


    HRPWM_setMEPStep(EPWM1_BASE, 55);
    HRPWM_setCounterCompareValue(EPWM1_BASE, HRPWM_COUNTER_COMPARE_A, 55);
    HRPWM_setTimeBasePeriod(EPWM1_BASE, 0x6D);


    EPWM_setTimeBasePeriod(EPWM1_BASE, PERIOD);
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, (0.5*PERIOD));
    EDIS;

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);   // Enable sync and clock to PWM
    EPWM_setTimeBaseCounter(EPWM1_BASE, 0);
}

void initEPWM(uint32_t epwm_base) {
    EALLOW;

    // enable correct clock
    switch(epwm_base) {
        case(EPWM1_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1); break;
        case(EPWM2_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM2); break;
        case(EPWM3_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM3); break;
        case(EPWM4_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM4); break;
        case(EPWM5_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM5); break;
        case(EPWM6_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM6); break;
        case(EPWM7_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM7); break;
        case(EPWM8_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM8); break;
    }

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_HRPWM);
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);  // Disable sync(Freeze clock to PWM as well)

    EPWM_setActionQualifierContSWForceShadowMode(epwm_base, EPWM_AQ_SW_IMMEDIATE_LOAD);
    EPWM_setPhaseShift(epwm_base, 0);

    EPWM_setTimeBaseCounterMode(epwm_base, EPWM_COUNTER_MODE_UP);
    EPWM_disablePhaseShiftLoad(epwm_base);
    EPWM_setSyncOutPulseMode(epwm_base, EPWM_SYNC_OUT_PULSE_DISABLED);
    EPWM_setClockPrescaler(epwm_base, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
    EPWM_setEmulationMode(epwm_base, EPWM_EMULATION_FREE_RUN);

    EPWM_setCounterCompareShadowLoadMode(epwm_base, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);

    EPWM_setActionQualifierAction(epwm_base, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
    EPWM_setActionQualifierAction(epwm_base, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    HRPWM_setMEPEdgeSelect(epwm_base, HRPWM_CHANNEL_A, HRPWM_MEP_CTRL_FALLING_EDGE);
    HRPWM_setMEPControlMode(epwm_base, HRPWM_CHANNEL_A, HRPWM_MEP_DUTY_PERIOD_CTRL);
    HRPWM_setCounterCompareShadowLoadEvent(epwm_base, HRPWM_CHANNEL_A, HRPWM_LOAD_ON_CNTR_ZERO);
    HRPWM_disableAutoConversion(epwm_base);
    HRPWM_disablePeriodControl(epwm_base);


    HRPWM_setMEPStep(epwm_base, 55);
    HRPWM_setCounterCompareValue(epwm_base, HRPWM_COUNTER_COMPARE_A, 55);
    HRPWM_setTimeBasePeriod(epwm_base, 100);


    EPWM_setTimeBasePeriod(epwm_base, PERIOD);
    EPWM_setCounterCompareValue(epwm_base, EPWM_COUNTER_COMPARE_A, (0.5*PERIOD));
    EDIS;

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);   // Enable sync and clock to PWM
    EPWM_setTimeBaseCounter(epwm_base, 0);
}


void change_pwm_duty_cycle(float dc) {
    // add in 0-100 check here
    if(dc < 0.0 || dc > 100.0) {
        return;
    }
    uint16_t dc_integer = (int)(dc);
    uint16_t dc_fraction = (int)((dc - dc_integer) * 100);
    duty_cycle = dc;
    HRPWM_setCounterCompareValue(EPWM1_BASE, HRPWM_COUNTER_COMPARE_A, dc_fraction);
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, (dc_integer*PERIOD) / 100);
//    EALLOW;
//    EDIS;
}

float get_duty_cycle(void) {
    return duty_cycle;
}
