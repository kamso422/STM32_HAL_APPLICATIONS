
#include "main.h"

void HAL_MspInit(void) {
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    SCB->SHCSR |= (0x7 << 16);
    HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
    HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
    HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
    //Enable the clocks

    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_SLEEP_DISABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_SLEEP_DISABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef uartPin = {0};
    uartPin.Mode = GPIO_MODE_AF_PP;
    uartPin.Alternate = GPIO_AF7_USART2;
    uartPin.Pin = GPIO_PIN_2;
    uartPin.Pull = GPIO_NOPULL;
    uartPin.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &uartPin);

}

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc){

	RCC_PeriphCLKInitTypeDef periClk = {0};
	periClk.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	periClk.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

	SystemClockConfig_LSE();

    if (HAL_RCCEx_PeriphCLKConfig(&periClk) != HAL_OK) {
        ErrorHandler();
    }

    __HAL_RCC_RTC_ENABLE();
}
