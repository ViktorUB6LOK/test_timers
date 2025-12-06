/*************************************************************************************
 Title	:   Analog Devices AD9833 DDS Wave Generator Library for STM32 Using HAL Libraries
 Author:    Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>  
 Software:  IAR Embedded Workbench for ARM
 Hardware:  Any STM32 device
 *************************************************************************************/
#ifndef _AD_9833_H
#define _AD_9833_H

//#define Modul_1_ON              // ВКЛ - Первый модуль
//#define Modul_2_ON              // ВКЛ - Второй модуль

//#include "main.h"
#include <math.h>
#include "stm32f4xx_hal.h"
#include "setting.h"

// ------------------------- Defines -------------------------
#define FMCLK 25000000        // Master Clock On AD9833
//-------- Первый модуль ----------------------
#ifdef Modul_1_ON
	#define AD9833PORT GPIOA      // PORT OF AD9833 Modul_1
	#define AD9833DATA GPIO_PIN_0 // SPI DATA PIN
	#define AD9833SCK GPIO_PIN_1  // SPI Clock PIN
	#define AD9833SS GPIO_PIN_2   // SPI Chip Select
#endif /*Modul_1_ON*/
//-------- Второй модуль ----------------------
#ifdef Modul_2_ON
	#define AD9833PORT_2 GPIOC       // PORT OF AD9833 Modul_2
	#define AD9833DATA_2 GPIO_PIN_0  // SPI DATA PIN
	#define AD9833SCK_2 GPIO_PIN_1   // SPI Clock PIN
	#define AD9833SS_2 GPIO_PIN_2    // SPI Chip Select
#endif /*Modul_2_ON*/

#define ASM_NOP() asm("NOP")  // Assembly NOPE (Little Delay)

enum WaveType {
	SIN, SQR, TRI
};
// Wave Selection Enum

// ------------------ Functions  ---------------------
#ifdef Modul_1_ON
	void AD9833_SetWave(uint16_t Wave);                     // Sets Output Wave Type
	void AD9833_SetWaveData(float Frequency, float Phase); // Sets Wave Frequency & Phase
	void AD9833_Init(uint16_t Wave, float FRQ, float Phase);  // Initializing AD9833
#endif /*Modul_1_ON*/

#ifdef Modul_2_ON
	void AD9833_SetWave_2(uint16_t Wave);                     // Sets Output Wave Type
	void AD9833_SetWaveData_2(float Frequency, float Phase); // Sets Wave Frequency & Phase
	void AD9833_Init_2(uint16_t Wave, float FRQ, float Phase);  // Initializing AD9833
#endif /*Modul_2_ON*/

#endif /*_AD_9833_H*/
