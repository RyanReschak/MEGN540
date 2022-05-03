/*
         MEGN540 Mechatronics Lab
    Copyright (C) Andrew Petruska, 2021.
       apetruska [at] mines [dot] edu
          www.mechanical.mines.edu
*/

/*
    Copyright (c) 2021 Andrew Petruska at Colorado School of Mines

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/

#include "../c_lib/Timing.h"


/** These define the internal counters that will be updated in the ISR to keep track of the time
 *  The volatile keyword is because they are changing in an ISR, the static means they are not
 *  visible (not global) outside of this file.
 */
static volatile uint32_t _count_ms = 0;

/**
 * Function SetupTimer0 initializes Timer0 to have a prescalar of XX and initializes the compare
 * feature for use in an ISR.  It also enables ISR's.
 *
 *  FCPU = 16 000 000 Hz,  1 us => 1 000 000 Hz,  A prescalar of 16 would give you millisecond resolution, but you dont
 *  have enough time to do those additions every microsecond, in addition the 8-bit timer will roll over every 256 microseconds.
 *  So, you need to be counting in (effectively) 4us increments.  So use a prescalar of 64. This will give you a resolution of 4us per
 *  Timer0 Bit and enough head-room to compare at with 249 for a 1kHz update.
 *
 *  Since Timer 0 will be triggering at a kHz, we may want other things to be updated at 1kHz too.
 *
 */
void SetupTimer0()
{
    // *** MEGN540 Lab 2 ***
    // Enable timing, setup prescalers, etc.

    //CTC and Normal Operation
    TCCR0A = 0;
    TCCR0A |= (1 << WGM01);

    //Setting Prescaler and not Force Compare
    TCCR0B = 0;
    TCCR0B |= (0 << CS02) | (1 << CS01) | (1 << CS00); //011 = 64 

    //Timer Counter = 0
    TCNT0 = 0;

    //Sets what to compare
    OCR0A = 249;
    OCR0B = 249;

    //Sets CompA Interrupt Enable
    TIMSK0 = 0;
    set_bit(TIMSK0, OCIE0A);
    //TIMSK0 = (1 << );

    //Enables Interrupts
    sei();
 
    _count_ms= 0;
    ms_counter_1 = 0;
    ms_counter_2 = 0;
    ms_counter_3 = 0;
    ms_counter_4 = 0;
}

/**
 * This function gets the current time and returns it in a Time_t structure.
 * @return
 */
float  GetTimeSec()
{
    Time_t time = GetTime();
    return (float)(time.millisec)/1000 + (float)(time.microsec)/1000000;
}
Time_t GetTime()
{
    // *** MEGN540 Lab 2 ***
    uint32_t t = 0;
    clear_bit(TIMSK0, OCIE0A);
    t = _count_ms;
    set_bit(TIMSK0, OCIE0A);


    Time_t time ={
                    .millisec = t,
                    .microsec = TCNT0*4
                 };

    return time;
}

/**
 * These functions return the individual parts of the Time_t struct, useful if you only care about
 * things on second or millisecond resolution.
 * @return
 */
uint32_t GetMilli()
{
    return _count_ms;
}
uint16_t GetMicro()
{
    // *** MEGN540 Lab 2 ***
    return TCNT0*4;
}


/**
 * This function takes a start time and calculates the time since that time, it returns it in the Time struct.
 * @param p_time_start a pointer to a start time struct
 * @return (Time_t) Time since the other time.
 */
float  SecondsSince(const Time_t* time_start_p )
{
    // *** MEGN540 Lab 2 ***
    Time_t time_now = GetTime();
    float delta_time = (float)(time_now.millisec - time_start_p->millisec)/1000 + (float)(time_now.microsec - time_start_p->microsec)/1000000;
    return delta_time;
}

/** This is the Interrupt Service Routine for the Timer0 Compare A feature.
 * You'll need to set the compare flags properly for it to work.
 */
ISR(TIMER0_COMPA_vect)
{
    // *** MEGN540 Lab 2 ***
    // YOU NEED TO RESET THE Timer0 Value to 0 again!
    TCNT0 = 0;
    // take care of upticks of both our internal and external variables.
    _count_ms ++;

    ms_counter_1 ++;
    ms_counter_2 ++;
    ms_counter_3 ++;
    ms_counter_4 ++;
}
