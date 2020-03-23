/*
 * src_epwm.c
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

/**
 * Add in code to make sure 0% duty cycle doesn't keep low-side MOSFET high
 */

#include <stdint.h>

#include "src_epwm.h"
#include "driverlib.h"
#include "device.h"

/**********************************************************
 *          P R I V A T E   V A R I A B L E S
 **********************************************************/

static float epwm1_duty_cycle = 0.0;
static float epwm2_duty_cycle = 0.0;
static float epwm3_duty_cycle;
static float epwm4_duty_cycle = 0.0;


// initEPWMGPIO - Configure ePWM GPIO
void initEPWMGPIO(void) {
    // Disable pull up on GPIO 0 and GPIO 2 and configure them as PWM1A and PWM2A output respectively.

    /* 3V3 Buck - EPWM1 */
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(GPIO_0_EPWM1A);       // LaunchPad pin 80
    GPIO_setPinConfig(GPIO_1_EPWM1B);       // LaunchPad pin 79

    /* 5V Buck - EPWM2 */
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(GPIO_2_EPWM2A);       // LaunchPad pin 76
    GPIO_setPinConfig(GPIO_3_EPWM2B);       // LaunchPad pin 75

    /* MPPT1_BASE - EPWM3 */
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(GPIO_4_EPWM3A);       // LaunchPad pin 36
    GPIO_setPinConfig(GPIO_5_EPWM3B);       // LaunchPad pin 35

    /* MPPT2_BASE - EPWM4 */
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(GPIO_6_EPWM4A);       // LaunchPad pin 78
    GPIO_setPinConfig(GPIO_7_EPWM4B);       // LaunchPad pin 77
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

    // Dead Band
    EPWM_setDeadBandCounterClock(EPWM1_BASE, EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);
    EPWM_setRisingEdgeDeadBandDelayInput(EPWM1_BASE, EPWM_DB_INPUT_EPWMA);

    HRPWM_setDeadbandMEPEdgeSelect(EPWM1_BASE, HRPWM_DB_MEP_CTRL_RED_FED);
    HRPWM_setRisingEdgeDelayLoadMode(EPWM1_BASE, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);
    HRPWM_setFallingEdgeDelayLoadMode(EPWM1_BASE, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);

    EPWM_setDeadBandOutputSwapMode(EPWM1_BASE, EPWM_DB_OUTPUT_A, true);
    EPWM_setDeadBandDelayMode(EPWM1_BASE, EPWM_DB_FED, true);

    EPWM_setDeadBandDelayPolarity(EPWM1_BASE, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_HIGH);

    EPWM_setDeadBandCounterClock(EPWM1_BASE, EPWM_DB_COUNTER_CLOCK_HALF_CYCLE);
    EPWM_setRisingEdgeDelayCount(EPWM1_BASE, 0x2000);
    EPWM_setFallingEdgeDelayCount(EPWM1_BASE, 0x2000);

    // Invert ePWMxA signal
    HRPWM_setChannelBOutputPath(EPWM1_BASE, HRPWM_OUTPUT_ON_B_INV_A);    // ePWMxB is inverse of ePWMxA

    // initialize PWM period
    HRPWM_setMEPStep(EPWM1_BASE, 55);
    HRPWM_setCounterCompareValue(EPWM1_BASE, HRPWM_COUNTER_COMPARE_A, 0);
    HRPWM_setTimeBasePeriod(EPWM1_BASE, 0x6D);


    EPWM_setTimeBasePeriod(EPWM1_BASE, PERIOD);
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 0);
    EDIS;

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);   // Enable sync and clock to PWM
    EPWM_setTimeBaseCounter(EPWM1_BASE, 0);
}


void initEPWM3(void) {
    EALLOW;

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM3);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_HRPWM);
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);  // Disable sync(Freeze clock to PWM as well)

    EPWM_setActionQualifierContSWForceShadowMode(EPWM3_BASE, EPWM_AQ_SW_IMMEDIATE_LOAD);
    EPWM_setPhaseShift(EPWM3_BASE, 0);

    EPWM_setTimeBaseCounterMode(EPWM3_BASE, EPWM_COUNTER_MODE_UP);
    EPWM_disablePhaseShiftLoad(EPWM3_BASE);
    EPWM_setSyncOutPulseMode(EPWM3_BASE, EPWM_SYNC_OUT_PULSE_DISABLED);
    EPWM_setClockPrescaler(EPWM3_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
    EPWM_setEmulationMode(EPWM3_BASE, EPWM_EMULATION_FREE_RUN);

    EPWM_setCounterCompareShadowLoadMode(EPWM3_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);

    EPWM_setActionQualifierAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
    EPWM_setActionQualifierAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    HRPWM_setMEPEdgeSelect(EPWM3_BASE, HRPWM_CHANNEL_A, HRPWM_MEP_CTRL_FALLING_EDGE);
    HRPWM_setMEPControlMode(EPWM3_BASE, HRPWM_CHANNEL_A, HRPWM_MEP_DUTY_PERIOD_CTRL);
    HRPWM_setCounterCompareShadowLoadEvent(EPWM3_BASE, HRPWM_CHANNEL_A, HRPWM_LOAD_ON_CNTR_ZERO);
    HRPWM_disableAutoConversion(EPWM3_BASE);
    HRPWM_disablePeriodControl(EPWM3_BASE);


    HRPWM_setMEPStep(EPWM3_BASE, 55);
    HRPWM_setCounterCompareValue(EPWM3_BASE, HRPWM_COUNTER_COMPARE_A, 0);
    HRPWM_setTimeBasePeriod(EPWM3_BASE, 0x6D);


    EPWM_setTimeBasePeriod(EPWM3_BASE, PERIOD);
    EPWM_setCounterCompareValue(EPWM3_BASE, EPWM_COUNTER_COMPARE_A, 0);
    EDIS;

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);   // Enable sync and clock to PWM
    EPWM_setTimeBaseCounter(EPWM3_BASE, 0);
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

    EALLOW;

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1);
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

    // Dead Band
    EPWM_setDeadBandCounterClock(epwm_base, EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);
    EPWM_setRisingEdgeDeadBandDelayInput(epwm_base, EPWM_DB_INPUT_EPWMA);

    HRPWM_setDeadbandMEPEdgeSelect(epwm_base, HRPWM_DB_MEP_CTRL_RED_FED);
    HRPWM_setRisingEdgeDelayLoadMode(epwm_base, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);
    HRPWM_setFallingEdgeDelayLoadMode(epwm_base, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);

    EPWM_setDeadBandOutputSwapMode(epwm_base, EPWM_DB_OUTPUT_A, true);
    EPWM_setDeadBandDelayMode(epwm_base, EPWM_DB_FED, true);

    EPWM_setDeadBandDelayPolarity(epwm_base, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_HIGH);

    EPWM_setDeadBandCounterClock(epwm_base, EPWM_DB_COUNTER_CLOCK_HALF_CYCLE);
    EPWM_setRisingEdgeDelayCount(epwm_base, 0x2000);
    EPWM_setFallingEdgeDelayCount(epwm_base, 0x2000);

    // Invert ePWMxA signal
    HRPWM_setChannelBOutputPath(epwm_base, HRPWM_OUTPUT_ON_B_INV_A);    // ePWMxB is inverse of ePWMxA

    // initialize PWM period
    HRPWM_setMEPStep(epwm_base, 55);
    HRPWM_setCounterCompareValue(epwm_base, HRPWM_COUNTER_COMPARE_A, 0);
    HRPWM_setTimeBasePeriod(epwm_base, 0x6D);


    EPWM_setTimeBasePeriod(epwm_base, PERIOD);
    EPWM_setCounterCompareValue(epwm_base, EPWM_COUNTER_COMPARE_A, 0);
    EDIS;

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);   // Enable sync and clock to PWM
    EPWM_setTimeBaseCounter(epwm_base, 0);
}

void change_pwm_duty_cycle(uint32_t epwm_base, float dc) {
//    float dc_new;
    uint32_t new_dc = 0;
    // add in 0-100 check here
    if(dc < 0.0) {
//        new_dc = 0;
        dc = 0.0;
    }
    else if(dc > 90.0) {
//        new_dc = 90.0;
        dc = 90.0;
    }

    switch(epwm_base) {
        case(EPWM1_BASE): epwm1_duty_cycle = dc; break;
        case(EPWM2_BASE): epwm2_duty_cycle = dc; break;
        case(EPWM3_BASE): epwm3_duty_cycle = dc; break;
        case(EPWM4_BASE): epwm4_duty_cycle = dc; break;
    }

    new_dc = (uint32_t)(((dc * PERIOD)/ 100.0) * 256.0);
    HRPWM_setCounterCompareValue(epwm_base, HRPWM_COUNTER_COMPARE_A, new_dc);
//    HRPWM_setCounterCompareValue(epwm_base, HRPWM_COUNTER_COMPARE_A, ((303 << 8) | 5700));
    //    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, (dc_integer*PERIOD) / 100);
//    EALLOW;
//    EDIS;
}

/**
 * @brief Returns the current duty cycle of an ePWM module
 *
 * @param epwm_base The base address of an ePWM module
 * @return Float with the duty cycle of the ePWM module specified. Returns
 *      -1.0 if the ePWM module specified is not in use or does not exist.
 */
float get_duty_cycle(uint32_t epwm_base) {
    switch(epwm_base) {
        case(EPWM1_BASE): return epwm1_duty_cycle;
        case(EPWM2_BASE): return epwm2_duty_cycle;
        case(EPWM3_BASE): return epwm3_duty_cycle;
        case(EPWM4_BASE): return epwm4_duty_cycle;
        default: return -1.0;  // for ePWM modules not used
    }
}
