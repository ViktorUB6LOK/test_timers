/*
 * dwin_app.h
 *
 *  Created on: Nov 30, 2025
 *      Author: Big_Viktor
 */

#ifndef DWIN_APP_H_
#define DWIN_APP_H_

#include "main.h"
#include "tim.h"
#include "dwin.h"
#include "ad9833_app.h"
#include "setting.h"


struct DWIN_VAR {             // объявление структура в main.c
	   uint16_t adress;
	   uint16_t data;
};

void DWIN_Set_var(void);
void DWIN_Start_page(void);
void DWIN_Select_mode (void);
void func_1 (void);            // flowmeter
void func_2 (void);            // speedmeter
void func_3 (void);            // flowmeter_speedmeter
void func_4 (void);            // IO flowmeter

#endif /* DWIN_APP_H_ */
