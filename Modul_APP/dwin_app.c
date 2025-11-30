/*
 * dwin_app.c
 *
 *  Created on: Nov 30, 2025
 *      Author: Big_Viktor
 */
#include "dwin_app.h"

void DWIN_Set_var()            // установка переменных в DWIN
{
	writeHalfWordDWIN(dwin_adress_flowmeter, 100);
	HAL_Delay(50);    // задержка не успевает передать
	writeHalfWordDWIN(dwin_adress_speedmeter, 100);
	HAL_Delay(50);
	writeHalfWordDWIN(dwin_adress_flowmeter_setting, 640);
	HAL_Delay(50);    // задержка не успевает передать
	writeHalfWordDWIN(dwin_adress_speedmeter_setting, 160);
	HAL_Delay(50);
}

void DWIN_Start_page() {
	writeWordDWIN(0x0004, 0x55aa5aa5); // сброс DWIN
	HAL_Delay(3000);
	DWIN_Set_var();
	goToPageDWIN(1);
}
