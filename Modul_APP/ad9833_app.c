/*
 * ad9833_app.c
 *
 *  Created on: Dec 3, 2025
 *      Author: user
 */

#include "ad9833_app.h"

// ---------   функция передачи значения параметров в модуль AD9833 --------------------------
void send_data_to_ad9833 (uint16_t data_flowmeter, uint16_t setting_ratio_flowmeter){
	static uint16_t old_data_flowmeter = 0;
	static uint16_t old_setting_ratio_flowmeter = 0;
	uint16_t flowmeter_freq_max = (setting_ratio_flowmeter * data_flowmeter_max) / 60; // max частота
	uint16_t flowmeter_freq = (setting_ratio_flowmeter * data_flowmeter) / 60;         // расчет частоты генератора

	if (((data_flowmeter != old_data_flowmeter) || (setting_ratio_flowmeter != old_setting_ratio_flowmeter)) && (flowmeter_freq <= flowmeter_freq_max)) {
	AD9833_SetWaveData(flowmeter_freq, 1); // передача частоты на модуль, 1- меандр
	   old_data_flowmeter = data_flowmeter;
	   old_setting_ratio_flowmeter = setting_ratio_flowmeter;
	}
}

// ------------ функция передачи значения параметров в модуль AD9833_2 -----------------------
void send_data_to_ad9833_2 (uint16_t data_speedmeter, uint16_t setting_ratio_speedmeter){
	    static uint16_t old_data_speedmeter = 0;
		static uint16_t old_setting_ratio_speedmeter = 0;
		float speedmeter_freq_max = data_speedmeter_max / (3.6 * setting_ratio_speedmeter / 100 ); // max частота
		float speedmeter_freq = ((float)(data_speedmeter/10)) / ((float)(3.6 * setting_ratio_speedmeter / 100 ));         // расчет частоты генератора

		if (((data_speedmeter != old_data_speedmeter) || (setting_ratio_speedmeter != old_setting_ratio_speedmeter)) && (speedmeter_freq <= speedmeter_freq_max)) {
		AD9833_SetWaveData_2(speedmeter_freq, 1); // передача частоты на модуль, 2- меандр
		   old_data_speedmeter = data_speedmeter;
		   old_setting_ratio_speedmeter = setting_ratio_speedmeter;
		}
}

void power_off_ad9833 (){
	// функция выключения модуля AD9833;
	AD9833_SetWaveData(0, 1);  // пока так - далее переделать что откл модуль совсем!
}

void power_off_ad9833_2 (){
	// функция выключения модуля AD9833;
	AD9833_SetWaveData_2(0, 1);  // пока так - далее переделать что откл модуль совсем!
}
