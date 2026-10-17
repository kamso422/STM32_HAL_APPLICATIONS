#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

void UartInit(void);
void gpioInit(void);
void rtcInit(void);
void gpioAnalogConfig(void);
void rtcCalenderConfig(void);
void SystemClockConfig_HSI(void);
void printmsg(char* format,...);

UART_HandleTypeDef uart2 = {0};
RTC_HandleTypeDef hrtc = {0};
RTC_TimeTypeDef hgetTime = {0};
RTC_DateTypeDef hgetDate = {0};

uint8_t command = 0;

int main(void) {

    HAL_Init();
    SystemClockConfig_HSI();
    UartInit();
    gpioInit();
    rtcInit();
    gpioAnalogConfig();

    //Enable write access to the backup domain
    __HAL_RCC_PWR_CLK_ENABLE();

    if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET){

    	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    	printmsg("woke up from the standby mode\r\n");
    }else{
    	rtcCalenderConfig();
    	printmsg("\nThis is the RTC Calendar test program\r\n");
    }


    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

    HAL_PWREx_EnableBkUpReg();

    while(1){
    	if(command == DISPLAY_TIME_CMD){
    		HAL_ResumeTick();
    		if(HAL_RTC_GetTime(&hrtc, &hgetTime, RTC_FORMAT_BIN) != HAL_OK){
    			ErrorHandler();
    		}

    		if(HAL_RTC_GetDate(&hrtc, &hgetDate, RTC_FORMAT_BIN) != HAL_OK){
    			ErrorHandler();
    		}

    		printmsg("Current time is : %02d:%02d:%02d\r\n", hgetTime.Hours,\
    				hgetTime.Minutes, hgetTime.Seconds);
    		printmsg("Current Date is: %02d-%02d-%02d\r\n",hgetDate.Date, hgetDate.Month,\
    				hgetDate.Year);
    	}else if(command == SLEEP_CMD){
    		HAL_ResumeTick();
    		printmsg("\nEntering Standby mode\n\n");
    		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    		HAL_PWR_EnterSTANDBYMode();
    	}
        HAL_SuspendTick();
        __WFI();
    }

    return 0;
}

void ErrorHandler(void) {
    while (1);
}

void SystemClockConfig_HSI(void) {
    RCC_OscInitTypeDef osc_init = {0};
    RCC_ClkInitTypeDef clk_init = {0};

    osc_init.HSIState = RCC_HSI_ON;
    osc_init.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI;

    clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk_init.AHBCLKDivider = RCC_SYSCLK_DIV4;
    clk_init.APB1CLKDivider = RCC_HCLK_DIV1;
    clk_init.APB2CLKDivider = RCC_HCLK_DIV1;


    if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {
        ErrorHandler();
    }
    if (HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_0) != HAL_OK) {
        ErrorHandler();
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void SystemClockConfig_HSE(void){

    RCC_OscInitTypeDef osc_init = {0};
    RCC_ClkInitTypeDef clk_init = {0};

    osc_init.HSEState = RCC_HSE_ON;

    clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk_init.AHBCLKDivider = RCC_SYSCLK_DIV4;
    clk_init.APB1CLKDivider = RCC_HCLK_DIV1;
    clk_init.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {
        ErrorHandler();
    }
    if (HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_0) != HAL_OK) {
        ErrorHandler();
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void SystemClockConfig_PLL(uint8_t FREQ) {
    uint8_t FLASH_LA = FLASH_LATENCY_0;

    RCC_OscInitTypeDef osc_init = {0};
    RCC_ClkInitTypeDef clk_init = {0};

    //Set up the clock source
    osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc_init.HSEState = RCC_HSE_BYPASS;
    osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc_init.PLL.PLLState = RCC_PLL_ON;
    osc_init.PLL.PLLM = 8U;
    osc_init.PLL.PLLP = 2U;
    switch (FREQ) {
        case SYSCLK_45_MHZ:
            osc_init.PLL.PLLN = 90;
            FLASH_LA = FLASH_LATENCY_1;
            break;
        case SYSCLK_50_MHZ:
            osc_init.PLL.PLLN = 100;
            FLASH_LA = FLASH_LATENCY_1;
            break;
        case SYSCLK_75_MHZ:
            osc_init.PLL.PLLN = 150;
            FLASH_LA = FLASH_LATENCY_2;
            break;
        case SYSCLK_80_MHZ:
            osc_init.PLL.PLLN = 160;
            FLASH_LA = FLASH_LATENCY_2;
            break;
        case SYSCLK_120_MHZ:
            osc_init.PLL.PLLN = 240;
            FLASH_LA = FLASH_LATENCY_3;
            break;
        case SYSCLK_150_MHZ:
            osc_init.PLL.PLLN = 300;
            FLASH_LA = FLASH_LATENCY_4;
            break;
        case SYSCLK_180_MHZ:
            osc_init.PLL.PLLN = 360;
            FLASH_LA = FLASH_LATENCY_5;
            break;
        default:
            osc_init.PLL.PLLN = 16;
            FLASH_LA = FLASH_LATENCY_0;
    }

    //Initialize clock prescalars
    clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK \
                         | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
    clk_init.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {
        ErrorHandler();
    }
    if (HAL_RCC_ClockConfig(&clk_init, FLASH_LA) != HAL_OK) {
        ErrorHandler();
    }

    //Redo the Systick configuration
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void UartInit(void) {

    uart2.Instance = USART2;
    uart2.Init.BaudRate = 115200;
    uart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart2.Init.Mode = UART_MODE_TX;
    uart2.Init.Parity = UART_PARITY_NONE;
    uart2.Init.StopBits = UART_STOPBITS_1;
    uart2.Init.WordLength = UART_WORDLENGTH_8B;

    if (HAL_UART_Init(&uart2) != HAL_OK) {
        ErrorHandler();
    }
}

void SystemClockConfig_LSE(void) {
    RCC_OscInitTypeDef osc_init = {0};
    osc_init.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    osc_init.LSEState = RCC_LSE_ON;

    if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {
        ErrorHandler();
    }

}


void gpioAnalogConfig(void){

	GPIO_InitTypeDef PortC = {0};
	GPIO_InitTypeDef PortA = {0};

	uint32_t gpioPins = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4| GPIO_PIN_5 | \
			GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | \
			GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_3 | GPIO_PIN_14 | GPIO_PIN_15;

	uint32_t gpioPinsA = GPIO_PIN_1 | GPIO_PIN_4| GPIO_PIN_5 | \
			GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10 | \
			GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;

	PortC.Pin = gpioPins;
	PortC.Mode = GPIO_MODE_ANALOG;

	PortA.Pin = gpioPinsA;
	PortA.Mode = GPIO_MODE_ANALOG;

	HAL_GPIO_Init(GPIOC, &PortC);
	HAL_GPIO_Init(GPIOA, &PortA);
}

void rtcCalenderConfig(void){

	RTC_TimeTypeDef htime = {0};
	RTC_DateTypeDef hdate = {0};

	htime.Hours = 9;
	htime.Minutes = 10;
	htime.Seconds = 30;
	htime.TimeFormat = RTC_HOURFORMAT12_PM;

	hdate.Date = 12;
	hdate.Month = RTC_MONTH_OCTOBER;
	hdate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
	hdate.Year = 18;

	if(HAL_RTC_SetTime(&hrtc, &htime, RTC_FORMAT_BIN) != HAL_OK){
		ErrorHandler();
	}

	if(HAL_RTC_SetDate(&hrtc, &hdate, RTC_FORMAT_BIN) != HAL_OK){
		ErrorHandler();
	}
}

void gpioInit(void){

	GPIO_InitTypeDef sleepButton = {0};
    GPIO_InitTypeDef button = {0};

    button.Mode = GPIO_MODE_IT_FALLING;
    button.Pin = GPIO_PIN_13;
    button.Pull = GPIO_PULLUP;
    button.Speed = GPIO_SPEED_FREQ_LOW;

    sleepButton.Mode = GPIO_MODE_IT_FALLING;
    sleepButton.Pin = GPIO_PIN_8;
    sleepButton.Pull = GPIO_PULLUP;
    sleepButton.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOC, &button);
    HAL_GPIO_Init(GPIOA, &sleepButton);

    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void rtcInit(void){

	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_12;
	hrtc.Init.AsynchPrediv = 0x7f;
	hrtc.Init.SynchPrediv = 0xff;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_LOW;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

	if(HAL_RTC_Init(&hrtc) != HAL_OK){
		ErrorHandler();
	}

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == GPIO_PIN_13){
		command = DISPLAY_TIME_CMD;
	}else{
		command = SLEEP_CMD;
	}

}

void printmsg(char* format,...){

	char str[80];

	/*Extract the argument list using VA apis*/
	va_list args;
	va_start(args, format);
	vsprintf(str, format, args);
	HAL_UART_Transmit(&uart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
	va_end(args);
}






