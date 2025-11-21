/* USER CODE BEGIN Header */
/*
 * Этап 1.
 *
 * При старте, таймер №3 будет аппаратно подавать этот сигнал на таймер №4
 * и тем самым запускать его. При остановке таймера №3 этот сигнал будет снят
 * и таймер №4 остановится. То есть, мы «вручную» запустили таймер №3
 * (пошёл отсчёт секунды), а он в свою очередь «толкает» таймер №4, который
 * начинает тактироваться от измеряемой частоты и складывать поступающие импульсы
 * в свой счётчик. Таким образом у нас получается следующее:
 * оба таймера, №3 (отмеряющий секундный интервал) и №4 (считающий импульсы)
 * стартанут чётко одновременно, а когда таймер №3 отмерит секунду и остановится,
 * то и таймер №4 тоже остановится (перестанет считать импульсы) — в счётчике
 * таймера №4 будет лежать количество импульсов входящей частоты полученные
 * за одну секунду со входа TIM4_ETR.
 * Для плавности - таймер 3 работает с частотой 5 Гц
 *
 * Этап 2.
 *
 * Тоже самое, только для измерения скорости таймер 8 тактирует таймер 5 (ext input)
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include <stdio.h>
#include <string.h>
#include "dwin.h"
#include "ad9833.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

struct DWIN_STRUCT {
	   uint16_t adress;
	   uint16_t data;
} DWIN_VAR;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//------------------- ВКЛ - ВЫКЛ -------------------------------------------------------------------------------
//#define EXTERN_FLOWMETER_ON             // ВКЛ Внешний источник flowmeter (режим работы - сниффер)

#define EXTERN_SPEEDMETER_ON            // ВКЛ Внешний источник speedmeter (режим работы - сниффер)
// !!!!!!!! - очень малая дискретность изменения частоты - метод не годится !
// вычислять частоту по длительности импульса !!! частота - float!

//#define AD9833_ON                       // ВКЛ обоих генераторов импульсов (их 2 в проекте)

#define DWIN_Tx_ON                      // ВКЛ Прием данных от DWIN (источник - DWIN)

//#define PRINT_TO_LCD_ON                 // ВКЛ инициализация LCD и печать данных на нем
//---------------------------------------------------------------------------------------------------------------

#define flowmeter_impuls_litr 600       // Параметр расходомера - кол-во импульсов на литр
#define freq_measure_flowmeter 5        // Частота измерений flow (в секунду = Гц) (настройка таймера 3)

#define speedmeter_impuls_100meter 160       // Параметр датчика скорости - кол-во импульсов на 100 метров
#define freq_measure_speedmeter    1        // Частота измерений flow (в секунду = Гц) (настройка таймера 8)

#define dwin_adress_flowmeter  0x2045
#define dwin_adress_speedmeter 0x2034

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//-------------------- Расходомер -----------------------------------------------------------------------------
//uint16_t flowmeter_impuls_litr = 600;         // Параметр расходомера - кол-во импульсов на литр
uint16_t max_flowmeter_pulse = 0;             // максимальная частота генератора (flow)
uint16_t count_flowmeter_pulse = 0;           // частота генератора (flow)
uint16_t old_count_flowmeter_pulse = 0;
//uint8_t freq_measure_flowmeter = 5;           // Частота измерений flow (в секунду = Гц)
//--------------------- Датчик скорости ------------------------------------------------------------------------
//uint16_t speedmeter_impuls_100metr = 160;        // Параметр датчика скорости - кол-во импульсов на 100 метров
uint16_t max_speedmeter_pulse = 0;                 // максимальная частота генератора (speed)
//float max_speedmeter_pulse = 0;                  // максимальная частота генератора (speed)
//float
float freq_speedmeter_pulse = 0.0f;                   // частота генератора (speed)
uint32_t duration_pulse_speedmeter_ms = 0;         // длительность импульса с датчика скорости (генератора) в мс
uint32_t old_duration_pulse_speedmeter_ms = 0;     // прежняя длительность импульса с датчика скорости (генератора) в мс
//uint8_t freq_measure_speedmeter = 5;             // Частота измерений speed (в секунду = Гц)
//---------------------------------------------------------------------------------------------------------------
uint16_t dwin_data_flowmeter = 0;   // есть же структура???
uint16_t dwin_data_speedmeter = 0;
//---------------------------------------------------------------------------------------------------------------
bool flag_flowmeter_tim3_IT = false;          // флаг сработки таймера 3 по прерыванию (счет EXT imp flowmeter)
bool flag_dwin_tx_IT = false;                 // флаг получения данных от DWIN в буфер UART
bool flag_speedmeter_tim2_IT = false;         // флаг сработки таймера 2 по прерыванию (счет EXT имп speedmeter)
//---------------------------------------------------------------------------------------------------------------

//struct DWIN_STR DWIN_VAR;
extern struct readDataDWIN_P readDataDWIN;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

#ifdef PRINT_TO_LCD_ON
	void LCD_Start(void);
	void printedtxt(char*);
#endif /*PRINT_TO_LCD_ON*/

void DWIN_Reset_var(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FSMC_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
#ifdef PRINT_TO_LCD_ON
	LCD_Start();
#endif /*PRINT_TO_LCD_ON*/
	DWIN_Reset_var();
// -------------Инициализация модулей AD9833 ---------------------------------------------
#ifdef AD9833_ON
  #ifdef Modul_1_ON
	AD9833_Init(SQR, 4, 0); // начальная инициализация - меандр, 4Гц - для тестирования
  #endif /*Modul_1_ON*/
  #ifdef Modul_2_ON
	AD9833_Init_2(SQR, 4, 0); // начальная инициализация - меандр, 4Гц - для тестирования
  #endif /*Modul_2_ON*/
#endif /*AD9833_ON*/
// ---------------------------------------------------------------------------------------
#ifdef EXTERN_FLOWMETER_ON
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start(&htim4);
#endif  /*EXTERN_FLOWMETER_ON*/

#ifdef EXTERN_SPEEDMETER_ON
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
#endif  /*EXTERN_SPEEDMETER_ON*/


#ifdef DWIN_Tx_ON
	//HAL_Delay(50); // нижеописанное вылечено прошивкой DWIN
	/* Необходима задержка инициализации чтоб не словить ответ от DWIN после его обнуления (посылка 4b4f - рукопожатие)
	 * функцией DWIN_Start(). После записи переменной в DWIN приходит ответ об успешной передаче данных в DWIN
	 * (рукопожатие). Без этой задержки данные будут приниматься с нарушениями.
	 */
	dwinUartDmaInit();
#endif /*DWIN_Tx_ON*/

//---------------- максимальная частота расходомера или генератора (имитатора расходомера)
	max_flowmeter_pulse = (flowmeter_impuls_litr * 200) / 60;

//------------- максимальная частота датчика скорости или генератора (имитатора д.скорости)
	max_speedmeter_pulse = (speedmeter_impuls_100meter * 10 * 35 / 3600);
// !!!!!!!! - очень малая дискретность изменения частоты - метод не годится !


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//=========================================================================================================================================
	while (1) {
//=========================================================================================================================================
#ifdef EXTERN_FLOWMETER_ON              //#if defined TIMER_ON || defined AD9833_ON

		/*
		 * Проверка срабатывания прерывания по таймеру 3 и изменению переменной счетчика таймера 4
		 * (чтоб лишний раз не писать в регистры AD9833 если частота не изменяется)
		 */

		if ((flag_flowmeter_tim3_IT) && (old_count_flowmeter_pulse != count_flowmeter_pulse)) {
			flag_flowmeter_tim3_IT = false;
			old_count_flowmeter_pulse = count_flowmeter_pulse;
			writeHalfWordDWIN(dwin_adress_flowmeter,
					(count_flowmeter_pulse * freq_measure_flowmeter * 60) / (flowmeter_impuls_litr));

#ifdef AD9833_ON
    #ifdef Modul_1_ON
			AD9833_SetWaveData(count_flowmeter_pulse * freq_measure_flowmeter, 0); // установка измеренной частоты (кол-во ипмульсов за секунду)
    #endif /*Modul_1_ON*/
    #ifdef Modul_2_ON  // Только для проверки !!! Modul_2 - для SPEED !
			AD9833_SetWaveData_2(count_flowmeter_pulse * freq_measure_flowmeter, 0); // установка измеренной частоты (кол-во ипмульсов за секунду)
    #endif /*Modul_2_ON*/
#endif /*AD9833_ON*/

			// отправка данных на LCD
#ifdef PRINT_TO_LCD_ON
			char str[50] = { 0, };
			sprintf(str, "FREQ: %u Hz -- Flowmeter: %u l/min \n",
					count_flowmeter_pulse * freq_measure_flowmeter,
					(count_flowmeter_pulse * freq_measure_flowmeter * 60) / (flowmeter_impuls_litr));
			printedtxt(str);
			HAL_GPIO_TogglePin(Out_PA7_GPIO_Port, Out_PA7_Pin);
#endif /*PRINT_TO_LCD_ON*/
		}
#endif /*EXTERN_FLOWMETER_ON*/

#ifdef DWIN_Tx_ON
		if (flag_dwin_tx_IT) {
			flag_dwin_tx_IT = false;
			parsingDWIN();
			DWIN_VAR.adress = readDataDWIN.parsingDataDWIN.data[0] << 8
					| readDataDWIN.parsingDataDWIN.data[1];
			DWIN_VAR.data = readDataDWIN.parsingDataDWIN.data[3] << 8
					| readDataDWIN.parsingDataDWIN.data[4];
			switch (DWIN_VAR.adress) {
			case 0x2045:
				dwin_data_flowmeter = DWIN_VAR.data;
#ifdef AD9833_ON
  #ifdef Modul_1_ON
				AD9833_SetWaveData(dwin_data_flowmeter * flowmeter_impuls_litr / 60, 1);
				// установка измеренной частоты (кол-во ипмульсов за секунду)
  #endif /*Modul_1_ON*/
  #ifdef Modul_2_ON  // Только для проверки Modul_2 - SPEED !
				AD9833_SetWaveData_2(dwin_data_flowmeter * flowmeter_impuls_litr / 60, 1);
				// установка измеренной частоты (кол-во ипмульсов за секунду)
  #endif /*Modul_2_ON*/
#endif /*AD9833_ON*/
				break;
			case 0x2034:
				dwin_data_speedmeter = DWIN_VAR.data;
				break;
			default:
				break;
			}
    #ifdef PRINT_TO_LCD_ON
			char str[45] = { 0, };
			sprintf(str, "Flowmeter=%u, Speedmeter=%u \n", dwin_data_flowmeter,
					dwin_data_speedmeter);
			printedtxt(str);
    #endif /*PRINT_TO_LCD_ON*/

		}
#endif /*TX_DWIN_ON*/

#ifdef EXTERN_SPEEDMETER_ON

		/*
		 * Проверка срабатывания прерывания по таймеру 3 и изменению переменной счетчика таймера 4
		 * (чтоб лишний раз не писать в регистры AD9833 если частота не изменяется)
		 */

		if ((flag_speedmeter_tim2_IT) && (old_duration_pulse_speedmeter_ms != duration_pulse_speedmeter_ms)) {
			flag_speedmeter_tim2_IT = false;
			old_duration_pulse_speedmeter_ms = duration_pulse_speedmeter_ms;
			freq_speedmeter_pulse = 1000.0f / duration_pulse_speedmeter_ms; // вычисляем частоту
			writeHalfWordDWIN(dwin_adress_speedmeter,
					((freq_speedmeter_pulse * speedmeter_impuls_100meter/100)*1.2));

//#ifdef AD9833_ON
//    #ifdef Modul_1_ON
//			AD9833_SetWaveData(count_flowmeter_pulse * freq_measure_flowmeter, 0); // установка измеренной частоты (кол-во ипмульсов за секунду)
//    #endif /*Modul_1_ON*/
//    #ifdef Modul_2_ON  // Только для проверки !!! Modul_2 - для SPEED !
//			AD9833_SetWaveData_2(count_flowmeter_pulse * freq_measure_flowmeter, 0); // установка измеренной частоты (кол-во ипмульсов за секунду)
//    #endif /*Modul_2_ON*/
//#endif /*AD9833_ON*/

			// отправка данных на LCD
//#ifdef PRINT_TO_LCD_ON
//			char str[50] = { 0, };
//			sprintf(str, "FREQ: %u Hz -- Flowmeter: %u l/min \n",
//					count_flowmeter_pulse * freq_measure_flowmeter,
//					(count_flowmeter_pulse * freq_measure_flowmeter * 60) / (flowmeter_impuls_litr));
//			printedtxt(str);
//			HAL_GPIO_TogglePin(Out_PA7_GPIO_Port, Out_PA7_Pin);
//#endif /*PRINT_TO_LCD_ON*/
		}
#endif /*EXTERN_SPEEDMETER_ON*/

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
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
void DWIN_Reset_var()            // Сброс на "0" переменных в DWIN
{
	writeHalfWordDWIN(dwin_adress_flowmeter, 0);
	HAL_Delay(50);    // задержка не успевает сделать сброс
	writeHalfWordDWIN(dwin_adress_speedmeter, 0);
	HAL_Delay(50);
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

//		 memset (strX, 0, sizeof (strX)); // образец

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	//---------------------------- FLOWMETER ---------------------------------------------------------
	if (htim == &htim3) // частота 5 Гц (0,2 сек) = задается для расчетов - freq_measure_flowmeter
			{
		//HAL_GPIO_TogglePin(Out_PA6_GPIO_Port, Out_PA6_Pin);
		count_flowmeter_pulse = __HAL_TIM_GET_COUNTER(&htim4); // значение в счётчике таймера №4 (за 0,2сек)
		if (count_flowmeter_pulse > max_flowmeter_pulse / freq_measure_flowmeter) {
			count_flowmeter_pulse = max_flowmeter_pulse / freq_measure_flowmeter;
		}  // ограничение по частоте - не более 400*5 = 2000 Гц
		HAL_TIM_Base_Stop_IT(&htim3);
		flag_flowmeter_tim3_IT = true;
       // обнуляем счётчики и рестартуем таймер №3
		__HAL_TIM_SET_COUNTER(&htim3, 0x0000);
		__HAL_TIM_SET_COUNTER(&htim4, 0x0000);
		HAL_TIM_Base_Start_IT(&htim3);
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart1) {
		flag_dwin_tx_IT = true;
	}
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){ // EXTERN SPEEDMETER
        if(htim->Instance == TIM2) {
                if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)      // RISING с LOW на HIGH
                        __HAL_TIM_SET_COUNTER(&htim2, 0x0000);     // обнуление счётчика
                else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) // RISING с LOW на HIGH
                {
                 duration_pulse_speedmeter_ms = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2) / 10; // чтение значения в регистре захвата/сравнения
                 flag_speedmeter_tim2_IT = true;
                 HAL_GPIO_TogglePin(Out_PA6_GPIO_Port, Out_PA6_Pin);  // тест
                }
        }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
