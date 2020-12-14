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
typedef struct Subroutine {
	uint8_t ID; //ID programu
	char name[20]; //nazwa programu
	uint8_t heatingCycle[5][2]; //tablica temperatur oraz odpowiadających im czasów grzania
								//(jeżeli cykl składa się z mniej niż 5 przedziałów, to w miejscu temperatury i czasu wpisane są 0)
								//pierwsza kolumna to temperatura, druga to czas
	uint8_t pumpingTime; //czas wysładzania na końcu cyklu (0, jeżeli ma być nieaktywne)
	bool regType; // typ regulacji
	double hist; //wartość histerezy
} Subroutine;
typedef struct List {
	struct Subroutine data;
	struct List *next;
} List;

uint8_t* readTemperature(uint8_t *msg, Subroutine *dataTemp,uint8_t currentCycle); //Konwertuje ciąg S...T... na dane
void push_front(List **head, Subroutine data);
void push_back(List **head, Subroutine data);
void show(List *head);
uint16_t list_size(List *head);
void convertToStucture(uint8_t *msg, struct List **prog); //funkcja zamieniająca tekst z wyswietlacza na dane struktury
void activeBrewing(Subroutine data); // rozpoczęcie danego programu warzelnego
Subroutine* szukajID(List *prog,uint8_t ID);

uint8_t *conv(uint8_t *msg,uint8_t *digit); //zamienia cyfry w ciągu na uint8_t
uint8_t *przewinDo(uint8_t *msg,uint8_t znak);
#endif /* SUBROUTINE_H_ */

