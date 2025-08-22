/*
 * control_ecu.h
 *
 *  Created on: Oct 30, 2024
 *      Author: Tweety
 */
#ifndef CONTROL_ECH_H_
#define CONTROL_ECH_H_

#include "std_types.h"

/************************Definitions******************/
#define RECEIVING_PASSWORDS_STAGE 			0
#define CHECK_PASSWORDS_MATCHING_STAGE 		1
#define SAVE_PASSWORD_INTO_EEPROM_STAGE 	2
#define MAIN_MENU_OPTION_STAGE 				3
#define OPEN_DOOR_STAGE						4

#define PASSWORD_SIZE 						5

#define MATCHED 						100
#define NOT_MATCHED 					50

                       /*************Functions Prototypes****************/


/* used to increment a global variable to indicate number of interrupts happend for timer1*/

void timer1_SetRequiredCallBackFunction(void);


 /* Receiving Password entered by the user from HMI_ECU and store it in array defined global*/

void receive_Password(uint8* a_PasswordArray);


 /* return if they are matched or not*/

uint8 check_TwoPasswords(uint8 *Password_array1,uint8 *Password_array2);


 /* Receiving data from the UART in the HMI_ECU*/

uint8 recieve_Byte(void);


 /* Sending data to the UART in the HMI_ECU*/

void send_Byte(uint8 byte);

#endif /* CONTROL_ECH_H_ */
