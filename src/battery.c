/*
 * battery.c
 *
 *  Created on: Jun 13, 2020
 *      Author: jack
 */

#include "battery.h"
#include "config.h"
#include "src_adc.h"


void init_battery(Battery_t * battery)
{
    battery->voltage = get_battery_v();
    battery->current = get_battery_i();
    battery->state = Supply;
    battery->charger.cc_cv = Charging_Inactive;
}

void update_battery(Battery_t * battery)
{
    battery->voltage = get_battery_v();
    battery->current = get_battery_i();
    determine_battery_state(battery);

    if(battery->state == Charge)
    {
        battery->charger.cc_cv = determine_cc_cv(*battery);
    }
    else
    {
        battery->charger.cc_cv = Charging_Inactive;
    }
}

eBatteryChargeType determine_cc_cv(Battery_t battery)
{
    if((battery.current <= I_BATTERY_MIN_LIMIT) && (battery.voltage >= V_BATTERY_MAX_LIMIT)) {
            return Battery_Full;
    }
    else if(battery.voltage < V_BATTERY_MAX_LIMIT)
    {
        return Continuous_Current;
    }
    else
    {
        return Continuous_Voltage;
    }
}

void determine_battery_state(Battery_t * battery)
{
    if(get_mppt_v(MPPT_ONE_ID) > battery->voltage)
    {
        battery->state = Charge;

        if(get_mppt_v(MPPT_ONE_ID) > get_mppt_v(MPPT_TWO_ID))
        {
            battery->charger.source = PV1;
        }
        else
        {
            battery->charger.source = PV2;
        }
    }
    else if(get_mppt_v(MPPT_TWO_ID) > battery->voltage)
    {
        battery->state = Charge;
        battery->charger.source = PV2;
    }
    else
    {
        battery->state = Supply;
        battery->charger.source = Not_Charging;
    }
}


