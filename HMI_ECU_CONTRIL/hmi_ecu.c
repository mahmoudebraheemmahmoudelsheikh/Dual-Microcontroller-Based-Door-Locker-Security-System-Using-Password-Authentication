/*
 * hmi_ecu.c
 *
 *  Created on: Oct 31, 2024
 *      Author: Tweety
 */
#include "hmi_ecu.h"
#include "gpio_interface.h"
#include "lcd.h"
#include "keypad.h"
#include "uart_interface.h"
#include "timer_interface.h"
#include <util/delay.h>
#include "Internal_mapping_register_private.h"  /* For enabling I-bit*/

                   /* Global Variables*/
static uint8 g_MC1_application_sequence=0;

static uint8 g_passwordArray1[5],g_passwordArray2[5];

static volatile uint8 g_tick=0;


int main(void)
{


	Timer_ConfigType Timer_configuration={0,0,Timer1,TIMER_F_CPU_1024,Compare_Mode};

	UART_ConfigType Uart_configuration={CHARACTER_SIZE_8,EVEN_PARITY,STOP_1BIT,9600};
	UART_init(&Uart_configuration);

	/*Enable Global Interrupt*/
	SREG_REG.Bits.I_Bit=1;

	/*Set timer1 to call back */
	Timer_setCallBack(timer1_SetRequiredCallBackFunction, Timer1);

	LCD_init();
	LCD_displayStringRowColumn(0,0,"Door Lock System");
	LCD_displayStringRowColumn(1,0,"Mahmoud Elsheikh");
	_delay_ms(2500);

	while(1)
	{
		switch(g_MC1_application_sequence)
		{
			case CREATE_PASSWORD_STAGE:
				LCD_clearScreen();
				LCD_displayStringRowColumn(0, 0,"Plz enter pass:");
				LCD_moveCursor(1, 0);
				create_Password(g_passwordArray1);
				LCD_clearScreen();
				LCD_displayStringRowColumn(0, 0,"Plz re-enter the");
				LCD_displayStringRowColumn(1, 0,"same pass: ");
				LCD_moveCursor(1, 10);
				create_Password(g_passwordArray2);

				/*Sending the password*/
				send_Password(g_passwordArray1);

				/*Sending the re-entered password */
				send_Password(g_passwordArray2);

				/* checking password stage*/
				g_MC1_application_sequence=CHECKING_PASSWORDS_MATCHING_STAGE;
				break;

			case CHECKING_PASSWORDS_MATCHING_STAGE:

				/*Check if the two passwords are matched or not*/
				if(Password_Checker()!=MATCHED)
				{
					/*  CREATE_PASSWORD_STAGE */
					g_MC1_application_sequence=CREATE_PASSWORD_STAGE;
				}
				else
				{
					/*go to next stage */
					g_MC1_application_sequence=MAIN_OPTIONS_STAGE;

				}
				break;

			case MAIN_OPTIONS_STAGE:
			{
				uint8 userChoiceKey;
				LCD_clearScreen();
				LCD_displayStringRowColumn(0, 0, "+ : OPEN DOOR");
				LCD_displayStringRowColumn(1, 0, "- : CHANGE PASS");
				userChoiceKey=KEYPAD_getPressedKey();
				while(userChoiceKey != '+' && userChoiceKey != '-'){};
				LCD_clearScreen();
				LCD_displayStringRowColumn(0, 0,"Plz enter old");
				LCD_displayStringRowColumn(1, 0,"pass :");

				create_Password(g_passwordArray1);

				send_Password(g_passwordArray1);


				if(recieve_Byte()==MATCHED)
				{
					send_Byte(userChoiceKey);

					g_MC1_application_sequence=recieve_Byte();
				}
				else
				{
					uint8 count;

					for(count=0;count<2;count++)
					{
						LCD_clearScreen();
						LCD_displayStringRowColumn(0, 0,"Plz enter old");
						LCD_displayStringRowColumn(1, 0,"pass:");

						create_Password(g_passwordArray1);
						send_Password(g_passwordArray1);

						if(recieve_Byte()==MATCHED)
						{
							g_MC1_application_sequence=MAIN_OPTIONS_STAGE;
							break;
						}
					}

					if(2==count)
					{
						LCD_clearScreen();

						Timer_configuration.timer_compare_MatchValue=46875;

						Timer_init(&Timer_configuration);

						LCD_displayStringRowColumn(0,1,"System LOCKED");
						LCD_displayStringRowColumn(1,0,"Wait for 1 min");

						while(g_tick < 10);
						{

						}

						/*stop timer1 via using deInit() Function*/
						Timer_DeInit(Timer1);

						/*Clear the g_tick for next time*/
						g_tick=0;

						/*go to MAIN_MENU_OPTION_STAGE where the user choose again what he want*/
						g_MC1_application_sequence=MAIN_OPTIONS_STAGE;
					}
				}
			break;
			}
			case OPEN_DOOR_STAGE:
			{
				/*
				   F_CPU= 8 MHZ and Timer Prescaler = 1024
				  so we need 117188 count from timer to time 15 sec
				  so we will put in compare register of timer1 value = 58594
				  we will need 2 interrupts for time 15 sec
				  NOTE Timer Timer_configuration its prescaler and ID inital value
				*/
				Timer_configuration.timer_compare_MatchValue=58594;

				/*Initialize Timer 1*/
				Timer_init(&Timer_configuration);

				/*Display Unlocking the door*/
				LCD_clearScreen();
				LCD_displayStringRowColumn(0,1,"Door Unlocking");
				LCD_displayStringRowColumn(1,4,"Please Wait");

				/*Wait for 15 sec Displaying DOOR UNLOCKING*/
				while(g_tick<2){};

				/*Stop Timer1*/
				Timer_DeInit(Timer1);

				/*Clear the g_tick for next time*/
				g_tick=0;

				LCD_clearScreen();

				Timer_configuration.timer_compare_MatchValue=23438;

				Timer_init(&Timer_configuration);

				LCD_clearScreen();
				LCD_displayStringRowColumn(0,0,"wait for people");
				LCD_displayStringRowColumn(1,3,"To Enter");

				while(g_tick<1){};

				recieve_Byte();

				/*Stop Timer1*/
				Timer_DeInit(Timer1);

				g_tick=0;

				/*Display locking the door*/
				LCD_clearScreen();
				LCD_displayStringRowColumn(0,2,"Door Locking");

				Timer_configuration.timer_compare_MatchValue=58594;
				/*Initialize Timer 1*/
				Timer_init(&Timer_configuration);

				/*Wait for 15 sec*/
				while(g_tick<2){};

				/*Stop Timer1*/
				Timer_DeInit(Timer1);

				/*Clear the g_tick for next time*/
				g_tick=0;

				/*go to MAIN_OPTIONS_STAGE where the user choose again what he want*/
				g_MC1_application_sequence=MAIN_OPTIONS_STAGE;
				break;

			}
		}
		}
}

                   /****** Functions Definitions******/
void create_Password(uint8 *a_PasswordArray)
{
	uint8 counter=0;
	uint8 pressedKey=KEYPAD_getPressedKey();
	while(pressedKey != '=')
	{
		pressedKey=KEYPAD_getPressedKey();
		if( ( pressedKey != '+' && pressedKey!= '-' && pressedKey!= '*' && pressedKey!= '%' && pressedKey!= '=') )
		{
			if(counter < PASSWORD_SIZE )
			{
				*(a_PasswordArray+counter)=pressedKey;
				LCD_displayCharacter('*');
				counter++;
			}
			_delay_ms(500);
		}
	}

}

/* Sending Password entered by the user from HMI_ECU TO Control_ECU through UART*/
void send_Password(uint8 *Password_ptr1)
{
	uint8 i;
	UART_sendByte(READY);
	while(UART_recieveByte()!=READY);
	for(i=0;i<PASSWORD_SIZE;i++)
	{
		UART_sendByte(Password_ptr1[i]);
	}
	while(UART_recieveByte()!=DONE);

}


/*1-Sending Password entered by the user from HMI_ECU TO Control_ECU through UART*/
uint8 Password_Checker(void)
{
	uint8 state=-1;
	state= recieve_Byte();
	return state;
}

/*  Receiving data from the UART in the HMI_ECU*/
uint8 recieve_Byte(void)
{
	uint8 byte;
	while(UART_recieveByte() != READY){}
	UART_sendByte(READY);
	byte=UART_recieveByte();
	UART_sendByte(DONE);
	return byte;
}
/* Sending data to the UART in the HMI_ECU*/
void send_Byte(uint8 byte)
{
	UART_sendByte(READY);
	while(UART_recieveByte() != READY){}
	UART_sendByte(byte);
	while(UART_recieveByte() != DONE){}
}

/*
 * Function Description :
 * used to increment a global variable to indicate number of interrupts happend for timer1
 */
void timer1_SetRequiredCallBackFunction(void)
{
	g_tick++;
}

