#include "lcd_app.h"

void LCD_Start()                 // Запуск LCD и вывод тестовой надписи
{
	lcdBacklightOn();
	lcdInit();
	lcdSetOrientation(LCD_ORIENTATION_LANDSCAPE);
	lcdFillRGB(COLOR_WHITE);

	// Пишем текст сверху экрана
	lcdSetTextFont(&Font12);
	lcdSetTextColor(COLOR_BLACK, COLOR_WHITE);
	lcdSetCursor(46, 5);   // xy
	lcdPrintf("Test MY PROJECT, NOV 2025, UB6LOK");
	lcdFillRect(300, 0, 320, 20, COLOR_RED);
}

void printedtxt(char *strinput)  // Вывод на LCD данных
{
	static uint8_t numstr = 1;       // номер строки на LCD
	static uint8_t position = 1;     // номер стороки для отображения на LCD
	char str[45] = { 0, };
	sprintf(str, "%u %s\n", position, strinput);

	if (numstr < 11) {
		lcdSetTextFont(&Font12);
		lcdSetCursor(20, numstr * 20);
		lcdPrintf(str);
		++numstr;
		++position;
	} else { // переполнение строк на LCD
		numstr = 1;
		lcdFillRGB(COLOR_WHITE);
		lcdSetTextFont(&Font12);
		lcdSetCursor(20, numstr * 20);
		lcdPrintf(str);
		++numstr;
		++position;
	}
}
