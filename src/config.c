/*
 * config.c
 *
 *  Created on: 11 lip 2016
 *      Author: kormo
 */
#include "stm32f0_discovery.h"
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_tim.h>
#include <stm32f0xx_dac.h>
#include <stm32f0xx_dma.h>
#include <stm32f0xx_adc.h>


#include "config.h"
#define GreenLED GPIO_Pin_9
#define BlueLED GPIO_Pin_8
#define LEDGPIO GPIOC
#define DACGPIO GPIOA
/*
const uint8_t SAMPLES =3;
volatile uint16_t RegularConvData [3] ;
*/
void InitClocks()
{
    // Set up 48 MHz Core Clock using HSI (8Mhz) with PLL x 6
    RCC_PLLConfig(RCC_PLLSource_HSI, RCC_PLLMul_6);
    //RCC_PLLConfig(RCC_PLLSource_HSI48,RCC_PLLMul_2);
    RCC_PLLCmd(ENABLE);


    // Wait for PLLRDY after enabling PLL.
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) != SET)
    { }

    //RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);  // Select the PLL as clock source.
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI48);
    SystemCoreClockUpdate();
}


// configure board
/*
void  InitBoard(void)
{


		GPIO_InitTypeDef   LEDs;
        SystemInit();
        SystemCoreClockUpdate();

        //Enable GPIO Clock
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
        // enable Timer
        // TIM3 for DAC
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 ,ENABLE);
        // TIM1 for ADC
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

        //GPIO_DeInit(LEDGPIO);

        //Initialize LEDs
        GPIO_StructInit(&LEDs);
        LEDs.GPIO_Pin = GreenLED | BlueLED;
        LEDs.GPIO_Mode = GPIO_Mode_OUT;
        LEDs.GPIO_OType = GPIO_OType_PP;
        LEDs.GPIO_PuPd = GPIO_PuPd_NOPULL;
        LEDs.GPIO_Speed = GPIO_Speed_Level_3; //50MHz
        GPIO_Init(LEDGPIO, &LEDs);
        // turn on green  and blue Leds
        GPIO_SetBits(LEDGPIO, GreenLED);
        GPIO_ResetBits(LEDGPIO, BlueLED);


}
*/




