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
#define setting_ratio_flowmeter_default  600        // литр/мин начальные (стартовая страница)
#define data_flowmeter_default  0                  // начальные данные (стартовая страница)
#define data_flowmeter_max 200                     // максимальное значение расходомера

#define dwin_adress_speedmeter 0x2000
#define dwin_adress_speedmeter_setting 0x2010
#define dwin_adress_button_speedmeter_start 0x2020
#define setting_ratio_speedmeter_default 160     // имп/100метров начальные (стартовая страница)
#define data_speedmeter_default 0                    // начальные данные (стартовая страница)
#define data_speedmeter_max 35                    // максимальное значение спидометра

#define dwin_adress_button_change_menu 0x5001            // адресс кнопки (выбор режима)
#define dwin_data_app_change_menu 0x0000              // режим выбора режима
#define dwin_data_app_flowmeter   0x0001              // режим расходомера
#define dwin_data_app_speedmeter  0x0002             // режим спидометра
#define dwin_data_app_flowmeter_speedmeter 0x0003              // режим совмещения расходомера и спидометра

// --------------------------------------------------------------------
enum dwin_page {
	page_start = 1, page_flowmeter, page_speedmeter, page_flowmeter_speedmeter
};

#endif /* SETTING_H_ */
