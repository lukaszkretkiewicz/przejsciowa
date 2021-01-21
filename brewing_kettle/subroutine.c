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

	char buffor[10] = "";
	uint8_t i = 0;
	char buffor2[62]="";
	strcpy(buffor2,(char*)msg);
	memset(buffor, '\0', sizeof(buffor));
	strncpy(buffor, (char*) msg, 4);
	if (strncmp((char*) msg, "SET", 3) == 0) { //-------------SET-------------
		msg = przewinDo(msg, ':');

		//----------odszyfrowanie temperatur----------

		for (uint8_t i = 0; i < 6; i++) {
			msg = readTemperature(msg, data, i);
		}
		activeBrewing(*data);
	} else if (strncmp((char*) msg, "STOP", 4) == 0) { //--------STOP--------------
		subroutine_Init(data);
	} else if (strstr((char*) msg, "P1")) { //----------P1-------------
		subroutine_Init(data);
		extern uint32_t startCounterTime;
		startCounterTime = 0;
		data->pumpingTime = 30;
		startPumping = 1;
	} else if (strstr((char*) msg, "P0")) { //-----------P0--------------
		data->pumpingTime = 0;
		startPumping = 0;
		HAL_GPIO_WritePin(POMPKA_PORT, POMPKA_PIN, GPIO_PIN_RESET);
	} else {

	}

}
void convertToStucture(uint8_t *msg, struct List **prog) {
	uint8_t buffor[10];
	uint8_t i = 0;
	Subroutine tempData;
	static uint8_t ID = 0;
//wypelnienie tablicy name[] i heatingCycle zerami
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

//----------odszyfrowanie temperatur----------
		for (uint8_t i = 0; i < 5; i++) {
			msg = readTemperature(msg, &tempData, i);
		}

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

	}
//--------przewin msg do konca------------
	msg = przewinDo(msg, '\0');
}
void activeBrewing(Subroutine data) {
//-----pętla grzania-------
	extern uint32_t CounterHeating;
	CounterHeating = 0;
	if (data.regType == BANGBANG) {
		startBangBang = true;
		startPIDReg = false;
		//grzanieRegDwustawna(data.heatingCycle[i], data.hist);
	} else {
		startBangBang = false;
		startPIDReg = true;
		//grzanieRegPID(data.heatingCycle[i]);
	}

}
void subroutine_Init(struct Subroutine *data) {
	startPIDReg = false;
	startBangBang = false;
	startPumping = false;

	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 2; j++)
			data->heatingCycle[i][j] = 0;
	data->pumpingTime = 0;
	HAL_GPIO_WritePin(POMPKA_PORT, POMPKA_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GRZALKA_PORT, GRZALKA_PIN, GPIO_PIN_RESET);
}
void pumping(uint8_t timeOfPumping) {
	extern uint32_t CounterPump;
	if (CounterPump <= timeOfPumping * 60) {
		HAL_GPIO_WritePin(POMPKA_PORT, POMPKA_PIN, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(POMPKA_PORT, POMPKA_PIN, GPIO_PIN_RESET);
		startPumping = false;
	}
}
void grzanieRegDwustawna(uint8_t setTemperature, uint8_t timeOfHeating,
		double hysteresis, uint8_t *numberOfCycle) {
	extern bool isHeating;
	extern double measuredTemperature;
	extern uint32_t CounterHeating;
	if (CounterHeating <= timeOfHeating * 60) {
		if ((double) setTemperature >= measuredTemperature - hysteresis) {
			isHeating = true;
			HAL_GPIO_WritePin(GRZALKA_PORT, GRZALKA_PIN, GPIO_PIN_SET);
		} else if ((double) setTemperature < measuredTemperature + hysteresis) {
			isHeating = false;
			HAL_GPIO_WritePin(GRZALKA_PORT, GRZALKA_PIN, GPIO_PIN_RESET);
		}
	} else {
		HAL_GPIO_WritePin(GRZALKA_PORT, GRZALKA_PIN, GPIO_PIN_RESET);
		CounterHeating = 0;
		if (*numberOfCycle < 4)
			(*numberOfCycle)++;
		else {
			*numberOfCycle = 0;
			startBangBang = false;
			startPumping = true;
			extern uint32_t CounterPump;
			CounterPump = 0;
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
			dataTemp->regType = *msg == '0' ? BANGBANG : PID;
			if (dataTemp->regType == PID)
				msg = przewinDo(msg, 'T');
			else if (dataTemp->regType == BANGBANG) { //histereza
				msg = przewinDo(msg, 'H');
				msg = convDouble(msg, &dataTemp->hist);
				*msg++;
			}

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
		return (*msg) != '\0' ? ++msg : msg; //zwroc wskaznik w ostatnim polozeniu
	} else
		return msg = NULL;
	//return (*msg) != '\0' ? ++msg : msg; //zwroc wskaznik w ostatnim polozeniu
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
char* textConversion(double dTemperature, char* targetVariable) {
	char* temp;
	temp = malloc(sizeof(char) * 25);
	char sTemperature[10]="";
	extern double measuredTemperature;
	strcpy(temp, targetVariable);
	sprintf(temp+strlen(temp), ".val=");
	itoa((int)(10.0 * measuredTemperature), sTemperature, 10);
	sprintf(temp + strlen(temp), sTemperature);

	return temp;
}

/*T_aktualna.val=50
 255
 255
 255
 */
