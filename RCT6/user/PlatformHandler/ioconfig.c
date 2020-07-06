/**
 * @file	ioconfig.c
 * @brief	Cau hinh GPIO WIZ550web
 * @version 1.0
 * @date	03/Sep/2019
 * @par Revision
 *			03/Sep/2019 - 1.0 Release
 * @author	tuannq
 * \n\n @par 
 */
#include <stdio.h>
#include "GPIO_STM32F10x.h"             // Keil::Device:GPIO
#include "stm32f10x_gpio.h"
#include "ioconfig.h"
#include "stm32f10x.h"
#include "delay.h"
#include "w5500init.h"
#include "eeprom_stm.h"
#define _U1_DEBUG_ENABLE_

// IWDG: STM32 Independent Watchdog Initialization
void IWDG_Config(void)
{
	
	/* Check if the system has resumed from IWDG reset */
  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
  { 
    /* IWDGRST flag set */
    /* Turn on LED0 */
    //GPIO_PinWrite(GPIOA, ?, 0);
    /* Clear reset flags */
    RCC_ClearFlag();
  }
  else
  {
    /* WWDGRST flag is not set */
    /* Turn off LED0 */
    //GPIO_PinWrite(GPIOA, ?, 1);
  }
	
	//RCC_LSICmd(ENABLE); //open LSI
	//while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
	{
		RCC_ClearFlag();
	}

	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_128); // 40Khz / 128 = 0.31KHz; 1 / 0.31KHz = 3.22ms
	//IWDG_SetReload(1250); // 1s, max 0xfff
	IWDG_SetReload(0xfff); // 4095 * 3.22ms = 13185.9ms = 13 seconds; it means if IWDG was not reloaded, MCU will reset!

	//IWDG_ReloadCounter();
	IWDG_Enable();
}

void factoryRST(void)
	{
		#ifdef _U1_DEBUG_ENABLE_
		printf("Factory reset!\r\n");
		#endif
		EE_WriteVariable(0,123);
		EE_WriteVariable(1,456);
		EE_WriteVariable(2,789);
		//IP 192.168.1.165
		EE_WriteVariable(4,192);
		EE_WriteVariable(5,168);
		EE_WriteVariable(6,22);
		EE_WriteVariable(7,165);
		
		//GW: 192.168.1.1
		EE_WriteVariable(8,192);
		EE_WriteVariable(9,168);
		EE_WriteVariable(10,22);
		EE_WriteVariable(11,252);
		
		//SN 255.255.255.0
		EE_WriteVariable(12,255);
		EE_WriteVariable(13,255);
		EE_WriteVariable(14,255);
		EE_WriteVariable(15,0);
	}

/**
  * @brief  test_eeprom
  * @param  Call this fuction for test store data to eeprom
  * @retval  
  */	
void test_eeprom(void)
{
	uint16_t a,b,c,d; 
	uint16_t confirm[3];
	
	//Kiem tra confirm xem dung ko, neu sai thi du lieu bi sai => reset factory setting
	// Neu dung thi load config
	
	EE_ReadVariable(0,&confirm[0]);
	EE_ReadVariable(1,&confirm[1]);
	EE_ReadVariable(2,&confirm[2]);	
	
	if( (confirm[0] == 123) && (confirm[1] == 456) && (confirm[2] == 789))
	{
		#ifdef _U1_DEBUG_ENABLE_
		printf("Right eeprom data, load configs now\r\n");
		#endif
		//Load IP
		EE_ReadVariable(4,&a);
		EE_ReadVariable(5,&b);
		EE_ReadVariable(6,&c);
		EE_ReadVariable(7,&d);
		gWIZNETINFO.ip[0] = a;
		gWIZNETINFO.ip[1] = b;
		gWIZNETINFO.ip[2] = c;
		gWIZNETINFO.ip[3] = d;
		//printf("Load ip: %d.%d.%d.%d",gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
		//Load GW
		EE_ReadVariable(8,&a);
		EE_ReadVariable(9,&b);
		EE_ReadVariable(10,&c);
		EE_ReadVariable(11,&d);
		gWIZNETINFO.gw[0] = a;
		gWIZNETINFO.gw[1] = b;
		gWIZNETINFO.gw[2] = c;
		gWIZNETINFO.gw[3] = d;
		//Load SN
		EE_ReadVariable(12,&a);
		EE_ReadVariable(13,&b);
		EE_ReadVariable(14,&c);
		EE_ReadVariable(15,&d);
		gWIZNETINFO.sn[0] = a;
		gWIZNETINFO.sn[1] = b;
		gWIZNETINFO.sn[2] = c;
		gWIZNETINFO.sn[3] = d;
	}
	else
	{
		#ifdef _U1_DEBUG_ENABLE_
		printf("Wrong eeprom data\r\n");
		#endif
		EE_WriteVariable(0,123);
		EE_WriteVariable(1,456);
		EE_WriteVariable(2,789);
		//IP 192.168.22.165
		EE_WriteVariable(4,192);
		EE_WriteVariable(5,168);
		EE_WriteVariable(6,22);
		EE_WriteVariable(7,165);
		gWIZNETINFO.ip[0] = 192;
		gWIZNETINFO.ip[1] = 168;
		gWIZNETINFO.ip[2] = 22;
		gWIZNETINFO.ip[3] = 165;
		//GW: 192.168.22.1
		EE_WriteVariable(8,192);
		EE_WriteVariable(9,168);
		EE_WriteVariable(10,22);
		EE_WriteVariable(11,252);
		gWIZNETINFO.gw[0] = 192;
		gWIZNETINFO.gw[1] = 168;
		gWIZNETINFO.gw[2] = 22;
		gWIZNETINFO.gw[3] = 252;
		//SN 255.255.255.0
		EE_WriteVariable(12,255);
		EE_WriteVariable(13,255);
		EE_WriteVariable(14,255);
		EE_WriteVariable(15,0);
		gWIZNETINFO.sn[0] = 255;
		gWIZNETINFO.sn[1] = 255;
		gWIZNETINFO.sn[2] = 255;
		gWIZNETINFO.sn[3] = 0;
	}
		

}

void scanOptos(void)
	{
		if(btPressed) 
		{
			delay_ms(2000);
			if(btPressed) printf("Reset factory setting\r\n");
			factoryRST();
			NVIC_SystemReset();
		}
		if(LGB3Actived) 
		{
			delay_ms(200);
			if(LGB3Actived) printf("No Alarm\r\n");//DB9 -3
			// Khi ON, no alarm den nay sang!
		}
		
		if(LGB4Actived) 
		{
			delay_ms(200);
			if(LGB4Actived) printf("LGB4Actived\r\n");
		}
		
		if(LGB6Actived) 
		{
			delay_ms(200);
			if(LGB6Actived) printf("LGB6Actived\r\n");
		}
		
		if(LGB7Actived) 
		{
			delay_ms(200);
			if(LGB7Actived) printf("LGB7Actived\r\n");
		}
		/**********************************************/
		if(LGB2Actived) 
		{
			delay_ms(200);
			if(LGB2Actived) printf("Bypass\r\n");//dang bat che do bypass, DB9-2 
		}
		if(LGB1Actived) 
		{
			delay_ms(200);
			if(LGB1Actived) printf("DME OFF\r\n");//DB9 1, actived for OFF, non active for on
			//khi may ON, tin hieu nay bien mat
		}
		if(LGA6Actived) 
		{
			delay_ms(200);
			if(LGA6Actived) printf("LGA6Actived\r\n");
		}
		if(LGA7Actived) 
		{
			delay_ms(200);
			if(LGA7Actived) printf("LGA7Actived\r\n");
		}
		/****************OPTO U51************************/
		if(LGA4Actived) 
		{
			delay_ms(200);
			if(LGA4Actived) printf("LGA4Actived\r\n");
		}
		if(LGA3Actived) 
		{
			delay_ms(200);
			if(LGA3Actived) printf("LGA3Actived\r\n");
		}
		if(LGA2Actived) 
		{
			delay_ms(200);
			if(LGA2Actived) printf("LGA2Actived\r\n");
		}
		if(LGA1Actived) 
		{
			delay_ms(200);
			if(LGA1Actived) printf("LGA1Actived\r\n");
		}
	}
void ledON(void)
	{
		GPIO_PinWrite(GPIOA, 1, 0);
	}
void ledOFF(void)
	{
		GPIO_PinWrite(GPIOA, 1, 1);
	}	
/** 12/Feb/2020, tuannq fix for new SCH
* @brief  GPIO_config cho kit WIZ550web
  * @param  GPIO_PinWrite chu y  GPIO_Pin_0 # 0???
  * @retval  
  */
void WIZ550webGPIO_config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;			//Interrupt parameter interrupt priority

	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	//All unuse to analog
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//All unuse to analog
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//All unuse to analog
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	//OPTO INPUT PULL UP : PC10...PC14 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//OPTO INPUT PULL UP : PB3...PB9 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//BUTTON => PC5 
	GPIO_PortClock   (GPIOC, true);
	GPIO_PinConfigure(GPIOC, 5, GPIO_IN_PULL_UP, GPIO_MODE_INPUT);
	//GPIO_PinWrite(GPIOC, 5, 1);
	
	//EX LED  => PA1 
	GPIO_PortClock   (GPIOA, true);
	GPIO_PinConfigure(GPIOA, 1, GPIO_OUT_PUSH_PULL, GPIO_MODE_OUT10MHZ);
	GPIO_PinWrite(GPIOA, 1, 1);
	
	
	// PB12 W5500_CS 
	GPIO_PortClock   (GPIOB, true);
	GPIO_PinConfigure(GPIOB, 12, GPIO_OUT_PUSH_PULL, GPIO_MODE_OUT50MHZ);
	GPIO_PinWrite(GPIOB, 12, 1);
	
	// PB0 W5500_RST 
	GPIO_PinConfigure(GPIOB, 0, GPIO_OUT_PUSH_PULL, GPIO_MODE_OUT10MHZ);
	GPIO_PinWrite(GPIOB, 0, 1);
	
	// PB1 W5500_INT 
	GPIO_PinConfigure(GPIOB, 1, GPIO_IN_PULL_UP, GPIO_MODE_INPUT);
	// Enable the EXTI0 Interrupt 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;				//W5500_INT External interrupt channel
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//Subpriority 1s
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//Enable external interrupt channel
	NVIC_Init(&NVIC_InitStructure);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	// Connect EXTI Line0 to PB1 
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
  
	
	// PA3 as W5500 interrupt input 
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	
	/* Check if the system has resumed from IWDG reset */
  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
  { 
    /* WWDGRST flag set */
    /* Turn on LED0 */
    //GPIO_PinWrite(GPIOA, ?, 0);
    /* Clear reset flags */
    RCC_ClearFlag();
  }
  else
  {
    /* WWDGRST flag is not set */
    /* Turn off LED0 */
    //GPIO_PinWrite(GPIOA, ?, 1);
  }
	//GPIO_PortClock   (GPIOB, true);
	//GPIO_PinConfigure(GPIOB, 3, GPIO_IN_PULL_UP, GPIO_MODE_INPUT);
	
	
}
