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

void DWIN_Select_mode() {
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
		      case (0x0001):              // режим 1 (генератор flowmeter)
		          goToPageDWIN(2);        // переключение на страницу режима 1
		          func_1 ();
		    	  break;
		      case (0x0002):              // режим 2 (генератор flowmeter)
		          goToPageDWIN(3);        // переключение на страницу режима 2
		    	  break;
		      case (0x0003):              // режим 3 (генератор flowmeter)
		          goToPageDWIN(4);        // переключение на страницу режима 3
		      	  break;
		      default:
		    	  break;
	}
  }
}

void func_1 (){
	    bool flag_func_1 = false;
		uint16_t adress_parsing = 0;
		uint16_t data_parsing   = 0;
		uint16_t setting_ratio_flowmeter = 0;    // коэфф. литр в мин - возможно они не нужны
		uint16_t data_flowmeter = 0;             // поток  - возможно они не нужны

		while (!flag_func_1){                         // пока не будет нажата кнопка выбора режима крутимся в цикле
			if (flag_dwin_tx_IT) {
							flag_dwin_tx_IT = false;
							parsingDWIN();
							adress_parsing = readDataDWIN.parsingDataDWIN.data[0] << 8
													| readDataDWIN.parsingDataDWIN.data[1];
							data_parsing = readDataDWIN.parsingDataDWIN.data[3] << 8
													| readDataDWIN.parsingDataDWIN.data[4];

							switch (adress_parsing) {
									      case (dwin_adress_flowmeter_setting):      // если нажали на выбор коэфф. литр/мин
									    		  setting_ratio_flowmeter = data_parsing;
									    		  send_data_to_ad9833 (data_flowmeter, setting_ratio_flowmeter);

									    		  // передаем в функцию обработки поток + коэфф.
									    	  break;
									      case (dwin_adress_flowmeter):            // если нажата - изменяем поток и даем команду на AD9833
		                                           data_flowmeter = data_parsing;
		                                           send_data_to_ad9833 (data_flowmeter, setting_ratio_flowmeter);

	                                            	// передаем в функцию обработки поток + коэфф.
									    	  break;
									      case (dwin_adress_button_back):          // если нажата выбор режима - true и выходим из функции
	                                           	flag_func_1 = true;
									      	  break;
									      default:
									    	  break;
								}
				}
		}
}
