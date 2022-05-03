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

#include "../c_lib/SerialIO.h"
#include "../c_lib/Timing.h"
#include "../c_lib/MEGN540_MessageHandeling.h"
#include "../c_lib/Battery_Monitor.h"
#include "../c_lib/Encoder.h"
#include "../c_lib/MotorPWM.h"
/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
    SetupTimer0();         // initialize timer zero functionality
    USB_SetupHardware();   // initialize USB
    Message_Handling_Init();  //iniitlize Message handling
    GlobalInterruptEnable(); // Enable Global Interrupts for USB and Timer etc.
    Battery_Monitor_Init(); //initialize battery adc
    Encoders_Init();        //initalize encoder interrupts
	Motor_PWM_Init( 100 );
	usb_flush_input_buffer();

    float bat_volt = 0.0f;
    Time_t bat_time = GetTime();
    const float THRESH_LOW_VOLT = 4.0f; // 4 Batteries in series and lowest is 1.0v before damage
    for (;;)
    {
        USB_Upkeep_Task();
        
        if (0.2 < SecondsSince(&bat_time)) {
            bat_volt = Battery_Voltage();
            bat_time = GetTime();
            if(bat_volt <= THRESH_LOW_VOLT) {
                struct __attribute__((__packed__)) {char let[7]; float volt; } msg = {
                    .let = {'B', 'A', 'T', ' ', 'L', 'O', 'W'},
                    .volt = bat_volt
                };
                usb_send_msg("c7sf", '!', &msg, sizeof(msg));

            }
        }

        //flag cases
        if (MSG_FLAG_Execute(&mf_loop_timer)){
            struct __attribute__((__packed__)) { uint8_t c; float f; } data;
            data.c = 1;
            data.f = SecondsSince(&mf_loop_timer.last_trigger_time);
            usb_send_msg("ccf", 'T', &data, sizeof(data));
            if(mf_loop_timer.duration <= 0.0f) {
                mf_loop_timer.active = false;
            } else {
                mf_loop_timer.last_trigger_time = GetTime();
            }
        }
        if(MSG_FLAG_Execute(&mf_send_time)){
            struct __attribute__((__packed__)) { uint8_t c; float f; } data;
            data.c = 0;
            data.f = GetTimeSec();
            usb_send_msg("ccf", 'T', &data, sizeof(data));
            if(mf_send_time.duration <= 0.0f) {
                mf_send_time.active = false;
            }
            else {
                mf_send_time.last_trigger_time = GetTime();
            }
        }
        if(MSG_FLAG_Execute(&mf_battery)){
            
            usb_send_msg("cf", 'B', &bat_volt, sizeof(bat_volt));
            if(mf_battery.duration <= 0.0f){
                mf_battery.active = false;
            } else {
                mf_battery.last_trigger_time = GetTime();
            }
        }
        if(MSG_FLAG_Execute(&mf_encoder)){
            struct __attribute__((__packed__)) { float l; float r; } data;
            data.l = Rad_Left();
            data.r = Rad_Right();
            usb_send_msg("cff", 'E', &data, sizeof(data));
            if(mf_encoder.duration <= 0.0f){
                mf_encoder.active = false;
            } else {
                mf_encoder.last_trigger_time = GetTime();
            }
        }
		if(MSG_FLAG_Execute(&mf_motor_pwm)){
			if(bat_volt <= THRESH_LOW_VOLT) {
            	char msg[9] = {'P', 'O', 'W', 'E', 'R', ' ', 'O', 'F', 'F'};
                usb_send_msg("c9s", '!', &msg, sizeof(msg));
            } else if (Is_Motor_PWM_Enabled()){
				
				if (mf_motor_pwm.duration > 0.0f){
					
					Motor_PWM_Left(0);
					Motor_PWM_Right(0);
					Motor_PWM_Enable (false);
					mf_motor_pwm.active = false;
				}
			}
		}
		if(MSG_FLAG_Execute(&mf_motor_id)){
			struct __attribute__((__packed__)) { float time; int16_t pwm_l; int16_t pwm_r; float encoder_l; float encoder_r; } data;
            data.time = GetTimeSec();
			data.pwm_l = Get_Motor_PWM_Left();
			data.pwm_r = Get_Motor_PWM_Right();
			data.encoder_l = Rad_Left();
            data.encoder_r = Rad_Right();
            usb_send_msg("cf2h2f", 'Q', &data, sizeof(data));
            if(mf_motor_id.duration <= 0.0f){
                mf_motor_id.active = false;
            } else {
                mf_motor_id.last_trigger_time = GetTime();
            }
		}
        if(MSG_FLAG_Execute(&mf_time_out)) {
            usb_flush_input_buffer();
            mf_time_out.active = false;
        }
        if(MSG_FLAG_Execute(&mf_restart)){
            break;
        }
        
        Message_Handling_Task();
    }
    return 0;
}
