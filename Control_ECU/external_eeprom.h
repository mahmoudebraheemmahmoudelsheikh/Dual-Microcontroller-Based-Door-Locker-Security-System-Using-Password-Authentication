/*
 * external_eeprom.h
 *
 *  Created on: Oct 25, 2024
 *      Author: Tweety
 */
#ifndef EXTERNAL_EEPROM_H_
#define EXTERNAL_EEPROM_H_

#include "std_types.h"

              /***************Preprocessor Macros****************/
#define ERROR 0
#define SUCCESS 1

               /*************Functions Prototypes*****************/

uint8 EEPROM_writeByte(uint16 u16addr,uint8 u8data);
uint8 EEPROM_readByte(uint16 u16addr,uint8 *u8data);

void EEPROM_writeArray(uint16 startaddress,uint8 *array,uint8 a_arraySize);
void EEPROM_readArray(uint16 startaddress,uint8 *array,uint8 a_arraysize);

#endif /* EXTERNAL_EEPROM_H_ */
