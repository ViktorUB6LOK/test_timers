/*
 * dwin_app.h
 *
 *  Created on: Nov 30, 2025
 *      Author: Big_Viktor
 */

#ifndef DWIN_APP_H_
#define DWIN_APP_H_

#include "main.h"
#include "dwin.h"

#define dwin_adress_flowmeter  0x1000
#define dwin_adress_speedmeter 0x2000
#define dwin_adress_flowmeter_setting  0x1010
#define dwin_adress_speedmeter_setting 0x2010

struct DWIN_VAR {             // объявление структура в main.c
	   uint16_t adress;
	   uint16_t data;
};

void DWIN_Set_var(void);
void DWIN_Start_page(void);


#endif /* DWIN_APP_H_ */
