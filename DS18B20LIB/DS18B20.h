/*
 * DS18B20.h
 *
 *  Created on: Jan 3, 2021
 *      Author: Łukasz
 */

#ifndef DS18B20_H_
#define DS18B20_H_
#include "main.h"
#include "stm32f1xx.h"

#define DS18B20_PORT DS18B20_GPIO_Port
#define DS18B20_PIN DS18B20_Pin

void delay (uint16_t time);
void DS18B20_Full(double *Temperature);
uint8_t DS18B20_Read (void);
void DS18B20_Write (uint8_t data);
uint8_t DS18B20_Start (void);
void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);


#endif /* DS18B20_H_ */
