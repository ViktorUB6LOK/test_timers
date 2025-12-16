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
extern bool flag_tim2_IT;            // флаг сработки таймера 2 по прерыванию (счет EXT имп speedmeter)
extern bool flag_tim2_IT_NON_inpuls; // флаг сработки таймера 2 по прерыванию (переполнение счетчика)
                                                   // для отслеживания пропадания входных импульсов

extern uint16_t input_pulse_counter;            // счетчик кол-ва входных импульсов за (1/freq_measure_input_pulse =0.2 сек)
extern uint32_t duration_input_pulse_mks;       // длительность входных импульсов (для малых частот - спидометр)
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
			func_5();
			break;
		case (dwin_data_app_Counter_flowmeter): // режим 6 (counter flowmeter)
			goToPageDWIN(page_Counter_flowmeter); // переключение на страницу режима 6
			func_6();
			break;
		default:
			break;
		}
	}
}

void func_1() { // out pulse flowmeter
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

void func_2() { // out pulse speedmeter
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

void func_3 (){ // out pulse flowmeter & speedmeter
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

void func_4 (){ // IO pulse flowmeter
/*
* на малых входных частотах изменение на 1 (за 0,2сек) -
* колебания отображаемой частоты +-5Гц (за счет x5 в расчетах)
*/
	    HAL_TIM_Base_Start_IT(&htim3);
		HAL_TIM_Base_Start(&htim4);
		bool flag_func_4 = false;           // для выхода из функции - нажата кнопка выбора режима работы
		bool status_flowmeter = false;      // Кнопка Старт - Стоп расходомера
		bool status_flowmeter_old = false;      // Кнопка Старт - Стоп расходомера
		uint16_t adress_parsing = 0;
		uint16_t data_parsing = 0;
		static uint16_t setting_ratio_flowmeter = setting_ratio_flowmeter_default;        // значение литр/мин
		uint16_t setting_ratio_flowmeter_old = 0;  // предыдущее значение параметра расходомера для сравнения при проверке условия
		static uint16_t data_flowmeter = data_flowmeter_default;           // начальное значение расходомера (0)

		uint16_t input_pulse_freq = 0;           // частота входного сигнала (в сек) = input_pulse_counter * freq_measure_input_pulse
		uint16_t input_pulse_freq_max = (setting_ratio_flowmeter * data_flowmeter_max) / 60;    // max входная частота c расходомера (с генератора)
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
		          			// Выключаем таймеры и сбрасываем счетчики (?)
		          			HAL_TIM_Base_Stop_IT(&htim3);
		          			HAL_TIM_Base_Stop(&htim4);
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
* - изменение значения длительности входного импульса - input_pulse_counter
* - изменение setting_ratio_flowmeter пользователем
*
* на малых входных частотах изменение на 1 (за 0,2сек) - колебания отображаемой частоты +-5Гц (за счет x5 в расчетах)
*/
	 if (((flag_flowmeter_tim3_IT) &&
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
			input_pulse_counter_old = input_pulse_counter;
		}
		else
			input_pulse_counter_old = input_pulse_counter;

	// Формула для расчета вывода значения расхода (л/мин) - (input_pulse_freq * 60) / (setting_ratio_flowmeter)

		writeHalfWordDWIN(dwin_adress_show_freq_input_1, input_pulse_freq);   // вывод на dwin (0x6000) частоты входного сигнала
		HAL_Delay(20);

// Проверить чтоб не долбило в регисты лишний раз !!!

		if (status_flowmeter){ // отображение если нажата кнопка Старт
			uint16_t freq_for_send_dwin = (input_pulse_freq * 60) / (setting_ratio_flowmeter); // расчет для отправки в dwin
		writeHalfWordDWIN(dwin_adress_flowmeter, freq_for_send_dwin);     // расход л/мин
		HAL_Delay(20);
        AD9833_SetWaveData(input_pulse_freq, 0);      // установка измеренной частоты (кол-во импульсов за секунду)
        HAL_GPIO_TogglePin(Out_PA6_GPIO_Port, Out_PA6_Pin);  // проверка частоты обращений к модулям
		}
		else {
			writeHalfWordDWIN(dwin_adress_flowmeter,0);     // вкл Стоп - расход 0 л/мин (отображение - 0 (выкл))
			HAL_Delay(20);
            AD9833_SetWaveData(0, 0);                // установка AD в 0 (выкл)
            HAL_GPIO_TogglePin(Out_PA7_GPIO_Port, Out_PA7_Pin);  // проверка частоты обращений к модулям
			}
	}
  } /*while*/
} /*end func_4*/

void func_5 (){ // IO pulse speedmeter
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1); // запуск 2-х каналов таймера в режиме сравнения
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	bool flag_func_5 = false;           // для выхода из функции - нажата кнопка выбора режима работы
	bool flag_input_freq_changed = false;  // часота входного сигнала изменилась
	bool flag_button_start = false;  // статус кнопки старт
	bool flag_button_stop = false;   // статус кнопки стоп
	bool flag_fix = false;           // статус фиксации кнопки Старт-Стоп для однократного пуска условия if()

	uint16_t adress_parsing = 0;
	uint16_t data_parsing = 0;
	float input_pulse_freq = .0f;
	uint16_t setting_ratio_speedmeter = setting_ratio_speedmeter_default;  // коэф. имп/100м
	uint16_t setting_ratio_speedmeter_old = 0;
    float input_pulse_freq_max = data_speedmeter_max / (3.6 * setting_ratio_speedmeter / 100); // для начала, далее расчет в парсинге
/*
 * Вычисление max частоты :
 * max_freq_speedmeter_pulse = 35 / (3.6 * speedmeter_impuls_100meter / 100 ); - ранее было ...
 * input_pulse_freq_max = data_speedmeter_max / (3.6 * setting_ratio_speedmeter / 100); - сейчас так
 *
 */
    uint16_t data_speedmeter = data_speedmeter_default;
             duration_input_pulse_mks = 0;
    uint32_t duration_input_pulse_mks_old = 0;

// ---- начальные установки для отображения на dwin- приборе -------------------------
		writeHalfWordDWIN(dwin_adress_speedmeter, data_speedmeter);
		HAL_Delay(20);
		writeHalfWordDWIN(dwin_adress_show_status_input, 0);
		HAL_Delay(20);
		writeHalfWordDWIN(dwin_adress_speedmeter_setting, setting_ratio_speedmeter);
		HAL_Delay(20);
// -----------------------------------------------------------------------------------
	while (!flag_func_5) {
		if (flag_dwin_tx_IT) {  // сработало прерывание - UART буфер заполнен
			flag_dwin_tx_IT = false;
			parsingDWIN();
			adress_parsing = readDataDWIN.parsingDataDWIN.data[0] << 8
						   | readDataDWIN.parsingDataDWIN.data[1];
			data_parsing = readDataDWIN.parsingDataDWIN.data[3] << 8
						   | readDataDWIN.parsingDataDWIN.data[4];
	  switch (adress_parsing) {
		case (dwin_adress_speedmeter_setting): // если нажали на выбор коэфф. имп/100м - установка
			setting_ratio_speedmeter = data_parsing;
			input_pulse_freq_max = data_speedmeter_max / (3.6 * setting_ratio_speedmeter / 100);
			break;
		case (dwin_adress_button_speedmeter_start):  // нажали Старт
			if (data_parsing) {
			  flag_button_start = true; // для работы кнопки Старт
			  flag_button_stop = false; // для работы кнопки Старт
			  flag_fix = true;
			}
			else {
			  flag_button_start = false; // для работы кнопки Старт
			  flag_button_stop = true; // для работы кнопки Старт
			  flag_fix = true;
			}
		   break;
	    case (dwin_adress_button_change_menu): // если нажата выбор режима - true и выходим из функции
			if (data_parsing == dwin_data_app_change_menu) {
			   power_off_ad9833_2();   // выкл AD
			   writeHalfWordDWIN(dwin_adress_button_speedmeter_start, 0);
			   HAL_Delay(20);
			   writeHalfWordDWIN(dwin_adress_speedmeter,0);     // вкл Стоп - скорость =0 -отображение  (выкл)
			   HAL_Delay(20);
			   writeHalfWordDWIN(dwin_adress_show_freq_input_2, 0); //  отображение входной частоты на dwin
			   HAL_Delay(20);
			   writeHalfWordDWIN(dwin_adress_show_status_input, 0);
			   HAL_Delay(20);
			   goToPageDWIN(page_start);
			   // Выключаем таймеры!!!
			   HAL_TIM_Base_Stop_IT(&htim2);
			   HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1); // запуск 2-х каналов таймера в режиме сравнения
			   HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);
			   flag_func_5 = true;
			}
		    break;
		default:
			break;
	} /*switch*/
 } /*if*/

/* -------- Расчет входной частоты и вывод на Dwin--------------------------------------------------------------------
* Условия пересчитывания:
* - прерывание (получение новых данных) - flag_speedmeter_tim2_IT
* - устранение незначительных изменений входных данных -
*                           - abs (duration_input_pulse_mks_old - duration_input_pulse_mks) > 100
* - прерывание (переполнение счетчика - нет данных)  - flag_speedmeter_tim2_IT_NON_inpuls
*/
		if (((flag_tim2_IT) && ( abs (duration_input_pulse_mks_old - duration_input_pulse_mks) > 100)) ||
				(flag_tim2_IT_NON_inpuls ))
	 {
			flag_tim2_IT = false;
			flag_input_freq_changed = true;  // изменилась входная частота

		if ((flag_tim2_IT_NON_inpuls) || (duration_input_pulse_mks == 0) ||
				(duration_input_pulse_mks > 6000000 -1)) {
			input_pulse_freq = 0;
			flag_tim2_IT_NON_inpuls = false;
			writeHalfWordDWIN(dwin_adress_show_status_input, 0);
			HAL_Delay(20);
		     }
		else {
			input_pulse_freq = (1000000.0f / duration_input_pulse_mks) + 0.001;  // вычисляем частоту входного импульса
			// + 0.001 - коррекция вычисленная экспериментальным путем
			writeHalfWordDWIN(dwin_adress_show_status_input, 1);
			HAL_Delay(20);
		}

			duration_input_pulse_mks_old = duration_input_pulse_mks;
			if (input_pulse_freq >= input_pulse_freq_max)    // проверка на превышение max частоты
				{input_pulse_freq = input_pulse_freq_max;}
			uint16_t input_freq_for_send_dwin = (uint16_t)	((input_pulse_freq * 1000));
			writeHalfWordDWIN(dwin_adress_show_freq_input_2, input_freq_for_send_dwin); //  отображение входной частоты на dwin
			HAL_Delay(20);
	 } /* Конец расчета частоты входного сигнала и вывода на DWIN*/
// ================================================================================================================

// ------------------ ТУТ условие чтоб не долбить регистры DWIN и AD9833 !!! --------------------------------------
		/*
		 * Вывод данных на DWIN и отправка их на AD9833
		 * Условия:
		 * - нажата кнопка Старт - status_speedmeter = true
		 * - часота входного сигнала изменилась - flag_input_freq_changed
		 * - коэф имп/100 м - изменился - setting_ratio_speedmeter_old != setting_ratio_speedmeter
		 * - для однократного пуска при смене статуса кнопки Старт - flag_fix
		 */

		if (flag_button_start)
		  { if (flag_input_freq_changed || setting_ratio_speedmeter_old != setting_ratio_speedmeter || flag_fix )
		     {
				setting_ratio_speedmeter_old = setting_ratio_speedmeter;
			    flag_input_freq_changed = false;
			    flag_fix = false;
/*
 * Расчет числа частоты передаваемого в DWIN:
 * - формула:
 * uint16_t speed_for_send_dwin = (uint16_t) 10* (input_pulse_freq * (setting_ratio_speedmeter / 100) * 3.6);
 * где (* 10) для отображения на dwin (десятичное число)
 * - 3,6 - коэф перевода 1м/с = 3.6км/час
 * Важно! - (setting_ratio_speedmeter / 100) = 160/100 = 1 (!), если тип uint_16t !!!
 * Поэтому формулу переписал на аналогичную :
 * speed_for_send_dwin = (uint16_t) (input_pulse_freq * setting_ratio_speedmeter  * 3.6) / 10
 */
		uint16_t speed_for_send_dwin = (uint16_t) (input_pulse_freq * setting_ratio_speedmeter  * 3.6) / 10.0f ;
		HAL_Delay(20);
		writeHalfWordDWIN(dwin_adress_speedmeter, speed_for_send_dwin); //  отображение входной частоты на DWIN
		HAL_Delay(20);
		AD9833_SetWaveData_2(input_pulse_freq, 0);      // установка измеренной частоты (кол-во импульсов за секунду)
		HAL_GPIO_TogglePin(Out_PA6_GPIO_Port, Out_PA6_Pin);  // проверка частоты обращений к модулям
	 }
 } /*if (flag_button_start)*/
// ===============================================================================================================

// ------------------ ТУТ условие чтоб не долбить регистры DWIN и AD9833 !!! --------------------------------------

		if (flag_button_stop) {// если нажата Стоп
		  if (flag_fix) // для однократного выполнения условия
			 {
			  flag_fix= 0;
			  writeHalfWordDWIN(dwin_adress_speedmeter,0);     // вкл Стоп - скорость =0 -отображение  (выкл)
			  HAL_Delay(20);
		      AD9833_SetWaveData_2(0, 0);                // установка AD в 0 (выкл)
		      HAL_GPIO_TogglePin(Out_PA7_GPIO_Port, Out_PA7_Pin); // проверка частоты обращений к модулям
			}
		} /*end if (flag_button_stop)*/
	} /*while*/
} /*end func_5*/

void func_6 (){ // Counter flowmeter

	/* Описание логики работы функции :
	 * - используем 2 таймера:
	 * - 1 -й 32-bit - для счета общего кол-ва поступающих с расходомера импульсов
	 *                 старт таймера в обычном режиме
	 *                 Остался только TIM1 (16-bit - 80 литров при 800 имп/литр) с ETR2
	 * - 2 -й 16-bit - отсчитывает интервалы для отображения показаний вылитой жидкости
	 *                 таймер будет работать в режиме прерывания с интервалом ХХ мсек.
	 *                 Используем: TIM6
	 * - Кнопки: Выбор режима (возврат), Старт, Стоп, Сброс, ввод кол-ва имп/литр
	 *
	 * Пути решения:
	 * - (используем уже инициализированые таймеры и в функции меняем их параметры под нашу задачу) -ПОКА НЕ БУДЕМ)
	 * - создаем новые таймеры, старые не трогаем!
	 *
	 * Обазцы команд работы с таймерами:
	 * __HAL_TIM_SET_COUNTER(&htim1, 0x0000); //установка обнуление счётчика
	 * __HAL_TIM_GET_COUNTER(&htim4);         // получение данных счетчика
	 *  __HAL_TIM_GET_AUTORELOAD(&htim1)    // получить заначение Counter Period
	 *
	 */

// запускаем 2  таймера

	bool flag_func_6 = false;        // для выхода из функции - нажата кнопка выбора режима работы
	bool flag_button_start = false;  // статус кнопки старт
	bool flag_button_stop = false;   // статус кнопки стоп
	bool flag_fix = false;           // статус фиксации кнопки Старт-Стоп для однократного пуска условия if()
	bool flag_button_reset = false;  // статус кнопка сброса

	uint16_t adress_parsing = 0;
	uint16_t data_parsing = 0;

	uint16_t setting_ratio_flowmeter = setting_ratio_flowmeter_default;  // коэф. имп/литр

// ----------------------- начальные установки для отображения на dwin- приборе -------------------------
				writeHalfWordDWIN(dwin_adress_show_counter_flowmeter, 0.000f); // вылито - 0
				HAL_Delay(10);
				writeHalfWordDWIN(dwin_adress_flowmeter_setting, setting_ratio_flowmeter); // 600 имп/литр
				HAL_Delay(10);
// ------------------------------------------------------------------------------------------------------
	while (!flag_func_6) {
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
			  // .................................................
	               break;
			    case (dwin_adress_button_flowmeter_reset):  // нажали Сброс
			   			  // .................................................
			   	   break;
				case (dwin_adress_button_change_menu): // если нажата выбор режима - true и выходим из функции
					if (data_parsing == dwin_data_app_change_menu) {
					 // Выключаем таймеры и сбрасываем счетчики (?)
//					   HAL_TIM_Base_Stop_IT(&htim3);
//					   HAL_TIM_Base_Stop(&htim4);
//					   __HAL_TIM_SET_COUNTER(&htim3, 0x0000);  // нужно ли это делать ?
//					   __HAL_TIM_SET_COUNTER(&htim4, 0x0000);  // нужно ли это делать ?

					   writeWordDWIN(dwin_adress_button_flowmeter_start, 0);  // сброс кнопки в положение Старт
					   writeWordDWIN(dwin_adress_button_flowmeter_reset, 0);  // сброс кнопки в положение Старт
					   writeWordDWIN(dwin_adress_show_counter_flowmeter, 0);  // сброс вылито на 0
					   goToPageDWIN(page_start);
				       flag_func_6 = true;
				     }
				  break;
				default:
				  break;
		} /*switch*/
	} /*if*/
	} /*while*/
} /*end func_6*/
