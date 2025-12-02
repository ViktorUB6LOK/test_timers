/*
 * dwin_app.c
 *
 *  Created on: Nov 30, 2025
 *      Author: Big_Viktor
 */
#include "dwin_app.h"

extern bool flag_dwin_tx_IT;
extern struct readDataDWIN_P readDataDWIN;

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
	HAL_Delay(5000);
	DWIN_Set_var();
	goToPageDWIN(1);
	HAL_Delay(400);
}

void DWIN_Select_mode() { // перенос в основной цикл
	bool flag_select_mode = false;
	uint16_t adress_parsing = 0;
	uint16_t data_parsing   = 0;
//	dwinUartDmaInit();

	while (!flag_select_mode){
		if (flag_dwin_tx_IT) {
				flag_dwin_tx_IT = false;
				parsingDWIN();
				adress_parsing = readDataDWIN.parsingDataDWIN.data[0] << 8
										| readDataDWIN.parsingDataDWIN.data[1];
				data_parsing = readDataDWIN.parsingDataDWIN.data[3] << 8
										| readDataDWIN.parsingDataDWIN.data[4];
				flag_select_mode = true;
		}
	}

	if (adress_parsing == 0x5001) {
		switch (data_parsing) {
		      case (0x0001):
		          goToPageDWIN(2);
		    	  break;
		      case (0x0002):
		          goToPageDWIN(3);
		    	  break;
		      case (0x0003):
		          goToPageDWIN(4);
		      	  break;
		      default:
		    	  break;
	}
  }
}
