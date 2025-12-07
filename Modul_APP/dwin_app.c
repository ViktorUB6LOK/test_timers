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
	HAL_Delay(2000);
	goToPageDWIN(page_start);
	HAL_Delay(100);
}

void DWIN_Select_mode() {
	bool flag_select_mode = false;
	uint16_t adress_parsing = 0;
	uint16_t data_parsing = 0;

	while (!flag_select_mode) {
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

	if (adress_parsing == dwin_adress_button_change_menu) {
		switch (data_parsing) {
		case (dwin_data_app_flowmeter):   // режим 1 (генератор flowmeter)
			goToPageDWIN(page_flowmeter);   // переключение на страницу режима 1
			func_1();
			break;
		case (dwin_data_app_speedmeter):       // режим 2 (генератор speedmeter)
			goToPageDWIN(page_speedmeter);  // переключение на страницу режима 2
			func_2();
			break;
		case (dwin_data_app_flowmeter_speedmeter): // режим 3 (генератор flowmeter + speedmeter)
			goToPageDWIN(page_flowmeter_speedmeter); // переключение на страницу режима 3
			break;
		default:
			break;
		}
	}
}

void func_1() {
	bool flag_func_1 = false;
	bool status_flowmeter = false;
	uint16_t adress_parsing = 0;
	uint16_t data_parsing = 0;
	static uint16_t setting_ratio_flowmeter = setting_ratio_flowmeter_default;
	static uint16_t data_flowmeter = data_flowmeter_default;
	// начальные установки -----------------------------------------------------------
	writeHalfWordDWIN(dwin_adress_flowmeter, data_flowmeter);
	HAL_Delay(10);
	writeHalfWordDWIN(dwin_adress_flowmeter_setting, setting_ratio_flowmeter);
	HAL_Delay(10);
	//send_data_to_ad9833 (data_flowmeter, setting_ratio_flowmeter, status_flowmeter);
	// -------------------------------------------------------------------------------

	while (!flag_func_1) { // пока не будет нажата кнопка выбора режима крутимся в цикле
		if (flag_dwin_tx_IT) {
			flag_dwin_tx_IT = false;
			parsingDWIN();
			adress_parsing = readDataDWIN.parsingDataDWIN.data[0] << 8
					| readDataDWIN.parsingDataDWIN.data[1];
			data_parsing = readDataDWIN.parsingDataDWIN.data[3] << 8
					| readDataDWIN.parsingDataDWIN.data[4];

			switch (adress_parsing) {
			case (dwin_adress_button_flowmeter_start):
				status_flowmeter = (bool) data_parsing;
				if (status_flowmeter)
					send_data_to_ad9833(data_flowmeter,
							setting_ratio_flowmeter);
				else
					power_off_ad9833(); // выключить AD
				break;

			case (dwin_adress_flowmeter_setting): // если нажали на выбор коэфф. литр/мин
				setting_ratio_flowmeter = data_parsing;
				if (status_flowmeter)
					send_data_to_ad9833(data_flowmeter,
							setting_ratio_flowmeter);
				// передаем в функцию обработки поток + коэфф.
				break;

			case (dwin_adress_flowmeter): // если нажата - изменяем поток и даем команду на AD9833
				data_flowmeter = data_parsing;
				if (status_flowmeter)
					send_data_to_ad9833(data_flowmeter,
							setting_ratio_flowmeter);

				// передаем в функцию обработки поток + коэфф.
				break;

			case (dwin_adress_button_change_menu): // если нажата выбор режима - true и выходим из функции
				if (data_parsing == dwin_data_app_change_menu) {
					power_off_ad9833();
					writeWordDWIN(dwin_adress_button_flowmeter_start, 0);
					goToPageDWIN(page_start);
				    flag_func_1 = true;
				}
				break;

			default:
				break;
			}
		}
	}
}

void func_2() {
	bool flag_func_2 = false;
	bool status_speedmeter = false;
	uint16_t adress_parsing = 0;
	uint16_t data_parsing = 0;
	static uint16_t data_speedmeter = data_speedmeter_default;
	static uint16_t setting_ratio_speedmeter = setting_ratio_speedmeter_default;

	// начальные установки -----------------------------------------------------------
	writeHalfWordDWIN(dwin_adress_speedmeter, data_speedmeter);
	HAL_Delay(10);
	writeHalfWordDWIN(dwin_adress_speedmeter_setting, setting_ratio_speedmeter);
	HAL_Delay(10);
	//send_data_to_ad9833_2 (data_speedmeter, setting_ratio_speedmeter);
	// -------------------------------------------------------------------------------

	while (!flag_func_2) { // пока не будет нажата кнопка выбора режима крутимся в цикле
		if (flag_dwin_tx_IT) {
			flag_dwin_tx_IT = false;
			parsingDWIN();
			adress_parsing = readDataDWIN.parsingDataDWIN.data[0] << 8
					| readDataDWIN.parsingDataDWIN.data[1];
			data_parsing = readDataDWIN.parsingDataDWIN.data[3] << 8
					| readDataDWIN.parsingDataDWIN.data[4];

			switch (adress_parsing) {
			case (dwin_adress_button_speedmeter_start):
				status_speedmeter = (bool) data_parsing;
				if (status_speedmeter)
					send_data_to_ad9833_2(data_speedmeter,
							setting_ratio_speedmeter);
				else
					power_off_ad9833_2(); // выключить AD_2
				break;
			case (dwin_adress_speedmeter_setting): // если нажали на выбор коэфф. имп/100м
				setting_ratio_speedmeter = data_parsing;
				if (status_speedmeter)
					send_data_to_ad9833_2(data_speedmeter,
							setting_ratio_speedmeter);
				// передаем в функцию обработки скорость + коэфф.
				break;
			case (dwin_adress_speedmeter): // если нажата - изменяем скорость и даем команду на AD9833
				data_speedmeter = data_parsing;
				if (status_speedmeter)
					send_data_to_ad9833_2(data_speedmeter,
							setting_ratio_speedmeter);

				// передаем в функцию обработки скорость + коэфф.
				break;
			case (dwin_adress_button_change_menu): // если нажата выбор режима - true и выходим из функции
				if (data_parsing == dwin_data_app_change_menu) {
					power_off_ad9833_2();
					writeWordDWIN(dwin_adress_button_speedmeter_start, 0);
					goToPageDWIN(page_start);
				    flag_func_2 = true;
				}
				break;
			default:
				break;
			}
		}
	}
}
