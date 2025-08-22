/*
 * timer.c
 *
 *  Created on: Oct 29, 2024
 *      Author: Tweety
 */

#include "timer_confugration.h"

#include"timer_interface.h"
#include <avr/interrupt.h>
                 /*Global Variables*/


  static volatile void (*g_Timers_callBackPtr[3])(void)={NULL_PTR};

  /********************* initialize the timers************************/
void Timer_init(const Timer_ConfigType *Config_Ptr) {

	switch (Config_Ptr->timer_ID) {

	case Timer0:

		TIMER0_TCCR0_REG.Byte = 0;

		TIMER0_TCNT0_REG.Byte = 0;
		TIMER0_OCR0_REG.Byte = 0;

		/*Clear Interrupt modules forTimer0 */
		TIMER_TIMSK_REG.Bits.OCIE0_Bit = 0;
		TIMER_TIMSK_REG.Bits.TOIE0_Bit = 0;

		/*Set Intial Value*/
		TIMER0_TCNT0_REG.Byte = ((Config_Ptr->timer_InitialValue)) & 0x00FF;

		TIMER0_TCCR0_REG.Byte = ((TIMER0_TCCR0_REG.Byte & 0xF8)
				| (Config_Ptr->timer_clock));

		TIMER0_TCCR0_REG.Bits.COM00_Bit = 0;
		TIMER0_TCCR0_REG.Bits.COM01_Bit = 0;

		switch (Config_Ptr->timer_mode) {
		case Overflow_Mode:

			TIMER0_TCCR0_REG.Bits.WGM00_Bit = 0;
			TIMER0_TCCR0_REG.Bits.WGM01_Bit = 0;

			/* SET FOC0 in any mode except PWM mode */
			TIMER0_TCCR0_REG.Bits.FOC0_Bit = 1;

			/* Enable Timer0 overflow interrupt */
			TIMER_TIMSK_REG.Bits.TOIE0_Bit = 1;
			break;

		case Compare_Mode:

			TIMER0_TCCR0_REG.Bits.WGM00_Bit = 0;
			TIMER0_TCCR0_REG.Bits.WGM01_Bit = 1;

			/* SET FOC0 in any mode except PWM mode */
			TIMER0_TCCR0_REG.Bits.FOC0_Bit = 1;

			/* set the compare value in register OCR0 & with 0xFF to make sure that it doesn't exceed 8 bits */
			TIMER0_OCR0_REG.Byte = ((Config_Ptr->timer_compare_MatchValue))
					& 0x00FF;

			/* Enable Timer0 compare match interrupt*/
			TIMER_TIMSK_REG.Bits.OCIE0_Bit = 1;

			break;
		}

		break; /*End of Timer0 case*/

	case Timer1:

		TIMER1_TCCR1A_REG.Byte = 0;
		TIMER1_TCCR1B_REG.Byte = 0;
		TIMER1_TCNT1_REG.TwoBytes = 0;
		TIMER1_OCR1A_REG.TwoBytes = 0;

		/*Clear all Interrupt modules Enable of Timer1 (TICIE1=0,OCIE1A=0,OCIE1B=0,TOIE1=0)*/
		TIMER_TIMSK_REG.Byte &= 0xC3;

		/*Set Intial Value*/
		TIMER1_TCNT1_REG.TwoBytes = (Config_Ptr->timer_InitialValue);

		TIMER1_TCCR1B_REG.Byte = (TIMER1_TCCR1B_REG.Byte & 0xF8)
				| (Config_Ptr->timer_clock);

		TIMER1_TCCR1A_REG.Byte &= 0x0F;

		switch (Config_Ptr->timer_mode) {
		case Overflow_Mode:

			TIMER1_TCCR1A_REG.Bits.WGM10_Bit = 0;
			TIMER1_TCCR1A_REG.Bits.WGM11_Bit = 0;
			TIMER1_TCCR1B_REG.Bits.WGM12_Bit = 0;
			TIMER1_TCCR1B_REG.Bits.WGM13_Bit = 0;

			/* Enable Timer1 overflow interrupt */
			TIMER_TIMSK_REG.Bits.TOIE1_Bit = 1;

			break;

		case Compare_Mode:

			TIMER1_TCCR1A_REG.Byte &= 0xFC;
			TIMER1_TCCR1B_REG.Bits.WGM12_Bit = 1;
			TIMER1_TCCR1B_REG.Bits.WGM13_Bit = 0;

			/* SET FOC1A/FOC1B in any mode except PWM mode */
			TIMER1_TCCR1A_REG.Bits.FOC1A_Bit = 1;
			TIMER1_TCCR1A_REG.Bits.FOC1B_Bit = 1;

			/* set the compare value in register OCR1A*/
			TIMER1_OCR1A_REG.TwoBytes = (Config_Ptr->timer_compare_MatchValue);

			/* Enable Timer1 compare match module interrupt*/
			TIMER_TIMSK_REG.Bits.OCIE1A_Bit = 1;

			break;
		}

		break; /*End of Timer1 case*/

	case Timer2:

		TIMER2_TCCR2_REG.Byte = 0;
		TIMER2_TCNT2_REG.Byte = 0;
		TIMER2_OCR2_REG.Byte = 0;

		/*Clear all Interrupt modules Enable of Timer2 (OCIE2=0,TOIE2=0)*/
		TIMER_TIMSK_REG.Bits.OCIE2_Bit = 0;
		TIMER_TIMSK_REG.Bits.TOIE2_Bit = 0;

		/*Set Intial Value & with 0xFF to make sure that it doesn't exceed 8 bits */
		TIMER2_TCNT2_REG.Byte = ((Config_Ptr->timer_InitialValue)) & 0x00FF;

		TIMER2_TCCR2_REG.Byte = ((TIMER2_TCCR2_REG.Byte & 0xF8)
				| (Config_Ptr->timer_clock));

		/*
		 *  Compare mode:
		 *  	OC2 disconnected
		 *      Clear COM20 bit in TCCR2 register
		 *      Clear COM21 bit in TCCR2 register
		 */
		TIMER2_TCCR2_REG.Bits.COM20_Bit = 0;
		TIMER2_TCCR2_REG.Bits.COM21_Bit = 0;

		switch (Config_Ptr->timer_mode) {
		case Overflow_Mode:

			/*
			 * Normal Overflow mode:
			 * 		Clear WGM20 bit in TCCR2 register
			 *      Clear WGM21 bit in TCCR2 register
			 */
			TIMER2_TCCR2_REG.Bits.WGM20_Bit = 0;
			TIMER2_TCCR2_REG.Bits.WGM21_Bit = 0;

			/* SET FOC2 in any mode except PWM mode */
			TIMER2_TCCR2_REG.Bits.FOC2_Bit = 1;

			/* Enable Timer2 overflow interrupt */
			TIMER_TIMSK_REG.Bits.TOIE2_Bit = 1;

			break;

		case Compare_Mode:

			/*
			 *  Compare mode:
			 *  	Clear WGM20 bit in TCCR2 register
			 *      SET WGM21 bit in TCCR2 register
			 */
			TIMER2_TCCR2_REG.Bits.WGM20_Bit = 0;
			TIMER2_TCCR2_REG.Bits.WGM21_Bit = 1;

			/* SET FOC2 in any mode except PWM mode */
			TIMER2_TCCR2_REG.Bits.FOC2_Bit = 1;

			/* set the compare value in register OCR2 & with 0xFF to make sure that it doesn't exceed 8 bits */
			TIMER2_OCR2_REG.Byte = ((Config_Ptr->timer_compare_MatchValue))
					& 0x00FF;

			/* Enable Timer2 compare match interrupt*/
			TIMER_TIMSK_REG.Bits.OCIE2_Bit = 1;

			break;
		}

		break; /*End of Timer2 case*/

	}/*End of the outer Super switch case*/

}

/****************Set CallBack Function******************/

void Timer_setCallBack(void (*a_ptr)(void), Timer_ID_Type a_timer_ID) {

	switch (a_timer_ID) {
	case Timer0:
		/* Save the address of the Call back function in a global Ptr to Function variable for timer0 */
		g_Timers_callBackPtr[0] = a_ptr;
		break;

	case Timer1:
		/* Save the address of the Call back function in a global Ptr to Function variable for timer1 */
		g_Timers_callBackPtr[1] = a_ptr;
		break;

	case Timer2:
		/* Save the address of the Call back function in a global Ptr to Function variable for timer2 */
		g_Timers_callBackPtr[2] = a_ptr;
		break;
	}

}

/***********************De initialize the timer****************/
void Timer_DeInit(Timer_ID_Type timer_type) {
	switch (timer_type) {
	case Timer0:

		/*Clear all register of Timer0*/
		TIMER0_TCCR0_REG.Byte = 0;
		TIMER0_TCNT0_REG.Byte = 0;
		TIMER0_OCR0_REG.Byte = 0;

		/*Clear all Interrupt modules Enable of Timer0 (OCIE0=0,TOIE0=0)*/
		TIMER_TIMSK_REG.Bits.OCIE0_Bit = 0;
		TIMER_TIMSK_REG.Bits.TOIE0_Bit = 0;
		break;

	case Timer1:

		/*Clear all register of Timer1*/
		TIMER1_TCCR1A_REG.Byte = 0;
		TIMER1_TCCR1B_REG.Byte = 0;
		TIMER1_TCNT1_REG.TwoBytes = 0;
		TIMER1_OCR1A_REG.TwoBytes = 0;

		/*Clear all Interrupt modules Enable of Timer1 (TICIE1=0,OCIE1A=0,OCIE1B=0,TOIE1=0)*/
		TIMER_TIMSK_REG.Byte &= 0xC3;
		break;

	case Timer2:

		/*Clear all register of Timer2*/
		TIMER2_TCCR2_REG.Byte = 0;
		TIMER2_TCNT2_REG.Byte = 0;
		TIMER2_OCR2_REG.Byte = 0;

		/*Clear all Interrupt modules Enable of Timer2 (OCIE2=0,TOIE2=0)*/
		TIMER_TIMSK_REG.Bits.OCIE2_Bit = 0;
		TIMER_TIMSK_REG.Bits.TOIE2_Bit = 0;
		break;

	}

}

                       /*************************TIMER0*******************/
             /*Interrupt Service Routines for timer0, timer1,timer2  */


                /*************************TIMER0*******************/
  ISR(TIMER0_OVF_vect)
  {
  	if(g_Timers_callBackPtr[0] != NULL_PTR)
  	{
  		g_Timers_callBackPtr[0]();
  	}
  }
                /*****************interrupt service routine for timer0 CTC***************/
  ISR(TIMER0_COMP_vect)
  {
  	if(g_Timers_callBackPtr[0] != NULL_PTR)
  	{
  		g_Timers_callBackPtr[0]();
  	}
  }

                        /***********************TIMER1*********************/
ISR(TIMER1_OVF_vect)
{
	if(g_Timers_callBackPtr[1] != NULL_PTR)
	{
		g_Timers_callBackPtr[1]();
	}
}

                          /*interrupt service routine for timer1 CTC*/
ISR(TIMER1_COMPA_vect)
{
	if(g_Timers_callBackPtr[1] != NULL_PTR)
	{
		g_Timers_callBackPtr[1]();
	}
}

                       /**************************TIMER2*********************/

/*interrupt service routine for timer2 Overflow*/
ISR(TIMER2_OVF_vect)
{
	if(g_Timers_callBackPtr[2] != NULL_PTR)
	{
		g_Timers_callBackPtr[2]();
	}
}

                       /*interrupt service routine for timer2 CTC*/
ISR(TIMER2_COMP_vect)
{
	if(g_Timers_callBackPtr[2] != NULL_PTR)
	{
		g_Timers_callBackPtr[2]();
	}
}

