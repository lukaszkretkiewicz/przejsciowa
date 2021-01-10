/*
 * subroutine.h
 *
 *  Created on: Nov 30, 2020
 *      Author: Łukasz
 */

#ifndef SUBROUTINE_H_
#define SUBROUTINE_H_
#include "stdio.h"
#include "stm32f1xx_hal.h"
#include "usart.h"
#include <stdbool.h>
#include "tim.h"
// FLAGI
bool startPIDReg;
bool startBangBang;
bool startPumping;


#define GRZALKA_PORT Grzalka_GPIO_Port
#define GRZALKA_PIN Grzalka_Pin

#define POMPKA_PORT Pompka_GPIO_Port
#define POMPKA_PIN Pompka_Pin
typedef enum {
		BANGBANG=0,
		PID=1
	}RegType;// typ regulacji
typedef struct Subroutine {
	uint8_t ID; //ID programu
	char name[20]; //nazwa programu
	uint8_t heatingCycle[5][2]; //tablica temperatur oraz odpowiadających im czasów grzania
								//(jeżeli cykl składa się z mniej niż 5 przedziałów, to w miejscu temperatury i czasu wpisane są 0)
								//pierwsza kolumna to temperatura, druga to czas
	uint8_t pumpingTime; //czas wysładzania na końcu cyklu (0, jeżeli ma być nieaktywne)
	RegType regType;
	//bool regType; // typ regulacji
	double hist; //wartość histerezy
} Subroutine;
typedef struct List {
	struct Subroutine data;
	struct List *next;
} List;
void subroutine_Init(struct Subroutine *data);
void setProgram(uint8_t *msg, struct Subroutine *data);
uint8_t* readTemperature(uint8_t *msg, Subroutine *dataTemp,uint8_t currentCycle); //Konwertuje ciąg S...T... na dane
void push_front(List **head, Subroutine data);
void push_back(List **head, Subroutine data);
void show(List *head);
uint16_t list_size(List *head);
void convertToStucture(uint8_t *msg, struct List **prog); //funkcja zamieniająca tekst z wyswietlacza na dane struktury
void activeBrewing(Subroutine data); // rozpoczęcie danego programu warzelnego
Subroutine* szukajID(List *prog,uint8_t ID);
void grzanieRegDwustawna(uint8_t setTemperature, uint8_t timeOfHeating,
		double hysteresis, uint8_t* numberOfCycle);
void grzanieRegPID(uint8_t *heatingCycle);
void pumping(uint8_t timeOfPumping);
uint8_t *conv(uint8_t *msg,uint8_t *digit); //zamienia cyfry w ciągu znakow na uint8_t
uint8_t* convDouble(uint8_t *msg, double *digit); //zamienia cyfry w ciągu znakow na double
uint8_t *przewinDo(uint8_t *msg,uint8_t znak);
#endif /* SUBROUTINE_H_ */

