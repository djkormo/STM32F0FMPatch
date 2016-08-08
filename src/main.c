// ----------------------------------------------------------------------------
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "algorithm.h"
#include "config.h"
#include "resources.h"

#include <math.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_tim.h>
#include <stm32f0xx_dac.h>
#include <stm32f0xx_dma.h>
#include <stm32f0xx_adc.h>


// diodes
#define GreenLED GPIO_Pin_9
#define BlueLED GPIO_Pin_8
#define LEDGPIO GPIOC

//Define Push button
#define PushButton_Pin GPIO_Pin_0
#define PushButton_GPIO GPIOA

/* Initialization structures */

GPIO_InitTypeDef			GPIO_InitStructure;
GPIO_InitTypeDef         	LEDs;
GPIO_InitTypeDef         	Buttons;
GPIO_InitTypeDef         	Pins;
TIM_TimeBaseInitTypeDef 	TTB;
DAC_InitTypeDef         	DAC_InitStructure;
NVIC_InitTypeDef         	DACNVIC_InitStructure;
NVIC_InitTypeDef         	ADCNVIC_InitStructure;
ADC_InitTypeDef 			ADC_InitStructure;

TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCInitStructure;
ADC_InitTypeDef ADC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;




#define SAMPLES 3
volatile uint16_t RegularConvData[SAMPLES];

// read from ADC above table (with DMA)
volatile uint16_t ADC1ConvertedPitchValue1 =0;
volatile uint16_t ADC1ConvertedPitchValue2 =0;
volatile uint16_t ADC1ConvertedPitchValue3 =0;

volatile uint16_t ADC1ConvertedPitchValue =0;
volatile uint16_t ADC1ConvertedModulationValue;// =0;
volatile uint16_t ADC1ConvertedDepthValue; //= 0;


//boundaries for DAC,ADC 12-bit (0..4095)

#define MININPUTADC 0.0
#define MAXINPUTADC 4095.0

volatile float VoltValue;
volatile float VoltValue1;
volatile float VoltValue2;
volatile float VoltValue3;


volatile uint32_t accumulator=0;
volatile uint16_t accumulator10bits=0;

volatile uint32_t accumulator1=0;
volatile uint16_t accumulator10bits1=0;
volatile uint32_t accumulator2=0;
volatile uint16_t accumulator10bits2=0;
volatile uint32_t accumulator3=0;
volatile uint16_t accumulator10bits3=0;


volatile uint32_t accumulatorStep=0;
volatile uint16_t CurrentTimerVal = 0;



volatile uint16_t FMPhase =0;
volatile uint8_t FMMod =1;
volatile uint8_t FMStep =0;
volatile uint8_t FMIndex =0;

const uint8_t FMPhaseLUT [16]=
{
		1,8,1,8,1,16,1,8,
		1,8,1,8,-8,8,-16,16
};

const uint8_t FMPhaseLUTSIZE =16;

int handleTIM =1;
int usingLeds =1;
int usingDAC=1;
int usingADC=1;
int usingLCD=0;


volatile uint16_t DAC1OutputData ;

// indexing lookup table fc table of sine shape
volatile uint16_t lutFcIndex =0;
volatile uint16_t lutStep =0;

volatile uint16_t lutFcIndex1 =0;
volatile uint16_t lutStep1 =0;
volatile uint16_t lutFcIndex2 =0;
volatile uint16_t lutStep2 =0;
volatile uint16_t lutFcIndex3 =0;
volatile uint16_t lutStep3 =0;



/*
void InitClocks()
{
    // Set up 48 MHz Core Clock using HSI (8Mhz) with PLL x 6
    RCC_PLLConfig(RCC_PLLSource_HSI, RCC_PLLMul_6);
    RCC_PLLCmd(ENABLE);

    // Wait for PLLRDY after enabling PLL.
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) != SET)
    { }

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);  // Select the PLL as clock source.
    SystemCoreClockUpdate();
}
*/

// configure board
void  InitBoard()
{


        SystemInit();
        SystemCoreClockUpdate();

        //Enable GPIO Clock
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
        // enable Timer
        // TIM3 for DAC
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 ,ENABLE);
        // TIM1 for ADC
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

        //Initialize LEDs
        GPIO_StructInit(&LEDs);
        LEDs.GPIO_Pin = GreenLED | BlueLED;
        LEDs.GPIO_Mode = GPIO_Mode_OUT;
        LEDs.GPIO_OType = GPIO_OType_PP;
        LEDs.GPIO_PuPd = GPIO_PuPd_NOPULL;
        LEDs.GPIO_Speed = GPIO_Speed_Level_1; //2MHz
        GPIO_Init(LEDGPIO, &LEDs);

        // turn on green  and turn 0ff blue Leds
        GPIO_SetBits(LEDGPIO, GreenLED);
        GPIO_ResetBits(LEDGPIO, BlueLED);


}
// configure DAC
void InitDAC(void)
{
	// for output PINS
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  /* DAC Periph clock enable */

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);



  // Configure PA.04/05 (DAC) as output -------------------------
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);


  /* Fill DAC InitStructure */


  	  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T3_TRGO;
 	  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;


 	  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

 	  //(+) Enable the DAC channel using DAC_Cmd()
 	  DAC_Cmd(DAC_Channel_1, ENABLE);


  /* DAC channel1 Configuration */
 	  DAC_Init(DAC_Channel_2, &DAC_InitStructure);

  /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is
     automatically connected to the DAC converter. */
 	  DAC_Cmd(DAC_Channel_2, ENABLE);



}


/*
 * Example of ADC with IRQ
 *
 * https://my.st.com/public/STe2ecommunities/mcu/Lists/STM32Discovery/Flat.aspx?RootFolder=https%3a%2f%2fmy%2est%2ecom%2fpublic%2fSTe2ecommunities%2fmcu%2fLists%2fSTM32Discovery%2fstm32f0%20interrupt&FolderCTID=0x01200200770978C69A1141439FE559EB459D75800084C20D8867EAD444A5987D47BE638E0F&currentviews=494
*/
/*
https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/STM32F0%20ADC-DMA%20touble&FolderCTID=0x01200200770978C69A1141439FE559EB459D7580009C4E14902C3CDE46A77F0FFD06506F5B&currentviews=35
*/

void InitADC(void)
{


  /* ADC1 DeInit */
  ADC_DeInit(ADC1);

  /* ADC1 Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_TIM1, ENABLE);

  /* DMA1 clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* TIM2 Configuration */
  TIM_DeInit(TIM1);

  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1; // 1 MHz, from 48 MHz
  TIM_TimeBaseStructure.TIM_Period = 50-1; // 100 Hz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  /* Output Compare PWM Mode configuration */
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; /* low edge by default */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0x01;
  TIM_OC4Init(TIM1, &TIM_OCInitStructure);

  /* TIM1 enable counter */
  TIM_Cmd(TIM1, ENABLE);

  /* Main Output Enable */
  TIM_CtrlPWMOutputs(TIM1, ENABLE);

  /* DMA1 Channel1 Config */
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&RegularConvData[0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = SAMPLES;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  /* DMA1 Channel1 enable */
  DMA_Cmd(DMA1_Channel1, ENABLE);

  /* ADC DMA request in circular mode */
  ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);

  /* Enable DMA1 Channel1 Half Transfer and Transfer Complete interrupt */
  //DMA_ITConfig(DMA1_Channel1, DMA_IT_HT, ENABLE);
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

  /* Enable ADC_DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Initialize ADC structure */
  ADC_StructInit(&ADC_InitStructure);

  /* Configure the ADC1 in continous mode withe a resolutuion equal to 12 bits  */
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // was DIsable
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;

  ADC_InitStructure.ADC_ExternalTrigConv =  ADC_ExternalTrigConv_T1_CC4;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
  ADC_Init(ADC1, &ADC_InitStructure);


  /* Convert the  ADC_Channnel_0  with  sampling time */
  ADC_ChannelConfig(ADC1, ADC_Channel_0 , ADC_SampleTime_239_5Cycles);

  /* Convert the  ADC_Channnel_1  with  sampling time */
  ADC_ChannelConfig(ADC1, ADC_Channel_1 , ADC_SampleTime_239_5Cycles);


  /* Convert the  ADC_Channnel_2  with  sampling time */
  ADC_ChannelConfig(ADC1, ADC_Channel_2 , ADC_SampleTime_239_5Cycles);

  /* Convert the  ADC_Channnel_3  with 7.5 Cycles as sampling time */
  //ADC_ChannelConfig(ADC1, ADC_Channel_3 , ADC_SampleTime_239_5Cycles);

  /* ADC Calibration */
  ADC_GetCalibrationFactor(ADC1);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Wait the ADCEN flag */
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN));

  /* ADC1 regular Software Start Conv */
  ADC_StartOfConversion(ADC1);


  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);


  /* Enable and set DMA1_Channel1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


}


void InitTimer()
{
			RCC_PLLConfig(RCC_PLLSource_HSI, RCC_PLLMul_6);
	 	 	SystemInit(); //Ensure CPU is running at correctly set clock speed
	     	SystemCoreClockUpdate(); //Update SystemCoreClock variable to current clock speed
	     	//SysTick_Config(SystemCoreClock/1000); //Set up a systick interrupt every millisecond
	     	SysTick_Config(SystemCoreClock);
	     	SystemCoreClockUpdate();
            TTB.TIM_CounterMode = TIM_CounterMode_Up;
            TTB.TIM_Prescaler = 1; //  4800 kHz // was 300-1
            TTB.TIM_Period = 1; //1Hz; // was 10-1
            TTB.TIM_RepetitionCounter = 0;
            TIM_TimeBaseInit(TIM3, &TTB);
            TIM_Cmd(TIM3, ENABLE);



            /* http://visualgdb.com/tutorials/arm/stm32/timers/ */
            TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
            TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
            /* http://forbot.pl/blog/artykuly/programowanie/kurs-stm32-7-liczniki-timery-w-praktyce-pwm-id8459 */


            NVIC_SetPriority(TIM3_IRQn, 0);
            NVIC_EnableIRQ (TIM3_IRQn);
            DACNVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
            DACNVIC_InitStructure.NVIC_IRQChannelPriority = 0;
            DACNVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&DACNVIC_InitStructure);
            // Higher Priority
            NVIC_SetPriority(TIM3_IRQn, 0);

}



// TIM3 steering the DAC Output
void TIM3_IRQHandler()
{



    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
    	/*
    	if (usingLeds)
    	{

        if (GPIO_ReadOutputDataBit(LEDGPIO, BlueLED))
                    GPIO_ResetBits(LEDGPIO, BlueLED);
                else
                    GPIO_SetBits(LEDGPIO, BlueLED);

        if (GPIO_ReadOutputDataBit(LEDGPIO, GreenLED))
                    GPIO_ResetBits(LEDGPIO, GreenLED);
                else
                    GPIO_SetBits(LEDGPIO, GreenLED);

    	}
		*/


    	if (usingDAC)
    	{

    		// base on http://lancasterhunt.co.uk/direct-digital-synthesis-dds-for-idiots-like-me/

    	    // 32 bit range accumulator

    	    accumulator1+=lutStep1;
    	    accumulator2+=lutStep2;
    	    accumulator3+=lutStep3;
    	    // Modulation FM
    	    //accumulator=+FMPhase*FMMod;
    	    // getting  10 higher bits from accumulator

    	    /*
    	       http://stackoverflow.com/questions/8011700/how-do-i-extract-specific-n-bits-of-a-32-bit-unsigned-integer-in-c
    	       http://stackoverflow.com/questions/9718453/extract-n-most-significant-non-zero-bits-from-int-in-c-without-loops
				extract the n most significant bits from an integer in C++ and convert those n bits to an integer.
				uint32_t a,b;
				int b = a>>(32-n);
    	    */
    	    //accumulator10bits=(accumulator & (0x1FFFF << (32 - 10))) >> (32 - 10);

    	    accumulator10bits1=accumulator1>>(32-10);
    	    accumulator10bits2=accumulator2>>(32-10);
    	    accumulator10bits3=accumulator3>>(32-10);
    	   // within range of  2^10

    	   if (accumulator10bits1>=LUTSIZE)
    	   	   {
    		   	   accumulator10bits1-=LUTSIZE;
    	   	   }
    	   if (accumulator10bits2>=LUTSIZE)
    	   	   {
    	       	   accumulator10bits2-=LUTSIZE;
    	       	}
    	   if (accumulator10bits3>=LUTSIZE)
    	   	   {
    	       	   accumulator10bits3-=LUTSIZE;
    	       	}


    	// index modulo number of samples

    	if ((lutFcIndex1)>=LUTSIZE)
    		{
    			lutFcIndex1-=LUTSIZE; // instead of zero
    		}
    	if ((lutFcIndex2)>=LUTSIZE)
    	    	{
    	    		lutFcIndex2-=LUTSIZE; // instead of zero
    	    	}
    	if ((lutFcIndex3)>=LUTSIZE)
    	    	{
    	    		lutFcIndex3-=LUTSIZE; // instead of zero
    	    	}

    	// sending 12-bits output signal

    	DAC1OutputData = (uint16_t)

							((0.33)*
								(
									Sine1024_12bit[lutFcIndex1]
												   +
									Sine1024_12bit[lutFcIndex2]
												   +
									Sine1024_12bit[lutFcIndex3]
								)

							);


							// sending 12-bits output signal
			DAC_SetChannel1Data(DAC_Align_12b_R,DAC1OutputData);



			lutFcIndex1+=lutStep1;
			lutFcIndex2+=lutStep2;
			lutFcIndex3+=lutStep3;
			// *BAD
			//lutFcIndex=accumulator10bits;

    	}

    	/*
    	 http://stackoverflow.com/questions/16889426/fm-synthesis-using-phase-accumulator
    	 http://blog.thelonepole.com/2011/07/numerically-controlled-oscillators/
    	 */



       TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    }

}

// DMA1 Channel steering ADC input
void DMA1_Channel1_IRQHandler(void) // Called at 40 Hz, LED Toggles at 20 Hz
{
  /* Test on DMA1 Channel1 Transfer Complete interrupt */
  if (DMA_GetITStatus(DMA1_IT_HT1))
  {
    /* Clear DMA1 Channel1 Half Transfer interrupt pending bits */
    DMA_ClearITPendingBit(DMA1_IT_HT1);

  }

  /* Test on DMA1 Channel1 Transfer Complete interrupt */
  if (DMA_GetITStatus(DMA1_IT_TC1))
  {
    /* Clear DMA1 Channel1 Transfer Complete interrupt pending bits */
    DMA_ClearITPendingBit(DMA1_IT_TC1);


    // read table from ADC (with DMA)
    // F1 (first sine)
    ADC1ConvertedPitchValue1=RegularConvData[0];
    // F2 (second sine)
    ADC1ConvertedPitchValue2=RegularConvData[1];
    // F3 (third sine)
    ADC1ConvertedPitchValue3=RegularConvData[2];
    // linear scale from ACD Value - F1
    lutStep1=rangeScaleLinear(ADC1ConvertedPitchValue1,(int)MININPUTADC,(int)MAXINPUTADC,1,512);
    // the same for F2
    lutStep2=rangeScaleLinear(ADC1ConvertedPitchValue2,(int)MININPUTADC,(int)MAXINPUTADC,1,512);
    // and for F3
    lutStep3=rangeScaleLinear(ADC1ConvertedPitchValue3,(int)MININPUTADC,(int)MAXINPUTADC,1,512);




  }

}



int main(void)
{

	InitClocks();
    InitBoard();

    //RCC_GetClocksFreq();

    if (usingDAC)
    {
    	InitDAC();
    }

    if (usingADC)
    {
    	InitADC();
    }

    InitTimer();



  // Infinite loop
  while (1)

  {


	  if (VoltValue < 2)
	  	     {

		  	  GPIO_ResetBits(GPIOC, BlueLED);
		  	  GPIO_ResetBits(GPIOC, GreenLED);

	  	     }
	  if (VoltValue < 4 && VoltValue>=2)
	  	     {

	  	       GPIO_ResetBits(GPIOC, BlueLED);
	  	  	   GPIO_SetBits(GPIOC, GreenLED);

	  	     }
	  if (VoltValue < 6 && VoltValue>=4)
	  	  	     {

	  	  	       GPIO_SetBits(GPIOC, BlueLED);
	  	  	       GPIO_ResetBits(GPIOC, GreenLED);

	  	  	     }

	  if (VoltValue < 8 && VoltValue>=6)
	  	  	     {

	  	  	       GPIO_SetBits(GPIOC, BlueLED);
	  	  	       GPIO_SetBits(GPIOC, GreenLED);

	  	  	     }

	  if (VoltValue < 11 && VoltValue>=8)
	  	  	     {

		  	  	  GPIO_SetBits(GPIOC, BlueLED);
		  	  	  GPIO_SetBits(GPIOC, GreenLED);

	  	  	     }




  }

  return 0;
}




#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
  while (1) {
  }
}
#endif

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
