/*
 * control_ecu.c
 *
 *  Created on: Oct 30, 2024
 *      Author: Tweety
 */
#include "control_ech.h"
#include "gpio_interface.h"
#include "twi_interface.h"
#include "uart_interface.h"
#include "pwm_interface.h"
#include "pir.h"
#include "dc_motor.h"
#include "buzzer.h"
#include "external_eeprom.h"
#include "Internal_mapping_register_private.h"
#include "timer_interface.h"
/******************Global Variables***************/
static uint8 g_MC2_application_sequence=0;

static uint8 g_passwordArray1[5],g_passwordArray2[5];

static volatile uint8 g_tick=0;
/*********************  Main Function  ******************/
int main(void)
{

	PWM_config Pwm_configuration={PWM0,NON_INVERTING,PWM_F_CPU_1024};
	Timer_ConfigType Timer_configuration={0,0,Timer1,TIMER_F_CPU_1024,Compare_Mode};
	UART_ConfigType Uart_configuration={CHARACTER_SIZE_8,EVEN_PARITY,STOP_1BIT,9600};
	I2c_ConfigType	I2c_configuration= {0x01,0x02,TWI_Prescaler_1};
	TWI_init(&I2c_configuration);
	/*Enable Global Interrupt*/
	SREG_REG.Bits.I_Bit=1;
	UART_init(&Uart_configuration);
	/*Set timer1 to call back */
	Timer_setCallBack(timer1_SetRequiredCallBackFunction, Timer1);
	/*Initialize password*/
	BUZZER_init();
	/*Initialize motor*/
	DcMOTOR_init();

	while(1)
	{
		switch(g_MC2_application_sequence)
		{
		case RECEIVING_PASSWORDS_STAGE:

			/*Receive the first password enter by user*/
			receive_Password(g_passwordArray1);

			/*Receive the re-enterd password enter by user*/
			receive_Password(g_passwordArray2);

			/* Increment Stage for next loop (go to CHECK_PASSWORDS_MATCHING_STAGE )*/
			g_MC2_application_sequence=CHECK_PASSWORDS_MATCHING_STAGE;
			break;

		case CHECK_PASSWORDS_MATCHING_STAGE:
		{

			uint8 twoPassowrdState=check_TwoPasswords(g_passwordArray1,g_passwordArray2);
                  /*Two passwords are matched*/
			if(twoPassowrdState == MATCHED)
			{
				/*Send to HMI_ECU that the two password are matched*/
				send_Byte(MATCHED);

				/*save passwords */
				g_MC2_application_sequence=SAVE_PASSWORD_INTO_EEPROM_STAGE;
			}
			/*/*Two passwords are NOT matched*/
			else if(twoPassowrdState == NOT_MATCHED)
			{
				/*Send to HMI_ECU that the two password are matched*/
				send_Byte(NOT_MATCHED);

				/*Go to RECEIVING_PASSWORDS_STAGE*/
				g_MC2_application_sequence=RECEIVING_PASSWORDS_STAGE;
			}
			else
			{
				/*Do Nothing*/
			}
			break;
		}
		case SAVE_PASSWORD_INTO_EEPROM_STAGE:

			/*Write the password in EEPROM at index start from 0x0000 to 0x0000+(PASWWORD_SIZE-1)*/
			EEPROM_writeArray(0X0000, g_passwordArray1, PASSWORD_SIZE);

			/* Increment Stage for next loop (go to MAIN_MENU_OPTION_STAGE )*/
			g_MC2_application_sequence=MAIN_MENU_OPTION_STAGE;

			break;

		case MAIN_MENU_OPTION_STAGE:
		{
			/*NOTE : i made this block scope bec. i want the local varriables defined in this case just to be local for its case*/
			uint8 twoPasswordState;

			/*Receive the password enter from user after choise an + or - option in main option menu*/
			receive_Password(g_passwordArray2);

			/*read the old password stored in the EEPROM*/
			EEPROM_readArray(0X0000, g_passwordArray1, PASSWORD_SIZE);

			/*Compare the oldest password stored in EEPROM with the Password enter via user*/
			twoPasswordState=check_TwoPasswords(g_passwordArray1, g_passwordArray2);

			/*if two password are matched then make for user what he/she choose in main menu option*/
			if(twoPasswordState==MATCHED)
			{
				uint8 recieveByte;

				/*Send Matched to HMI ECU*/
				send_Byte(MATCHED);

				recieveByte=recieve_Byte();

				if(recieveByte=='+')
				{
					/*send to HMI ECU 3 which is indication for HMI ECU to go to OPEN_DOOR_STAGE in HMI ECU for displaying the required messages */
					send_Byte(3);

					/* Increment Stage for next loop (go to OPEN_DOOR_STAGE to unlocking/locking the door )*/
					g_MC2_application_sequence=OPEN_DOOR_STAGE;
				}
				else if (recieveByte=='-')
				{
					/*send to HMI ECU 0 which is indication for HMI ECU to go to CREATE_PASSWORD_STAGE in HMI ECU */
					send_Byte(0);

					/* go to first  Stage RECEIVING_PASSWORDS_STAGE in CONTROL ECU to make a new password again*/
					g_MC2_application_sequence=RECEIVING_PASSWORDS_STAGE;
				}
				else
				{
					/*Do Nothing*/
				}
			}
			else if (twoPasswordState==NOT_MATCHED)
			{
				uint8 count;

				send_Byte(NOT_MATCHED);
				for(count=0;count<2;count++)
				{
					/*Receive the entered password by user*/
					receive_Password(g_passwordArray2);

					/*Read the password already stored in External EEPROM*/
					EEPROM_readArray(0X0000, g_passwordArray1, PASSWORD_SIZE);

					if(check_TwoPasswords(g_passwordArray1, g_passwordArray2)==MATCHED)
					{
						/*Send the HMI ECU that two password are matched*/
						send_Byte(MATCHED);

						/*go to MAIN_MENU_OPTION_STAGE where the user choose again what he want  */
						g_MC2_application_sequence=MAIN_MENU_OPTION_STAGE;
						break;
					}
					else
					{
						send_Byte(NOT_MATCHED);
					}
				}

				if(count==2)
				{


					Timer_configuration.timer_compare_MatchValue=46875;

					Timer_init(&Timer_configuration);

					BUZZER_on();
					while(g_tick < 10)
					{

					}

					/*stop timer1 via using deInit() Function*/
					Timer_DeInit(Timer1);

					g_tick=0;

					/*Turn off the buzzer after the 1 min ends*/
					BUZZER_off();
				}
					/* go to MAIN_MENU_OPTION_STAGE */
					g_MC2_application_sequence=MAIN_MENU_OPTION_STAGE;
			}
			else
			{
				/*Do Nothing*/
			}
		}
			break;
		case OPEN_DOOR_STAGE:

			Timer_configuration.timer_compare_MatchValue=58594;

			/*Intialize PWM of timer0*/
			PWM_init(&Pwm_configuration);

			/*Initialize Timer 1*/
			Timer_init(&Timer_configuration);

			/*Turn on the motor clockwise */
			DCMOTOR_rotateWithControl(ROTATE_CLOCKWISE, 100, FIRST_DC_MOTOR, PWM0);

			/*Wait for 15 sec Rotating motor clockwise*/
			while(g_tick<2){};

			/*Stop Timer1*/
			Timer_DeInit(Timer1);

			g_tick=0;

			/*stop the dc motor */
			DCMOTOR_rotateWithControl(STOP_ROTATION, 0, FIRST_DC_MOTOR, PWM0);

			//set Compare value = 23438
			Timer_configuration.timer_compare_MatchValue=23438;
			Timer_init(&Timer_configuration);

			/*wait for 3 sec and the motor is stop*/
			while(g_tick<1){};

			while(PIR_Motion()==MOTION_DECTECTED);

			send_Byte(NO_MOTION_DECTECTED);

			/*Stop Timer1*/
			Timer_DeInit(Timer1);

			g_tick=0;

			/*Turn on the motor clockwise */
			DCMOTOR_rotateWithControl(ROTATE_ANTI_CLOCKWISE, 100, FIRST_DC_MOTOR, PWM0);

			Timer_configuration.timer_compare_MatchValue=58594;
			/*Initialize Timer 1*/
			Timer_init(&Timer_configuration);

			/*Wait for 15 sec*/
			while(g_tick<2){};

			/*Stop Timer1*/
			Timer_DeInit(Timer1);

			g_tick=0;
			/*Turn off the motor*/
			DCMOTOR_rotateWithControl(STOP_ROTATION, 0, FIRST_DC_MOTOR, PWM0);

			g_MC2_application_sequence=MAIN_MENU_OPTION_STAGE;
			break;
		}

		}

	}


/*   Functions Definitions  */
void timer1_SetRequiredCallBackFunction(void)
{
	g_tick++;
}

/*
 * Function Description :
 * Receiving Password entered by the user from HMI_ECU and store it in array defined global
 */
void receive_Password(uint8* a_PasswordArray)
{
	uint8 i=0;
	while(UART_recieveByte()!=READY);
	UART_sendByte(READY);
	while(i<PASSWORD_SIZE)
	{
		a_PasswordArray[i]=UART_recieveByte();
		i++;

	}
	UART_sendByte(DONE);
}


uint8 check_TwoPasswords(uint8 *Password_array1,uint8 *Password_array2)
{
	uint8 i;
	for(i=0;i<PASSWORD_SIZE;i++)
	{
		if(Password_array1[i]!=Password_array2[i])
		{
			return NOT_MATCHED;
		}
	}
	return MATCHED;
}

// Receiving data from the UART in the HMI_ECU

uint8 recieve_Byte(void)
{
	uint8 byte;
	while(UART_recieveByte() != READY){}
	UART_sendByte(READY);
	byte=UART_recieveByte();
	UART_sendByte(DONE);
	return byte;
}

 // Sending data to the UART in the HMI_ECU

void send_Byte(uint8 byte)
{
	UART_sendByte(READY);
	while(UART_recieveByte() != READY){}
	UART_sendByte(byte);
	while(UART_recieveByte() != DONE){}
}
