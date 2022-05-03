//Ryan Reschak
#include "Filter.h"

/**
 * Function Filter_Init initializes the filter given two float arrays and the order of the filter.  Note that the
 * size of the array will be one larger than the order. (First order systems have two coefficients).
 *
 *  1/A_0*( SUM( B_i * input_i )  -   SUM( A_i * output_i) )
 *         i=0..N                    i=1..N
 *
 *  Note a 5-point moving average filter has coefficients:
 *      numerator_coeffs   = { 5 0 0 0 0 };
 *      denominator_coeffs = { 1 1 1 1 1 };
 *      order = 4;
 *
 * @param p_filt pointer to the filter object
 * @param numerator_coeffs The numerator coefficients (B/beta traditionally)
 * @param denominator_coeffs The denominator coefficients (A/alpha traditionally)
 * @param order The filter order
 */
void  Filter_Init ( Filter_Data_t* p_filt, float* numerator_coeffs, float* denominator_coeffs, uint8_t order )
{	
	//numerator/ B coeffs
	rb_initialize_F(&p_filt->numerator);
	for (int i = 0; i <= order; i++)
		rb_push_back_F(&p_filt->numerator, numerator_coeffs[i]);
	//denominator / A coeffs
	rb_initialize_F(&p_filt->denominator);
	for (int i = 0; i <= order; i++)
		rb_push_back_F(&p_filt->denominator, denominator_coeffs[i]);
	//ouptputs (A list)
	rb_initialize_F(&p_filt->out_list);
	//inputs (B list)
	rb_initialize_F(&p_filt->in_list);

	Filter_SetTo(p_filt, 0);
	
	return;
}

/**
 * Function Filter_ShiftBy shifts the input list and output list to keep the filter in the same frame. This especially
 * useful when initializing the filter to the current value or handling wrapping/overflow issues.
 * @param p_filt
 * @param shift_amount
 */
void  Filter_ShiftBy( Filter_Data_t* p_filt, float shift_amount )
{
	uint8_t length = rb_length_F(&p_filt->denominator);

	for (uint8_t i = 0; i < length; i++)
		rb_push_front_F(&p_filt->in_list, shift_amount);
	for (uint8_t i = 0; i < length; i++)
		rb_push_front_F(&p_filt->out_list, shift_amount);
    return;
}

/**
 * Function Filter_SetTo sets the initial values for the input and output lists to a constant defined value. This
 * helps to initialize or re-initialize the filter as desired.
 * @param p_filt Pointer to a Filter_Data sturcture
 * @param amount The value to re-initialize the filter to.
 */
void Filter_SetTo( Filter_Data_t* p_filt, float amount )
{
	uint8_t length = rb_length_F(&p_filt->denominator);

	for (uint8_t i = 0; i < length; i++)
		rb_push_front_F(&p_filt->in_list, amount);
	for (uint8_t i = 0; i < length; i++)
		rb_push_front_F(&p_filt->out_list, amount);
	return;
}

/**
 * Function Filter_Value adds a new value to the filter and returns the new output.
 * @param p_filt pointer to the filter object
 * @param value the new measurement or value
 * @return The newly filtered value
 */
float Filter_Value( Filter_Data_t* p_filt, float value)
{
	float output = 0.0f;
	uint8_t length = rb_length_F(&p_filt->denominator);
	
	//store new value
	rb_push_front_F(&p_filt->in_list, value);
	
	//SUM(B_i * input_i) starting at 0... N
	for (int i = 0; i < length; i++)
		output += rb_get_F(&p_filt->numerator, i)*rb_get_F(&p_filt->in_list, i);
	//- SUM( A_i * output_i) ) starting at 1... N
	for (int i = 1; i < length; i++)
		output -= rb_get_F(&p_filt->denominator, i) * rb_get_F(&p_filt->out_list, i-1);
	//Divide by A0
	output /= rb_get_F(&p_filt->denominator, 0);

	//store new output for next time
	rb_push_front_F(&p_filt->out_list, output);

	return output;
}

/**
 * Function Filter_Last_Output returns the most up-to-date filtered value without updating the filter.
 * @return The latest filtered value
 */
float Filter_Last_Output( Filter_Data_t* p_filt )
{
	return rb_get_F(&p_filt->out_list, 0);
}
