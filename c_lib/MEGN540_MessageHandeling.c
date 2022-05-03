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

#include "MEGN540_MessageHandeling.h"


static inline void MSG_FLAG_Init(MSG_FLAG_t* p_flag)
{
    p_flag->active = false;
    p_flag->duration = -1;
    p_flag->last_trigger_time.millisec=0;
    p_flag->last_trigger_time.microsec=0;
}


/**
 * Function MSG_FLAG_Execute indicates if the action associated with the message flag should be executed
 * in the main loop both because its active and because its time.
 * @return [bool] True for execute action, False for skip action
 */
bool MSG_FLAG_Execute( MSG_FLAG_t* p_flag)
{
    // *** MEGN540  ***
    // THIS FUNCTION WILL BE MOST USEFUL FORM LAB 2 ON.
    // What is the logic to indicate an action should be executed?
    // For Lab 1, ignore the timing part.
    
    if(p_flag->active) {
        if(p_flag->duration < SecondsSince(&p_flag->last_trigger_time))
            return true;
    }
    
    return false;
}


/**
 * Function Message_Handling_Init initializes the message handling and all associated state flags and data to their default
 * conditions.
 */
void Message_Handling_Init()
{
    // *** MEGN540  ***
    // YOUR CODE HERE. This is where you'd initialize any
    // state machine flags to control your main-loop state machine

    MSG_FLAG_Init( &mf_restart ); // needs to be initialized to the default values.
    MSG_FLAG_Init( &mf_loop_timer ); // needs to be initialized to the default values.
    MSG_FLAG_Init( &mf_time_float_send ); // needs to be initialized to the default values.
    MSG_FLAG_Init( &mf_send_time ); // needs to be initialized to the default values.
    MSG_FLAG_Init( &mf_time_out);
    MSG_FLAG_Init( &mf_battery); 
    MSG_FLAG_Init( &mf_encoder);
    MSG_FLAG_Init( &mf_motor_pwm); /// Motor PWM setting
    MSG_FLAG_Init( &mf_motor_id); /// Reports Encoder Rads on request
    MSG_FLAG_Init( &mf_control_pos );
    MSG_FLAG_Init( &mf_control_vel );
    MSG_FLAG_Init( &mf_controller );
    return;
}

/**
 * Function Message_Handler processes USB messages as necessary and sets status flags to control the flow of the program.
 * It returns true unless the program receives a reset message.
 * @return
 */
void Message_Handling_Task() {

    //float radius = 0.01885;
    //float car_width = 0.125;
    float radius = 0.018;
    float car_width = 0.11;

    // Check to see if their is data in waiting
    if( !usb_msg_length() )
        return; // nothing to process...

    // Get Your command designator without removal so if their are not enough bytes yet, the command persists
    char command = usb_msg_peek();

    // process command
    switch( command )
    {
        case '*':
            if( usb_msg_length() >= MEGN540_Message_Len('*') )
            {
                //then process your times...

                // remove the command frosm the usb recieved buffer using the usb_msg_get() function
                usb_msg_get(); // removes the first character from the received buffer, we already know it was a * so no need to save it as a variable
            
                // Build a meaningful structure to put your data in. Here we want two floats.
                struct __attribute__((__packed__)) { float v1; float v2; } data;

                // Copy the bytes from the usb receive buffer into our structure so we can use the information
                usb_msg_read_into( &data, sizeof(data) );
                
                // Do the thing you need to do. Here we want to multiply
                float ret_val = data.v1 * data.v2;

                // send response right here if appropriate.
                usb_send_msg("cf", command, &ret_val, sizeof(ret_val));
            } else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;
        case '/':
            if( usb_msg_length() >= MEGN540_Message_Len('/') )
            {
                //then process your divide...
                
                // remove the command from the usb recieved buffer using the usb_msg_get() function
                usb_msg_get(); // removes the first character from the received buffer, we already know it was a * so no need to save it as a variable

                // Build a meaningful structure to put your data in. Here we want two floats.
                struct __attribute__((__packed__)) { float v1; float v2; } data;

                // Copy the bytes from the usb receive buffer into our structure so we can use the information
                usb_msg_read_into( &data, sizeof(data) );

                // Do the thing you need to do. Here we want to multiply
                float ret_val = data.v1 / data.v2;

                // send response right here if appropriate.
                usb_send_msg("cf", command, &ret_val, sizeof(ret_val));
            } else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;
        case '+':
            if( usb_msg_length() >= MEGN540_Message_Len('+') )
            {
                mf_time_out.active = false;
                //then process your plus...
                
                // remove the command from the usb recieved buffer using the usb_msg_get() function
                usb_msg_get(); // removes the first character from the received buffer, we already know it was a * so no need to save it as a variable

                // Build a meaningful structure to put your data in. Here we want two floats.
                struct __attribute__((__packed__)) { float v1; float v2; } data;

                // Copy the bytes from the usb receive buffer into our structure so we can use the information
                usb_msg_read_into( &data, sizeof(data) );

                // Do the thing you need to do. Here we want to multiply
                float ret_val = data.v1 + data.v2;

                // send response right here if appropriate.
                usb_send_msg("cf", command, &ret_val, sizeof(ret_val));
            } else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;
        case '-':
            if( usb_msg_length() >= MEGN540_Message_Len('-') )
            {
                mf_time_out.active = false;
                //then process your minus...
                
                // remove the command from the usb recieved buffer using the usb_msg_get() function
                usb_msg_get(); // removes the first character from the received buffer, we already know it was a * so no need to save it as a variable

                // Build a meaningful structure to put your data in. Here we want two floats.
                struct __attribute__((__packed__)) { float v1; float v2; } data;

                // Copy the bytes from the usb receive buffer into our structure so we can use the information
                usb_msg_read_into( &data, sizeof(data) );

                // Do the thing you need to do. Here we want to multiply
                float ret_val = data.v1 - data.v2;

                // send response right here if appropriate.
                usb_send_msg("cf", command, &ret_val, sizeof(ret_val));
            } else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;
        case '~':
            if( usb_msg_length() >= MEGN540_Message_Len('~') )
            {
                //then process your reset by setting the mf_restart flag
                
                mf_restart.active = true;
                mf_restart.duration = 0;
                mf_restart.last_trigger_time = GetTime();
            }
            break;
        case 't':
            if( usb_msg_length() >= MEGN540_Message_Len('t') )
            {
                mf_time_out.active = false;
                usb_msg_get(); //pops t character off
                uint8_t c = usb_msg_get();

                switch(c){
                    case 0:
                        //Get time now
                        mf_send_time.active = true;
                        mf_send_time.duration = -1.0f;
                        mf_send_time.last_trigger_time = GetTime();
                        
                        break;
                    case 1:
                        //Get Loop Time
                        mf_loop_timer.active = true;
                        mf_loop_timer.duration = -1.0f;
                        mf_loop_timer.last_trigger_time = GetTime();
                        break;
                }
            } else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;
        case 'T':
            if( usb_msg_length() >= MEGN540_Message_Len('T') )
            {
                mf_time_out.active = false;
                usb_msg_get(); //pops t character off
                uint8_t c = usb_msg_get();
                float duration;
                usb_msg_read_into( &duration, sizeof(duration) );

                switch(c){
                    case 0:
                        //Get time now
                        mf_send_time.active = true;
                        mf_send_time.duration = duration;
                        mf_send_time.last_trigger_time = GetTime();
                        
                        break;
                    case 1:
                        //Get Loop Time
                        mf_loop_timer.active = true;
                        mf_loop_timer.duration = duration;
                        mf_loop_timer.last_trigger_time = GetTime();
                        break;
                }
            } else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }

            break;
        case 'b':
        case 'B':
            if( (command == 'B' && usb_msg_length() >= MEGN540_Message_Len('B')) || (command == 'b' && usb_msg_length() >= MEGN540_Message_Len('b')))
            {
                mf_battery.active = true;
                 usb_msg_get(); //pops b character off
                if(command == 'b') {
                    mf_battery.duration = -1;
                }
                else {
                    usb_msg_read_into( &mf_battery.duration, sizeof(mf_battery.duration) );
                }
                mf_battery.last_trigger_time = GetTime();
            }else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;
        case 'e':
        case 'E':
            if( (command == 'e' && usb_msg_length() >= MEGN540_Message_Len('e')) || (command == 'E' && usb_msg_length() >= MEGN540_Message_Len('E')))
            {
                mf_encoder.active = true;
                 usb_msg_get(); //pops e character off
                if(command == 'e') {
                    mf_encoder.duration = -1;
                }
                else {
                    usb_msg_read_into( &mf_encoder.duration, sizeof(mf_encoder.duration) );
                }
                mf_encoder.last_trigger_time = GetTime();
            }else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;
        case 'p':
        case 'P':
            if( (command == 'p' && usb_msg_length() >= MEGN540_Message_Len('p')) || (command == 'P' && usb_msg_length() >= MEGN540_Message_Len('P')))
            {
                
                mf_motor_pwm.active = true;
                usb_msg_get(); //pops p character off

                if(command == 'p') {
                    struct __attribute__((__packed__)) { int16_t left; int16_t right; } data;
                    usb_msg_read_into( &data, sizeof(data) );
                    
                    Motor_PWM_Left(data.left);
				    Motor_PWM_Right(data.right);

                    //usb_send_msg("chh", command, &data, sizeof(data));
                    mf_motor_pwm.duration = -1;
                    
                }
                else {
                    struct __attribute__((__packed__)) { int16_t left; int16_t right; float dur; } data;
                    usb_msg_read_into( &data, sizeof(data) );
                    Motor_PWM_Left(data.left);
				    Motor_PWM_Right(data.right);
                    //usb_send_msg("chhf", command, &data, sizeof(data));
                    mf_motor_pwm.duration = data.dur;

                }
                Motor_PWM_Enable(true);

                mf_motor_pwm.last_trigger_time = GetTime();
            }else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;
        case 's':
        case 'S':
            if( usb_msg_length() >= MEGN540_Message_Len('s') )
            {
                usb_msg_get();
                Motor_PWM_Left(0);
				Motor_PWM_Right(0);
                Motor_PWM_Enable(false);
                mf_controller.active = false;
            }
            break;
        case 'q':
        case 'Q':
            if( (command == 'Q' && usb_msg_length() >= MEGN540_Message_Len('Q')) || (command == 'q' && usb_msg_length() >= MEGN540_Message_Len('q')))
            {
                mf_motor_id.active = true;
                 usb_msg_get(); //pops b character off
                if(command == 'q') {
                    mf_motor_id.duration = -1;
                }
                else {
                    usb_msg_read_into( &mf_motor_id.duration, sizeof(mf_motor_id.duration) );
                }
                mf_motor_id.last_trigger_time = GetTime();
            }else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;
        case 'd':
        case 'D':
            if( (command == 'D' && usb_msg_length() >= MEGN540_Message_Len('D')) || (command == 'd' && usb_msg_length() >= MEGN540_Message_Len('d')))
            {
                
                usb_msg_get(); //pops d character off
                
                float sl = 0.0f;
                float sr = 0.0f;
                if(command == 'd') {
                    struct __attribute__((__packed__)) { float linear; float angular; } data;
                    usb_msg_read_into( &data, sizeof(data) );
                    sl = (data.linear - car_width*data.angular/2.0f)/radius;
                    sr = (data.linear + car_width*data.angular/2.0f)/radius;
                    
                    mf_control_pos.duration = -1;
                }
                else {
                    struct __attribute__((__packed__)) { float linear; float angular; float dur; } data;
                    usb_msg_read_into( &data, sizeof(data) );
                    if(data.angular<0){
                        data.angular = data.angular*0.9f;
                    }
                    sl = (data.linear - car_width*data.angular/2.0f)/radius;
                    sr = (data.linear + car_width*data.angular/2.0f)/radius;
                    
                    mf_control_pos.duration = data.dur;
                }
                struct __attribute__((__packed__)) {float left; float right; } msg = {
                    .left = sl,
                    .right = sr
                
                };
                usb_send_msg("cff", 'D', &msg, sizeof(msg));
                Controller_Set_Target_Position(&left_cont, sl+Rad_Left());
                Controller_Set_Target_Position(&right_cont, sr+Rad_Right());
                if (!mf_controller.active){
                    Controller_SetTo(&left_cont, Rad_Left());
                    Controller_SetTo(&right_cont, Rad_Right());
                }
                
                mf_control_pos.last_trigger_time = GetTime();

                mf_controller.active = true;
                mf_controller.duration = left_cont.update_period; 
                mf_controller.last_trigger_time = GetTime();
                mf_control_pos.active = true;
                Motor_PWM_Enable(true);
            }else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;

        case 'v':
        case 'V':
            if( (command == 'V' && usb_msg_length() >= MEGN540_Message_Len('V')) || (command == 'v' && usb_msg_length() >= MEGN540_Message_Len('v')))
            {
                mf_control_vel.active = true;
                
                usb_msg_get();
                float sl = 0.0f;
                float sr = 0.0f;
                if(command == 'v') {
                    struct __attribute__((__packed__)) { float linear; float angular; } data;
                    usb_msg_read_into( &data, sizeof(data) );
                    sl = (data.linear - car_width*data.angular/2.0f)/radius;
                    sr = (data.linear + car_width*data.angular/2.0f)/radius;
                    
                    mf_control_vel.duration = -1;
                }
                else {
                    struct __attribute__((__packed__)) { float linear; float angular; float dur; } data;
                    usb_msg_read_into( &data, sizeof(data) );
                    sl = (data.linear - car_width*data.angular/2.0f)/radius;
                    sr = (data.linear + car_width*data.angular/2.0f)/radius;
                    
                    mf_control_vel.duration = data.dur;
                }
                struct __attribute__((__packed__)) {float left; float right; } msg = {
                    .left = sl,
                    .right = sr
                
                };
                usb_send_msg("cff", 'V', &msg, sizeof(msg));
                Controller_Set_Target_Velocity(&left_cont, sl);
                Controller_Set_Target_Velocity(&right_cont, sr);
            
                Controller_SetTo(&left_cont, Rad_Left());
                Controller_SetTo(&right_cont, Rad_Right());
                

                mf_control_vel.last_trigger_time = GetTime();

                mf_controller.active = true;
                mf_controller.duration = left_cont.update_period; //1 KHz update
                mf_controller.last_trigger_time = GetTime();
                Motor_PWM_Enable(true);
            }else if(usb_msg_length() == 1){
                mf_time_out.active = true;
                mf_time_out.duration = 0.1f;
                mf_time_out.last_trigger_time = GetTime();
            }
            break;
        default:
            // What to do if you dont recognize the command character
            
            usb_send_msg("cc", command, "?", sizeof(char));
            usb_flush_input_buffer();
            break;
    }
}



/**
 * Function MEGN540_Message_Len returns the number of bytes associated with a command string per the
 * class documentation;
 * @param cmd
 * @return Size of expected string. Returns 0 if unreconized.
 */
uint8_t MEGN540_Message_Len( char cmd )
{
    switch(cmd)
    {
        case '~': return	1; break;
        case '*': return	9; break;
        case '/': return	9; break;
        case '+': return	9; break;
        case '-': return	9; break;
        case 't': return	2; break;
        case 'T': return	6; break;
        case 'e': return	1; break;
        case 'E': return	5; break;
        case 'b': return	1; break;
        case 'B': return	5; break;
//        case 'a': return	1; break;
//        case 'A': return 	5; break;
//        case 'w': return	1; break;
//        case 'W': return 	5; break;
//        case 'm': return	1; break;
//        case 'M': return	5; break;
        case 'p': return	5; break;
        case 'P': return	9; break;
        case 's': return 	1; break;
        case 'S': return 	1; break;
        case 'q': return	1; break;
        case 'Q': return 	5; break;
        case 'd': return 	9; break;
        case 'D': return	13; break;
        case 'v': return	9; break;
        case 'V': return	13; break;
        default:  return	0; break;
    }
}
