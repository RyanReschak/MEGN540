#include "Encoder.h"

/**
* Internal counters for the Interrupts to increment or decrement as necessary.
*/
static volatile bool _last_right_A;  // Static limits it's use to this file
static volatile bool _last_right_B;  // Static limits it's use to this file
static volatile bool _last_right_XOR; // Necessary to check if triggered the ISR or not

static volatile bool _last_left_A;   // Static limits it's use to this file
static volatile bool _last_left_B;   // Static limits it's use to this file
static volatile bool _last_left_XOR; // Necessary to check if PB4 triggered the ISR or not

static volatile int32_t _left_counts;   // Static limits it's use to this file
static volatile int32_t _right_counts;  // Static limits it's use to this file

/** Helper Funcions for Accessing Bit Information */
// *** MEGN540 Lab 3 TODO ***
// Hint, use avr's bit_is_set function to help
static inline bool Right_XOR() { return bit_is_set(PINE, PINE6); }//((PORTE & (1 << PORTE6)) >> PORTE6); } 
static inline bool Right_B()   { return bit_is_set(PINF, PINF0); }//((PORTF & (1 << PORTF0)) >> PORTF0); }
static inline bool Right_A()   { return Right_B() ^ Right_XOR(); }

static inline bool Left_XOR() { return bit_is_set(PINB, PINB4); }//((PORTB & (1 << PORTB4)) >> PORTB4); }
static inline bool Left_B()   { return bit_is_set(PINE, PINE2); }//((PORTE & (1 << PORTE2)) >> PORTE2); }
static inline bool Left_A()   { return Left_B() ^ Left_XOR(); }

/**
 * Function Encoders_Init initializes the encoders, sets up the pin change interrupts, and zeros the initial encoder
 * counts.
 */
void Encoders_Init()
{
    // *** MEGN540 Lab3 ***
    // YOUR CODE HERE

    // Left encoder uses PB4 and PE2 pins as digital inputs. External interrupt PCINT4 is necessary to detect
    // the change in XOR flag. You'll need to see Section 11.1.5 - 11.1.7 for setup and use.
    // Note that the PCINT interrupt is trigered by any PCINT pin. In the ISR you should check to make sure
    // the interrupt triggered is the one you intend on processing.


    // Right encoder uses PE6 adn PF0 as digital inputs. External interrupt INT6 is necessary to detect
    // the changes in XOR flag. You'll need to see Sections 11.1.2-11.1.4 for setup and use.
    // You'll use the INT6_vect ISR flag.

    //First set Pin Values to correct direction/input
    clear_bit(DDRE, DDE6);
    clear_bit(DDRE, DDE2);
    clear_bit(DDRB, DDB4);
    clear_bit(DDRF, DDF0);

    //Enable Interrupts Right XOR
    set_bit(EICRB, ISC60);
    clear_bit(EICRB, ISC61);//Mode of the interrupt
    set_bit(EIMSK, INT6);  //Enables interrupts for INT6
    //Enable Interrupts Left XOR
    set_bit(PCICR, PCIE0);
    set_bit(PCMSK0, PCINT4);

    //Set Last Left and Set Last Right  
    _last_right_B = Right_B();
    _last_right_XOR = Right_XOR();
    _last_right_A = Right_A();
     
    _last_left_B = Left_B();  
    _last_left_XOR = Left_XOR();  
    _last_left_A = Left_A(); 

    _left_counts = 0;  
    _right_counts = 0;  
}


/**
 * Function Counts_Left returns the number of counts from the left encoder.
 * @return [int32_t] The count number.
 */
int32_t Counts_Left()
{
    // *** MEGN540 Lab3 ***
    // YOUR CODE HERE
    // Note: Interrupts can trigger during a function call and an int32 requires
    // multiple clock cycles to read/save. You may want to stop interrupts, copy the value,
    // and re-enable interrupts to prevent this from corrupting your read/write.
    clear_bit(PCMSK0, PCINT4);
    int32_t temp =_left_counts; 
    set_bit(PCMSK0, PCINT4);

    return temp;
}

/**
 * Function Counts_Right returns the number of counts from the right encoder.
 * @return [int32_t] The count number.
 */
int32_t Counts_Right()
{
    // *** MEGN540 Lab3 ***
    // YOUR CODE HERE
    // Note: Interrupts can trigger during a function call and an int32 requires
    // multiple clock cycles to read/save. You may want to stop interrupts, copy the value,
    // and re-enable interrupts to prevent this from corrupting your read/write.
    clear_bit(EIMSK, INT6);
    int32_t temp =  _right_counts; 
    set_bit(EIMSK, INT6);
    return temp;
}

/**
 * Function Rad_Left returns the number of radians for the left encoder.
 * @return [float] Encoder angle in radians
 */
float Rad_Left()
{
    // *** MEGN540 Lab3 ***
    // YOUR CODE HERE.  How many counts per rotation???
    return 2*3.1415/(75.0f*12.0f)*(float)(Counts_Left());
}

/**
 * Function Rad_Right returns the number of radians for the left encoder.
 * @return [float] Encoder angle in radians
 */
float Rad_Right()
{
    // *** MEGN540 Lab3 ***
    // YOUR CODE HERE.  How many counts per rotation???
    return 2*3.1415/(75.0f*12.0f)*(float)(Counts_Right());
}

/**
 * Interrupt Service Routine for the left Encoder. Note: May need to check that it is actually PCINT4 that triggered, as
 * the Pin Change Interrupts can trigger for multiple pins.
 * @return
 */
ISR(PCINT0_vect)
{
    if (_last_left_XOR != Left_XOR()){
        _left_counts += (Left_A() ^ _last_left_B)-(Left_B() ^ _last_left_A) ; //CCW - CW
        _last_left_B = Left_B();
        _last_left_A = Left_A();
        _last_left_XOR = Left_XOR();
    }
}


/**
 * Interrupt Service Routine for the right Encoder.
 * @return
 */
ISR(INT6_vect)
{
    _right_counts += (Right_A() ^ _last_right_B)-(Right_B() ^ _last_right_A) ; //CCW - CW
    _last_right_B = Right_B();
    _last_right_A = Right_A();
    _last_right_XOR = Right_XOR();
}