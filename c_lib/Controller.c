#include "Controller.h"

/**
 * Function Initialize_Controller setsup the z-transform based controller for the system.
 */
void Controller_Init(Controller_t* p_cont, float kp, float* num, float* den, uint8_t order, float update_period){
    Filter_Init ( &p_cont->controller, num, den, order );
    p_cont->kp = kp;
    p_cont->update_period = update_period;
    p_cont->target_vel = 0;
    p_cont->target_pos = 0;
}

/**
 * Function Controller_Set_Target_Velocity sets the target velocity for the 
 * controller.
 */
void Controller_Set_Target_Velocity( Controller_t* p_cont, float vel ){
    p_cont->target_vel = vel;
}

/**
 * Function Controller_Set_Target_Position sets the target postion for the 
 * controller, this also sets the target velocity to 0.
 */
void Controller_Set_Target_Position( Controller_t* p_cont, float pos ){
    p_cont->target_pos = pos;
    p_cont->target_vel = 0.0;
}

/**
 * Function Controller_Update takes in a new measurement and returns the
 * new control value.
 */
float Controller_Update( Controller_t* p_cont, float measurement, float dt ){
    float output_now = measurement;//Filter_Value(&p_cont->controller, measurement);

    p_cont->target_pos += p_cont->target_vel*dt;

    return p_cont->kp*(p_cont->target_pos - output_now);
}

/**
 * Function Controller_Last returns the last control command
 */
float Controller_Last( Controller_t* p_cont){
    return Filter_Last_Output(&p_cont->controller);
}

/**
 * Function Controller_SettTo sets the Filter's input and output lists
 * to match the measurement so it starts with zero error.
 */
void Controller_SetTo(Controller_t* p_cont, float measurement ){
    Filter_SetTo(&p_cont->controller, measurement);
}

/**
 * Function Controller_ShiftBy shifts the Filter's input and output lists
 * by the desired amount. This is helpful when dealing with wrapping.
 */
void Controller_ShiftBy(Controller_t* p_cont, float measurement ){
    Filter_ShiftBy(&p_cont->controller, measurement);
}
