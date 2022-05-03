#include "MotorPWM.h"

/**
 * Function MotorPWM_Init initializes the motor PWM on Timer 1 for PWM based voltage control of the motors.
 * The Motor PWM system shall initialize in the disabled state for safety reasons. You should specifically enable
 * Motor PWM outputs only as necessary.
 * @param [uint16_t] MAX_PWM is the maximum PWM value to use. This controls the PWM frequency.
 */
void Motor_PWM_Init( uint16_t MAX_PWM ) {
    //Setting PWM
    set_bit(DDRB, DDB5);
    set_bit(DDRB, DDB6);

    Motor_PWM_Enable(false);

    //PWM mode 
    clear_bit(TCCR1A, WGM11);
    clear_bit(TCCR1A, WGM10);
    set_bit(TCCR1B, WGM13);
    clear_bit(TCCR1B, WGM12);
    
    //Set PWM prescalor to 1024
    set_bit(TCCR1B, CS12);
    clear_bit(TCCR1B, CS11);
    set_bit(TCCR1B, CS10);

    Set_MAX_Motor_PWM(MAX_PWM);
    
    //Setting Direction Pins (output)
    set_bit(DDRB, DDB1);
    set_bit(DDRB, DDB2);

    //set Direction to backward
    set_bit(PORTB, PORTB1);
    set_bit(PORTB, PORTB2);
}

/**
 * Function MotorPWM_Enable enables or disables the motor PWM outputs.
 * @param [bool] enable (true set enable, false set disable)
 */
void Motor_PWM_Enable( bool enable ){
    if(enable){
        //Left motor compare on clear at TOP and decount 
        set_bit(TCCR1A, COM1B1);
        clear_bit(TCCR1A, COM1B0);

        //Right Motor
        set_bit(TCCR1A, COM1A1);
        clear_bit(TCCR1A, COM1A0);
    } else{
        //Left motor disonnect 
        clear_bit(TCCR1A, COM1B1);
        clear_bit(TCCR1A, COM1B0);

        //Right Motor
        clear_bit(TCCR1A, COM1A1);
        clear_bit(TCCR1A, COM1A0);
    }
}

/**
 * Function Is_Motor_PWM_Enabled returns if the motor PWM is enabled for output.
 * @param [bool] true if enabled, false if disabled
 */
bool Is_Motor_PWM_Enabled(){
    if (bit_is_set(TCCR1A, COM1B1) && bit_is_set(TCCR1A, COM1A1))
        return true;
    return false;
}

/**
 * Function Motor_PWM_Left sets the PWM duty cycle for the left motor.
 * @return [int32_t] The count number.
 */
void Motor_PWM_Left( int16_t pwm ){
    uint16_t pwm_now;

    if (pwm < 0) { //Backwards
        set_bit(PORTB, PORTB2);
        pwm_now = (uint16_t)(pwm*-1);
    } else {
        clear_bit(PORTB, PORTB2);
        pwm_now = (uint16_t)(pwm);
    }

    if (pwm_now > Get_MAX_Motor_PWM())
        return;

    OCR1B =  pwm_now;
}

/**
 * Function Motor_PWM_Right sets the PWM duty cycle for the right motor.
 * @return [int32_t] The count number.
 */
void Motor_PWM_Right( int16_t pwm ) {
    uint16_t pwm_now;

    if (pwm < 0) { //Backwards
        set_bit(PORTB, PORTB1);
        pwm_now = (uint16_t)(pwm*-1);
    } else {
        clear_bit(PORTB, PORTB1);
        pwm_now = (uint16_t)(pwm);
    }

    if (pwm_now > Get_MAX_Motor_PWM())
        return;

    OCR1A = pwm_now;
}

/**
 * Function Get_Motor_PWM_Left returns the current PWM duty cycle for the left motor. If disabled it returns what the
 * PWM duty cycle would be.
 * @return [int16_t] duty-cycle for the left motor's pwm
 */
int16_t Get_Motor_PWM_Left() {
    int16_t value = OCR1B;
    if (bit_is_set(PORTB, PORTB2))
        return value*-1;

    return value;
}

/**
 * Function Get_Motor_PWM_Right returns the current PWM duty cycle for the right motor. If disabled it returns what the
 * PWM duty cycle would be.
 * @return [int16_t] duty-cycle for the right motor's pwm
 */
int16_t Get_Motor_PWM_Right() {
    int16_t value = OCR1A;
    if (bit_is_set(PORTB, PORTB1))
        return value*-1;

    return value;
}

/**
 * Function Get_MAX_Motor_PWM() returns the PWM count that corresponds to 100 percent duty cycle (all on), this is the
 * same as the value written into ICR1 as (TOP).
 */
uint16_t Get_MAX_Motor_PWM() {
    data.split.LSB = ICR1L;
    data.split.MSB = ICR1H;

    return data.value;
}

/**
 * Function Set_MAX_Motor_PWM sets the maximum pwm count. This function sets the timer counts to zero because
 * the ICR1 can cause undesired behaviors if change dynamically below the current counts.  See page 128 of the
 * atmega32U4 datasheat.
 */
void Set_MAX_Motor_PWM( uint16_t MAX_PWM ) {
    data.value = MAX_PWM;

    ICR1L = data.split.LSB;
    ICR1H = data.split.MSB;

    TCNT1H = 0;
    TCNT1L = 0;
}