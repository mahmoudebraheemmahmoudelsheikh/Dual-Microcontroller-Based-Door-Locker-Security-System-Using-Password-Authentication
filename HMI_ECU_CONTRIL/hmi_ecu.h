/*
 * hmi_ecu.h
 *
 *  Created on: Oct 31, 2024
 *      Author: Tweety
 */
#ifndef HMI_ECU_H_
#define HMI_ECU_H_

#include "std_types.h"

#define CREATE_PASSWORD_STAGE 			  0
#define CHECKING_PASSWORDS_MATCHING_STAGE 1
#define MAIN_OPTIONS_STAGE 				  2
#define OPEN_DOOR_STAGE 				  3

#define PASSWORD_SIZE 					  5

#define MATCHED 						100
#define NOT_MATCHED 					50


void create_Password(uint8 *a_PasswordArray);

void send_Password(uint8 *Password_ptr1);


uint8 Password_Checker(void);

void timer1_SetRequiredCallBackFunction(void);


uint8 recieve_Byte(void);


void send_Byte(uint8 byte);

#endif /* HMI_ECU_H_ */
