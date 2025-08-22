/*
 * pwm.c
 *
 *  Created on: Oct 26, 2024
 *      Author: Tweety
 */

#include "gpio_interface.h"
#include "pwm_interface.h"
#include "timer_confugration.h"


                   /***********function definition**********/

void PWM_init(const PWM_config *Config_Ptr)
{
	if(Config_Ptr->num == PWM0)
	{
		TIMER0_TCCR0_REG.Bits.FOC0_Bit=0;

		TIMER0_TCCR0_REG.Bits.WGM00_Bit=1;
		TIMER0_TCCR0_REG.Bits.WGM01_Bit=1;
		TIMER0_TCCR0_REG.Byte|=(Config_Ptr->mode);
		TIMER0_TCCR0_REG.Byte|=(Config_Ptr->prescaleTimer);

		GPIO_setupPinDirection(PORTB_ID, PIN3_ID, PIN_OUTPUT);

		TIMER0_OCR0_REG.Byte=0;

	}

	else if(Config_Ptr->num == PWM2)
	{
		TIMER2_TCCR2_REG.Bits.FOC2_Bit=0;

		TIMER2_TCCR2_REG.Bits.WGM20_Bit=1;
		TIMER2_TCCR2_REG.Bits.WGM21_Bit=1;

		TIMER2_TCCR2_REG.Byte|=(Config_Ptr->mode);

		TIMER2_TCCR2_REG.Byte|=(Config_Ptr->prescaleTimer);

		GPIO_setupPinDirection(PORTD_ID, PIN7_ID, PIN_OUTPUT);

		TIMER2_OCR2_REG.Byte=0;

	}
}

void PWM_setDutyCycle(Pwm_Number a_pwmNum,uint8 a_duty)
{
	if(a_pwmNum == PWM0)
	{
		TIMER0_OCR0_REG.Byte=((uint32)(a_duty* 255) / 100);
	}

	else if(a_pwmNum == PWM2)
	{
		TIMER2_OCR2_REG.Byte=((uint32)(a_duty* 255) / 100);
	}
}
