/**
 * @file	ioconfig.h
 * @brief	Cau hinh GPIO WIZ550web
 * @version 1.0
 * @date	03/Sep/2019
 * @par Revision
 *			03/Sep/2019 - 1.0 Release
 * @author	tuannq
 * \n\n @par 
 */
 
 #ifndef __IOCONFIG_H
#define __IOCONFIG_H

void IWDG_Config(void);
void WIZ550webGPIO_config(void);
void ledON(void);
void ledOFF(void);
#define btPressed GPIO_PinRead(GPIOC,5) ==0
//OPTO U53
#define LGB6Actived GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_10) == 0//IA1/PC10
#define LGB7Actived GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11) == 0//IA2/PC11
#define LGB4Actived GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12) == 0//IA3/PC12
#define LGB3Actived GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3) == 0//IA4/PB3
//OPTO U52
#define LGB2Actived GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4) == 0//IA5/PB4
#define LGB1Actived GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5) == 0//IA6/PB5
#define LGA6Actived GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6) == 0//IB1/PB6
#define LGA7Actived GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7) == 0//IB2/PB7
//OPTO U51
#define LGA4Actived GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8) == 0//IB3/PB8
#define LGA3Actived GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9) == 0//IB4/PB9
#define LGA2Actived GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13) == 0//IB5/PC13
#define LGA1Actived GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_14) == 0//IB6/PC14
void scanOptos(void);
void test_eeprom(void);
#endif

