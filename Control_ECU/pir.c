/*
 * pir.c
 *
 *  Created on: Oct 30, 2024
 *      Author: Tweety
 */
#include "pir.h"

void PIR_init()
{
	GPIO_setupPinDirection(PIR_PORT_ID, PIR_PIN_ID, PIN_INPUT);
}

uint8 PIR_Motion(void)
{
	uint8 motion ;
	motion=GPIO_readPin(PIR_PORT_ID, PIR_PIN_ID);

	/*if their is a motion we will return MOTION_DETECTED else we will return NO_MOTION_DETECTED */
	return motion;
}

