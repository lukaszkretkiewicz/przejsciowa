/*
 * subroutine.c
 *
 *  Created on: Nov 30, 2020
 *      Author: Łukasz
 */
#include "subroutine.h"
#include <string.h>
#include<ctype.h>
#include<stdlib.h>
#include<main.h>
#include<stm32f1xx.h>
void setProgram(uint8_t *msg, struct Subroutine *data) {
	uint8_t buffor[10];
	uint8_t i = 0;

	//wypelnienie tablicy name[] i heatingCycle zerami

	//przewin do ' ' i pobierz komendę do buffora, aby ją porównać
	while ((buffor[i++] = *msg++) != ' ')
		;
	if (strncmp((char*) buffor, "SET", 3) == 0) { //-------------SET-------------
		msg = przewinDo(msg, ':');

		//----------odszyfrowanie temperatur----------
		for (uint8_t i = 0; i < 5; i++) {
			msg = readTemperature(msg, data, i);
		}
	}

}
void convertToStucture(uint8_t *msg, struct List **prog) {
	uint8_t buffor[10];
	uint8_t i = 0;
	Subroutine tempData;
	static uint8_t ID = 0;
	//wypelnienie tablicy name[] i heatingCycle zerami
	for (int i = 0; i < sizeof(tempData.name); i++) {
		tempData.name[i] = 0;
	}
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 2; j++)
			tempData.heatingCycle[i][j] = 0;
	tempData.pumpingTime = 0;
	//przewin do ' ' i pobierz komendę do buffora, aby ją porównać
	while ((buffor[i++] = *msg++) != ' ')
		;
	//-----------odszyfrowanie komend--------------
	if (strncmp((char*) buffor, "NEW", 3) == 0) { //------------NEW---------
		//stworzenie nowego programu na liscie

		msg = przewinDo(msg, '[');
		uint8_t i = 0;
		while (!(*(++msg) == ']'))
			tempData.name[i++] = *msg; // tymczasowa nazwa
//----------odszyfrowanie temperatur----------
		for (uint8_t i = 0; i < 5; i++) {
			msg = readTemperature(msg, &tempData, i);
		}
//----------nadanie nowego ID------------
		tempData.ID = ID;
		ID++;
		push_front(prog, tempData);
		(*prog)->data = tempData;
	} else if (strncmp((char*) buffor, "LIST", 4) == 0) { //------------LIST--------------
		show(*prog);
	} else if (strncmp((char*) buffor, "READ", 4) == 0) {

	} else if (strncmp((char*) buffor, "SET", 3) == 0) { //-------------SET-------------
		msg = przewinDo(msg, ':');

		//----------odszyfrowanie temperatur----------
		for (uint8_t i = 0; i < 5; i++) {
			msg = readTemperature(msg, &tempData, i);
		}

	} else if (strncmp((char*) buffor, "ACTIVE", 6) == 0) { //----------ACTIVE-------------
		uint8_t tempID;
		msg = przewinDo(msg, '[');
		msg = conv(msg, &tempID);
		Subroutine *p; // wskaznik do konkretnego programu, ktory bedzie aktywowany
		p = szukajID(*prog, tempID);

		activeBrewing(*p);
	}
//--------przewin msg do konca------------
	msg = przewinDo(msg, '\0');
}
Subroutine* szukajID(List *prog, uint8_t ID) {
	List *current = prog;
	for (int i = 0; i < list_size(prog); i++, current = current->next) {
		if ((prog)->data.ID == ID) {
			return &(prog)->data;
		}
	}
	return NULL;
}
void activeBrewing(Subroutine data) {
	//-----pętla grzania-------
	if (data.regType == 0) {
		__HAL_TIM_SET_COUNTER(&htim1, 0);
		startBangBang = true;
		startPIDReg = false;
		//grzanieRegDwustawna(data.heatingCycle[i], data.hist);
	} else {
		__HAL_TIM_SET_COUNTER(&htim1, 0);
		startBangBang = false;
		startPIDReg = true;
		//grzanieRegPID(data.heatingCycle[i]);
	}

}
void subroutine_Init(struct Subroutine *data) {
	startPIDReg = false;
	startBangBang = false;
	startPumping = false;
	for (int i = 0; i < sizeof(data->name); i++) {
		data->name[i]=0;

		}
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 2; j++)
				data->heatingCycle[i][j] = 0;
		data->pumpingTime = 0;
}
void pumping(uint8_t timeOfPumping) {
	if (__HAL_TIM_GET_COUNTER(&htim1) <= timeOfPumping * 60) {
		HAL_GPIO_WritePin(POMPKA_PORT, POMPKA_PIN, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(POMPKA_PORT, POMPKA_PIN, GPIO_PIN_RESET);
		startPumping = false;
	}
}
void grzanieRegDwustawna(uint8_t setTemperature, uint8_t timeOfHeating,
		double hysteresis, uint8_t *numberOfCycle) {
	extern bool isHeating;
	//extern uint32_t startCounterTime;
	extern double measuredTemperature;
	//uint32_t beginingOfHeating = startCounterTime;

	if (__HAL_TIM_GET_COUNTER(&htim1) <= timeOfHeating * 60) {
		if ((double) setTemperature <= measuredTemperature - hysteresis) {
			isHeating = true;
			HAL_GPIO_WritePin(GRZALKA_PORT, GRZALKA_PIN, GPIO_PIN_SET);
		} else if ((double) setTemperature
				>= measuredTemperature + hysteresis) {
			isHeating = false;
			HAL_GPIO_WritePin(GRZALKA_PORT, GRZALKA_PIN, GPIO_PIN_RESET);
		}
	} else {
		__HAL_TIM_SET_COUNTER(&htim1, 0);
		if (*numberOfCycle < 4)
			(*numberOfCycle)++;
		else {
			numberOfCycle = 0;
			startBangBang = false;
			startPumping = true;
			__HAL_TIM_SET_COUNTER(&htim1, 0);
		}
	}
}
void grzanieRegPID(uint8_t *heatingCycle) {

}
uint8_t* przewinDo(uint8_t *msg, uint8_t znak) {
	while (!(*(++msg) == znak))
		;
	return msg;
}
uint8_t* readTemperature(uint8_t *msg, Subroutine *dataTemp,
		uint8_t currentCycle) {
//--------jezeli faktycznieeee poprawnie uzyte----------
	if ((*msg) != '\0') {
		//-------przewiń do S------
		if (strstr((char*) msg, "S")) {
			while ((*msg++) != 'S')
				;
			//-------PID lub dwustawna--------
			dataTemp->regType = *msg == 0 ? BANGBANG : PID;
			msg = przewinDo(msg, ';');
			*msg++;
		}
		//-------Wypełnienie cykli--------

		/*tymczasowa convna temp służy do
		 zamienienia kilkucyfrowego ciągu znaków w liczbę*/
		while (*msg != ';') {

			//-----Pobranie temperatury------
			if (*msg == 'T') {
				msg = conv(msg, &dataTemp->heatingCycle[currentCycle][0]);

			} else if (*msg == 'H') { //histereza
				msg = convDouble(msg, &dataTemp->hist);
				//---------Pobranie czasu-------
			} else if (*msg == 't') {
				msg = conv(msg, &dataTemp->heatingCycle[currentCycle][1]);
			} else if (*msg == 'P') {
				msg = conv(msg, &dataTemp->pumpingTime);
			} else
				break;

		}
	}
	return (*msg) != '\0' ? ++msg : msg; //zwroc wskaznik w ostatnim polozeniu
}
uint8_t* conv(uint8_t *msg, uint8_t *digit) {
	uint8_t temp[3] = { 'a' };
	uint8_t i;
	++msg;
	for (i = 0; isdigit(*msg); i++) {
		temp[i] = *msg++;
	}
	*digit = atoi((char*) temp);
	return msg;
}
uint8_t* convDouble(uint8_t *msg, double *digit) {
	uint8_t temp[4] = { 'a' };
	uint8_t i;
	++msg;
	for (i = 0; isdigit(*msg) || *msg == '.' || *msg == ','; i++) {
		temp[i] = *msg++;
	}
	(*digit) = atof((char*) temp);
	return msg;
}
void push_front(List **head, Subroutine data) {
	List *current;
	current = (List*) malloc(sizeof(List));

	current->data = data;
	current->next = (*head);
	*head = current;
}
void push_back(List **head, Subroutine data) {
	if (*head == NULL) {
		*head = (List*) malloc(sizeof(List));

		(*head)->next = NULL;
	} else {
		List *current = *head;

		while (current->next != NULL) {
			current = current->next;
		}

		current->next = (List*) malloc(sizeof(List));
		current->next->next = NULL;
	}
}
void show(List *head) {
	char tekst1[] = "List is empty\n\r";
	if (head == NULL)
		HAL_UART_Transmit_DMA(&huart2, tekst1, sizeof(tekst1));
	else {
		List *current = head;
		uint8_t *wysylanaWiadomosc;
		wysylanaWiadomosc = (uint8_t*) malloc(
				list_size(head) * sizeof(uint8_t)
						* strlen("ID: %d\n\rNazwa: %s\n\rTyp regulacji: %s\n\r")
						+ 13);
		/*Przez to, że \s to raz PID, raz łańcuch o innej dlugosci, to niepoprawnie dobierana jest rzeczywista
		 *  dlugosc calkowitego lancucha znakow i malloc dostaje niepoprawny rozmiar.
		 *  mozna sprobowac np pisac size+=spintf(buffer[100]...) i na tym size pozniej zrobic malloc. 		 */

		uint8_t i = 0;
		do {
			if (i == 0) {
				sprintf((char*) wysylanaWiadomosc,
						"ID: %d\n\rNazwa: %s\n\rTyp regulacji: %s\n\r",
						current->data.ID, current->data.name,
						current->data.regType == 1 ? "PID" : "Dwustawna");
			} else {
				sprintf(
						(char*) wysylanaWiadomosc
								+ strlen((char*) wysylanaWiadomosc),
						"ID: %d\n\rNazwa: %s\n\rTyp regulacji: %s\n\r",
						current->data.ID, current->data.name,
						current->data.regType == 1 ? "PID" : "Dwustawna");
			}

			current = current->next;
			i++;
		} while (current != NULL);

		HAL_UART_Transmit_DMA(&huart2, wysylanaWiadomosc,
				strlen((char*) wysylanaWiadomosc));
		free(wysylanaWiadomosc);
	}
}
uint16_t list_size(List *head) {
	uint16_t counter = 0;
	if (head == NULL)
		return counter;
	else {
		List *current = head;
		do {
			counter++;
			current = current->next;
		} while (current != NULL);
	}
	return counter;
}
