#include "Battery_Monitor.h"

static const float BITS_TO_BATTERY_VOLTS = 11.2f/1024.0f;

/**
 * Function Battery_Monitor_Init initializes the Battery Monitor to record the current battery voltages.
 */
void Battery_Monitor_Init()
{

	clear_bit(DDRF, DDF6);
    clear_bit(ADMUX, REFS1);
    set_bit(ADMUX, REFS0);

    //MUX5 - MUX0 => ADC6
    clear_bit(ADCSRB, MUX5);
    clear_bit(ADMUX, MUX4);
    clear_bit(ADMUX, MUX3);
    set_bit(ADMUX, MUX2);
    set_bit(ADMUX, MUX1);
    clear_bit(ADMUX, MUX0);

    set_bit(ADCSRA, ADEN);
    clear_bit(ADCSRA, ADSC);
    clear_bit(ADCSRA, ADIE);
    clear_bit(ADCSRA, ADATE);
    //Pre-scalor of 128
    set_bit(ADCSRA, ADPS2);
    set_bit(ADCSRA, ADPS1);
    set_bit(ADCSRA, ADPS0);

    ADCSRB = 0;
    
    //Butterworth Filter
    uint8_t order = 4;
    float numerator_coeffs[5] = {0.0048, 0.0193, 0.0289, 0.0193, 0.0048};
    float denominator_coeffs[5] = {1, -2.3695, 2.314, -1.0547, 0.1874};
    
    Filter_Init ( &bat_filter, numerator_coeffs, denominator_coeffs, order );

    Filter_SetTo( &bat_filter, 0.0f);
    
}

/**
 * Function Battery_Voltage initiates the A/D measurement and returns the result for the battery voltage.
 */
float Battery_Voltage()
{
    // A Union to assist with reading the LSB and MSB in the  16 bit register
    union { struct {uint8_t LSB; uint8_t MSB; } split; uint16_t value;} data;

    set_bit(ADCSRA, ADSC);
    while(bit_is_set(ADCSRA, ADSC));
    data.split.LSB = ADCL;
    data.split.MSB = ADCH;

    return Filter_Value( &bat_filter, data.value * BITS_TO_BATTERY_VOLTS );

}
