/*
 * setting.h
 *
 *  Created on: Dec 6, 2025
 *      Author: Big_Viktor
 */

#ifndef SETTING_H_
#define SETTING_H_

// -------------------- Модули AD9833 настраиваются в ad9833.h -----------------------------------

#define Modul_1_ON              // ВКЛ - Первый модуль
#define Modul_2_ON              // ВКЛ - Второй модуль

// ------------------- Setting dwin_app -------------------------------
#define dwin_adress_flowmeter  0x1000
#define dwin_adress_flowmeter_setting  0x1010
#define dwin_adress_button_flowmeter_start 0x1020
#define setting_ratio_flowmeter_default  600        // имп/литр начальные (стартовая страница)
#define data_flowmeter_default  0                  // начальные данные (стартовая страница)
#define data_flowmeter_max 200                     // максимальное значение расходомера

#define dwin_adress_speedmeter 0x2000
#define dwin_adress_speedmeter_setting 0x2010
#define dwin_adress_button_speedmeter_start 0x2020
#define setting_ratio_speedmeter_default 160         // имп/100метров начальные (стартовая страница)
#define data_speedmeter_default 0                    // начальные данные (стартовая страница)
#define data_speedmeter_max 35                       // максимальное значение спидометра

#define dwin_adress_button_change_menu 0x5001         // адресс кнопки (выбор режима)
#define dwin_data_app_change_menu 0x0000              // режим выбора режима
#define dwin_data_app_flowmeter   0x0001              // режим расходомера
#define dwin_data_app_speedmeter  0x0002              // режим спидометра
#define dwin_data_app_flowmeter_speedmeter 0x0003     // режим совмещения расходомера и спидометра
#define dwin_data_app_IO_flowmeter   0x0004           // режим IO расходомера
#define dwin_data_app_IO_speedmeter  0x0005           // режим IO спидометра
#define dwin_data_app_IO_flowmeter_speedmeter 0x0006  // режим совмещения IO расходомера и спидометра

#define freq_measure_input_pulse 5     // частота измерения входного сигнала 1 (flow) - настройка таймера 3 - 0,2сек
#define dwin_adress_show_freq_input_1 0x6000 // адрес переменной для вывода частоты входного сигнала flow
#define dwin_adress_show_freq_input_2 0x6005 // адрес переменной для вывода частоты входного сигнала speed
#define dwin_adress_show_status_input 0x5005 // адрес сообщения о статусе входного сигнала


/* setting_ratio_flowmeter - параметр расходомера - кол-во имп/литр (задается пользователем)
*  input_pulse_counter  - счетчик кол-ва входных импульсов, колбэк из main.c
*  input_pulse_freq      = input_pulse_counter * freq_measure_input_pulse - частота входных импульсов, Гц
*  freq_measure_input_pulse - частота измерений, задается в define
*  input_pulse_freq_max  = (setting_ratio_flowmeter * data_flowmeter_max) / 60;    // max входная частота входных импульсов
*
*/

// --------------------------------------------------------------------
enum dwin_page { // нумерация переключаемых страниц в dwin
	page_start = 1, page_flowmeter, page_speedmeter, page_flowmeter_speedmeter,
	page_IO_flowmeter = 10, page_IO_speedmeter = 12, page_IO_flowmeter_speedmeter
};

#endif /* SETTING_H_ */
