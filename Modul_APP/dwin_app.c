/*
 * dwin_app.c
 *
 *  Created on: Nov 30, 2025
 *      Author: Big_Viktor
 */
#include "dwin_app.h"

extern bool flag_dwin_tx_IT;
extern struct readDataDWIN_P readDataDWIN;

void DWIN_Start_page() {
	writeWordDWIN(0x0004, 0x55aa5aa5); // сброс DWIN
	HAL_Delay(3000);
	goToPageDWIN(1);
	HAL_Delay(400);
}

void DWIN_Select_mode() {
	bool flag_select_mode = false;
	uint16_t adress_parsing = 0;
	uint16_t data_parsing   = 0;

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
		          func_2 ();
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
		static uint16_t setting_ratio_flowmeter = 600;
		static uint16_t data_flowmeter = 10;
		// начальные установки -----------------------------------------------------------
		writeHalfWordDWIN(dwin_adress_flowmeter, data_flowmeter);
		HAL_Delay(10);
		writeHalfWordDWIN(dwin_adress_flowmeter_setting, setting_ratio_flowmeter);
		HAL_Delay(10);
		send_data_to_ad9833 (data_flowmeter, setting_ratio_flowmeter);
		// -------------------------------------------------------------------------------

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
									      case (dwin_adress_button_back): // если нажата выбор режима - true и выходим из функции
									    		  // дать команду на остановку генератора
									    		if (data_parsing == 0) goToPageDWIN(1);
	                                           	flag_func_1 = true;
									      	  break;
									      default:
									    	  break;
								}
				}
		}
}

void func_2 (){
	    bool flag_func_2 = false;
		uint16_t adress_parsing = 0;
		uint16_t data_parsing   = 0;
		static uint16_t data_speedmeter = 2;
		static uint16_t setting_ratio_speedmeter = 160;

		// начальные установки -----------------------------------------------------------
		writeHalfWordDWIN(dwin_adress_speedmeter, data_speedmeter);
		HAL_Delay(10);
		writeHalfWordDWIN(dwin_adress_speedmeter_setting, setting_ratio_speedmeter);
		HAL_Delay(10);
		send_data_to_ad9833_2 (data_speedmeter, setting_ratio_speedmeter);
		// -------------------------------------------------------------------------------

		while (!flag_func_2){                         // пока не будет нажата кнопка выбора режима крутимся в цикле
			if (flag_dwin_tx_IT) {
							flag_dwin_tx_IT = false;
							parsingDWIN();
							adress_parsing = readDataDWIN.parsingDataDWIN.data[0] << 8
													| readDataDWIN.parsingDataDWIN.data[1];
							data_parsing = readDataDWIN.parsingDataDWIN.data[3] << 8
													| readDataDWIN.parsingDataDWIN.data[4];

							switch (adress_parsing) {
									      case (dwin_adress_speedmeter_setting):      // если нажали на выбор коэфф. имп/100м
									    		  setting_ratio_speedmeter = data_parsing;
									    		  send_data_to_ad9833_2 (data_speedmeter, setting_ratio_speedmeter);
									    		  // передаем в функцию обработки скорость + коэфф.
									    	  break;
									      case (dwin_adress_speedmeter):            // если нажата - изменяем скорость и даем команду на AD9833
		                                           data_speedmeter = data_parsing;
		                                           send_data_to_ad9833_2 (data_speedmeter, setting_ratio_speedmeter);

	                                            	// передаем в функцию обработки скорость + коэфф.
									    	  break;
									      case (dwin_adress_button_back): // если нажата выбор режима - true и выходим из функции
									    		  // дать команду на остановку генератора
									    		if (data_parsing == 0) goToPageDWIN(1);
	                                           	flag_func_2 = true;
									      	  break;
									      default:
									    	  break;
								}
				}
		}
}
