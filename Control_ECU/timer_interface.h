/*
 * timer_interface.h
 *
 *  Created on: Oct 29, 2024
 *      Author: Tweety
 */

#ifndef TIMER_INTERFACE_H_
#define TIMER_INTERFACE_H_

#include "std_types.h"

//  types_declaration
typedef enum
{

	 	//	Timer0 and Timer1 have the same Prescalers but  timer2 has no Counter mode
        // 	timer2 represent final T2

	TIMER_NO_CLOCK,TIMER_F_CPU_CLOCK,TIMER_F_CPU_8,TIMER_F_CPU_64,TIMER_F_CPU_256,TIMER_F_CPU_1024,EXT_CLK_FALLING,EXT_CLK_RISING,
	TIMER_NO_CLOCK_T2=0,TIMER_F_CPU_CLOCK_T2=1,TIMER_F_CPU_8_T2=2,TIMER_F_CPU_32_T2=3,TIMER_F_CPU_64_T2=4,TIMER_F_CPU_128_T2=5,
	TIMER_F_CPU_256_T2=6,TIMER_F_CPU_1024_T2=7
}Timer_Clock;

//timer module
typedef enum
{
	Timer0,Timer1,Timer2
}Timer_ID_Type;

//timer mode
typedef enum
{
// timer2 does not have pwm mode
	Overflow_Mode,Compare_Mode
}Timer_Mode;

typedef struct
{
	uint16 timer_InitialValue;
	uint16 timer_compare_MatchValue;	/*it will be used in compare mode only*/
	Timer_ID_Type  timer_ID;
	Timer_Clock timer_clock;
	Timer_Mode  timer_mode;

}Timer_ConfigType;

                    /*functions Prototypes */

//Description: Function to initialize the Timer driver
void Timer_init(const Timer_ConfigType * Config_Ptr);

//Description: Function to disable the Timer via Timer_ID
void Timer_deInit(Timer_ID_Type timer_type);

//Description: Function to set the Call Back function address to the required Timer.

void Timer_setCallBack(void(*a_ptr)(void), Timer_ID_Type a_timer_ID );

#endif /* TIMER_INTERFACE_H_ */
