/*
 * dwin_app.c
 *
 *  Created on: Nov 30, 2025
 *      Author: Big_Viktor
 */
#include "dwin_app.h"

//--- задекларированы в main.c ----------------------------------------------------------------------------------
extern bool flag_dwin_tx_IT;                  // флаг получения данных от DWIN в буфер UART
extern bool flag_flowmeter_tim3_IT;   // флаг сработки таймера 3 по прерыванию (счет EXT imp flowmeter)
extern bool flag_speedmeter_tim2_IT;  // флаг сработки таймера 2 по прерыванию (счет EXT имп speedmeter)

extern uint16_t input_pulse_counter;            // счетчик кол-ва входных импульсов за (1/freq_measure_input_pulse =0.2 сек)
//---------------------------------------------------------------------------------------------------------------
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
		    func_3();
			break;
		case (dwin_data_app_IO_flowmeter): // режим 4 (IO flowmeter)
			goToPageDWIN(page_IO_flowmeter); // переключение на страницу режима 4
			func_4();
			break;
		case (dwin_data_app_IO_speedmeter): // режим 5 (IO speedmeter)
			goToPageDWIN(page_IO_speedmeter); // переключение на страницу режима 5
			//func_4();
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

void func_3 (){
	    bool flag_func_3 = false;
		bool status_flowmeter = false;
	    bool status_speedmeter = false;

		uint16_t adress_parsing = 0;
		uint16_t data_parsing = 0;

		static uint16_t setting_ratio_flowmeter = setting_ratio_flowmeter_default;
		static uint16_t data_flowmeter = data_flowmeter_default;
		static uint16_t data_speedmeter = data_speedmeter_default;
		static uint16_t setting_ratio_speedmeter = setting_ratio_speedmeter_default;

		// начальные установки -----------------------------------------------------------
		writeHalfWordDWIN(dwin_adress_flowmeter, data_flowmeter);
		HAL_Delay(10);
		writeHalfWordDWIN(dwin_adress_flowmeter_setting, setting_ratio_flowmeter);
		HAL_Delay(10);
		writeHalfWordDWIN(dwin_adress_speedmeter, data_speedmeter);
		HAL_Delay(10);
		writeHalfWordDWIN(dwin_adress_speedmeter_setting, setting_ratio_speedmeter);
		HAL_Delay(10);
		// -------------------------------------------------------------------------------

		while (!flag_func_3) { // пока не будет нажата кнопка выбора режима крутимся в цикле
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
						send_data_to_ad9833(data_flowmeter,	setting_ratio_flowmeter);
					else
						power_off_ad9833(); // выключить AD
					break;
				case (dwin_adress_button_speedmeter_start):
					status_speedmeter = (bool) data_parsing;
				    if (status_speedmeter)
					  send_data_to_ad9833_2(data_speedmeter,setting_ratio_speedmeter);
					else
					   power_off_ad9833_2(); // выключить AD_2
				break;

				case (dwin_adress_flowmeter_setting): // если нажали на выбор коэфф. литр/мин
					setting_ratio_flowmeter = data_parsing;
					if (status_flowmeter)
						send_data_to_ad9833(data_flowmeter,	setting_ratio_flowmeter);
					break;
				case (dwin_adress_speedmeter_setting): // если нажали на выбор коэфф. имп/100м
					setting_ratio_speedmeter = data_parsing;
					if (status_speedmeter)
						send_data_to_ad9833_2(data_speedmeter,setting_ratio_speedmeter);
					break;

				case (dwin_adress_flowmeter): // если нажата - изменяем поток и даем команду на AD9833
					data_flowmeter = data_parsing;
					if (status_flowmeter)
						send_data_to_ad9833(data_flowmeter,setting_ratio_flowmeter);
					break;
				case (dwin_adress_speedmeter): // если нажата - изменяем скорость и даем команду на AD9833
					data_speedmeter = data_parsing;
					if (status_speedmeter)
						send_data_to_ad9833_2(data_speedmeter, setting_ratio_speedmeter);
					break;

				case (dwin_adress_button_change_menu): // если нажата выбор режима - true и выходим из функции
					if (data_parsing == dwin_data_app_change_menu) {
						power_off_ad9833();
						power_off_ad9833_2();
						writeWordDWIN(dwin_adress_button_flowmeter_start, 0); // сброс кнопки Старт-Стоп
					        	HAL_Delay(10);
						writeWordDWIN(dwin_adress_button_speedmeter_start, 0);// сброс кнопки Старт-Стоп
					        	HAL_Delay(10);
					        data_flowmeter  = data_flowmeter_default;       // сброс расходомера на значение по умолчанию (0)
					        data_speedmeter = data_speedmeter_default;      // сброс спидометра на значение по умолчанию (0)
						goToPageDWIN(page_start);
					    flag_func_3 = true;
					}
					break;

				default:
					break;
				}
			}
		}

}

void func_4 (){
	    HAL_TIM_Base_Start_IT(&htim3);
		HAL_TIM_Base_Start(&htim4);
		bool flag_func_4 = false;           // для выхода из функции - нажата кнопка выбора режима работы
		bool status_flowmeter = false;      // Кнопка Старт - Стоп расходомера
		bool status_flowmeter_old = false;      // Кнопка Старт - Стоп расходомера
		uint16_t adress_parsing = 0;
		uint16_t data_parsing = 0;
		static uint16_t setting_ratio_flowmeter = setting_ratio_flowmeter_default;        // значение литр/мин
		uint16_t setting_ratio_flowmeter_old = 0;  // предыдущее значение параметра расходомера для сравнения при проверке условия
		static uint16_t data_flowmeter = data_flowmeter_default;                          // начальное значение расходомера (0)

		uint16_t input_pulse_freq = 0;           // частота входного сигнала (в сек) = input_pulse_counter * freq_measure_input_pulse
		uint16_t input_pulse_freq_max = (setting_ratio_flowmeter * data_flowmeter_max) / 60;    // max входная частота c расходомера (с генератора)
	//	uint16_t input_pulse_freq_old = 0;       // предыдущее значение частоты генератора (flow) для сравнения при проверке условия
		uint16_t input_pulse_counter_old = 0;       // предыдущее значение счетчика (flow) для сравнения при проверке условия
		// начальные установки для отображения на dwin- приборе -------------------------
			writeHalfWordDWIN(dwin_adress_flowmeter, data_flowmeter);
			HAL_Delay(10);
			writeHalfWordDWIN(dwin_adress_flowmeter_setting, setting_ratio_flowmeter);
			HAL_Delay(10);
    	// -------------------------------------------------------------------------------

		while (!flag_func_4) {
			if (flag_dwin_tx_IT) {  // сработало прерывание - UART буфер заполнен
						flag_dwin_tx_IT = false;
						parsingDWIN();
						adress_parsing = readDataDWIN.parsingDataDWIN.data[0] << 8
								| readDataDWIN.parsingDataDWIN.data[1];
						data_parsing = readDataDWIN.parsingDataDWIN.data[3] << 8
								| readDataDWIN.parsingDataDWIN.data[4];
			  switch (adress_parsing) {
		          case (dwin_adress_flowmeter_setting): // если нажали на выбор коэфф. литр/мин - установка
						setting_ratio_flowmeter = data_parsing;
				     break;
		          case (dwin_adress_button_flowmeter_start):  // нажали Старт
		          				status_flowmeter = (bool) data_parsing;
		             break;
		          case (dwin_adress_button_change_menu): // если нажата выбор режима - true и выходим из функции
		          		if (data_parsing == dwin_data_app_change_menu) {
		          			HAL_TIM_Base_Stop_IT(&htim3);
		          			__HAL_TIM_SET_COUNTER(&htim3, 0x0000);  // нужно ли это делать ?
		          		    __HAL_TIM_SET_COUNTER(&htim4, 0x0000);  // нужно ли это делать ?
		          			power_off_ad9833();
		          			writeWordDWIN(dwin_adress_button_flowmeter_start, 0);
		          			goToPageDWIN(page_start);
		          		    flag_func_4 = true;
		          		}
		             break;
		          default:
		             break;
		     } /*switch*/
	    } /*if*/
/*
* Проверка срабатывания прерывания по таймеру 3 и изменению переменной счетчика таймера 4
* (чтоб лишний раз не писать в регистры AD9833 если частота не изменяется)
* Проверка чтоб обновить данные на dwin и AD:
* - нажата кнопка Старт - для вкл и выкл показаний dwin
* - срабатывание прерывания - flag_flowmeter_tim3_IT
* - изменение значения входной частоты
* - изменение setting_ratio_flowmeter пользователем
*
* на малых входных частотах изменение на 1 (за 0,2сек) - колебания отображаемой частоты +-5Гц (за счет x5 в расчетах)
*/
	 if (((flag_flowmeter_tim3_IT) &&
//      	 (input_pulse_freq_old != input_pulse_freq))
			 (input_pulse_counter_old != input_pulse_counter)) ||
			 ((status_flowmeter_old != status_flowmeter)) ||
			 (setting_ratio_flowmeter_old != setting_ratio_flowmeter)) {
		flag_flowmeter_tim3_IT = false;
		setting_ratio_flowmeter_old = setting_ratio_flowmeter;
		status_flowmeter_old = status_flowmeter;
		input_pulse_freq = input_pulse_counter * freq_measure_input_pulse;             // частота входного сигнала в Гц
		input_pulse_freq_max = (setting_ratio_flowmeter * data_flowmeter_max) / 60;    // max частота входного сигнала в Гц

		if (input_pulse_freq >= input_pulse_freq_max) {  // проверка на превышение значения входной частоты ...
			input_pulse_freq = input_pulse_freq_max;
//		    input_pulse_freq_old = input_pulse_freq;
			input_pulse_counter_old = input_pulse_counter;
		}
		else
//			input_pulse_freq_old = input_pulse_freq;
			input_pulse_counter_old = input_pulse_counter;

	// Формула для расчета вывода значения расхода (л/мин) - (input_pulse_freq * 60) / (setting_ratio_flowmeter)

		writeHalfWordDWIN(dwin_adress_show_freq_input_1, input_pulse_freq);   // вывод на dwin (0x6000) частоты входного сигнала
		HAL_Delay(20);

		if (status_flowmeter){ // отображение если нажата кнопка Старт
			uint16_t freq_for_send_dwin = (input_pulse_freq * 60) / (setting_ratio_flowmeter); // расчет для отправки в dwin
		writeHalfWordDWIN(dwin_adress_flowmeter, freq_for_send_dwin);     // расход л/мин
		HAL_Delay(20);
        AD9833_SetWaveData(input_pulse_freq, 0);      // установка измеренной частоты (кол-во импульсов за секунду)
		}
		else {
			writeHalfWordDWIN(dwin_adress_flowmeter,0);     // вкл Стоп - расход 0 л/мин (отображение - 0 (выкл))
			HAL_Delay(20);
            AD9833_SetWaveData(0, 0);                // установка AD в 0 (выкл)
			}
	}
  } /*while*/
} /*end void*/


