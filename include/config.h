#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>


// diodes
//#define GreenLED GPIO_Pin_9
//#define BlueLED GPIO_Pin_8
//#define LEDGPIO GPIOC

//Define Push button
#define PushButton_Pin GPIO_Pin_0
#define PushButton_GPIO GPIOA
#define GreenLED GPIO_Pin_9
#define BlueLED GPIO_Pin_8
#define LEDGPIO GPIOC
/*
extern const uint8_t SAMPLES;
extern uint16_t RegularConvData[];
*/
extern void InitClocks(void);
//extern void InitBoard(void);
//extern void InitDAC(void);
//extern void InitDACTimer(void);
//extern void InitADC(void);
#endif /* CONFIG_H_ */
