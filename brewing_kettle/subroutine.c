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
	} else if (strncmp((char*) buffor, "LIST", 4) == 0) { //------------LIST--------------
		show(*prog);
	} else if (strncmp((char*) buffor, "READ", 4) == 0) {

	} else if (strncmp((char*) buffor, "SET", 3) == 0) { //-------------SET-------------
		msg = przewinDo(msg, '[');
		msg = conv(msg, &tempData.ID);
		msg = przewinDo(msg, '[');
		uint8_t i = 0;
		while (!(*(++msg) == ']'))
			tempData.name[i++] = *msg; // tymczasowa nazwa
		//----------odszyfrowanie temperatur----------
		for (uint8_t i = 0; i < 5; i++) {
			msg = readTemperature(msg, &tempData, i);
		}

	} else if (strncmp((char*) buffor, "ACTIVE", 6) == 0) { //----------ACTIVE-------------

	}

	//----------Przypisanie tempData do wejsciowej listy
	(*prog)->data = tempData;
}
uint8_t* przewinDo(uint8_t *msg, uint8_t znak) {
	while (!(*(++msg) == znak))
		;
	return msg;
}
uint8_t* readTemperature(uint8_t *msg, Subroutine *dataTemp,
		uint8_t currentCycle) {
	//--------jezeli poprawnie uzyte---------
	if ((*msg) != '\0' && strstr((char*) msg, "S")) {
		//-------przewiń do S------
		while ((*msg++) != 'S')
			;
		//-------PID lub dwustawna--------
		dataTemp->regType = *msg++ == 0 ? false : true;

		//-------Wypełnienie cykli--------

		/*tymczasowa convna temp służy do
		 zamienienia kilkucyfrowego ciągu znaków w liczbę*/
		while (*msg != '\0') {

			//-----Pobranie temperatury------
			if (*msg == 'T') {
				msg = conv(msg, &dataTemp->heatingCycle[currentCycle][0]);

				//---------Pobranie czasu-------
			} else if (*msg == 't') {
				msg = conv(msg, &dataTemp->heatingCycle[currentCycle][1]);
			} else if (*msg == 'H') { //histereza

			} else if (*msg == 'P') {
				msg = conv(msg, &dataTemp->pumpingTime);
			}
		}
	}
	return msg; //zwroc wskaznik w ostatnim polozeniu
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
	uint8_t tekst1[]="List is empty\n\r";
	uint8_t tekst[100];
	if (head == NULL)
		HAL_UART_Transmit_DMA(&huart2, (char*)tekst1, sizeof(tekst1));
	else {
		List *current = head;
		do {
			uint8_t size1;
			size1=sprintf(tekst,"ID: %d\n\rNazwa: %s\c\rTyp regulacji: %s\n\r",current->data.ID,current->data.name,current->data.regType==1?"PID":"Dwustawna");


			HAL_UART_Transmit_DMA(&huart2, tekst, sizeof(tekst1));

			current = current->next;
		} while (current != NULL);

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
