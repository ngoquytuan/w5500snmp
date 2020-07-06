/**
 * @file	
 * @brief	
 * @version 
 * @date	16/Jan/2020
 * @par Revision
 *			
 * @author	tuannq
 * \n\n @par 
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CRC16_H
#define __CRC16_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
unsigned short CRC16_CCITT_calc(uint8_t input[], int length, unsigned short crc /*=0x0000*/) ;
#define bittest(D,i) ((D>>i) & 0x01 )

typedef struct vor_Status_t
{
		uint16_t AzimuthAngle;
		uint16_t Mod30Hz;
		uint16_t Mod9960Hz;
		uint16_t Deviation ;
		int16_t  RFlevel;
		int8_t   year;
		int8_t   month;
		int8_t   date;
		int8_t   hour;
		int8_t   min;
		int8_t   sec;
		uint8_t  Tx1Ant;					//11-b3
		uint8_t  Tx1Main;					//11-b4
		uint8_t  Tx1On;						//11-b5
		uint8_t  Tx2On;						//11-b6
		uint8_t  DualEquipment;		//24-b0
		uint8_t  LocalControl;		//11-b2
		uint8_t  Mon1Bypass;			//10-b3
		uint8_t  IntPriAlarm;			//10-b6
		uint8_t  IntMonBypass;		//11-b0
		uint8_t  OnBattery;				//6-b1
		uint8_t  ACFail;					//6-b2
		uint8_t  Status;
		
		uint8_t  Alert;//8-b0
		uint8_t  i9b5;//LOAD for vor 1150 a
		uint8_t  i10b4;//alarm for vor 1150 pmdt
		uint8_t  i10b2;//load for vor 1150 pmdt
		
		uint8_t  i10b0;// for vor 1150 pmdt
		uint8_t  i10b1;// for vor 1150 pmdt
		uint8_t  i10b5;// for vor 1150 pmdt
		
		//uint8_t  i13b4;
}vor_Status;
typedef struct dme_Status_t
{
		uint16_t DelayUs;
		uint16_t SpacingUs;
		uint16_t TxPowerWatts;
		uint16_t Efficiency ;
		uint16_t PRFppps;
		int8_t   ERP;
		int8_t   year;
		int8_t   month;
		int8_t   date;
		int8_t   hour;
		int8_t   min;
		int8_t   sec;
		uint8_t  MaintenanceAlert;//8-b0
		uint8_t  Tx1Ant;					//13-b0
		uint8_t  Tx1Main;					//13-b1
		uint8_t  Tx1On;						//13-b2
		uint8_t  Tx2On;						//13-b3
		uint8_t  DualEquipment;		//15-b0
		uint8_t  LocalControl;		//12-b4
		uint8_t  IntPriAlarm;			//10-b0
		uint8_t  IntMonBypass;		//10-b2
		uint8_t  OnBattery;				//8-b2
		uint8_t  ACFail;					//8-b3
		uint8_t  Status;
		
		uint8_t  i9b5;
		uint8_t  i10b2;
		uint8_t  i10b3;
		uint8_t  i10b5;
		uint8_t  i11b0;
		uint8_t  i11b1;
		uint8_t  i13b4;
}dme_Status;

extern dme_Status dmestatus;
extern vor_Status vorstatus;

void prasing_message(uint8_t *message);
#endif /* __UART_H */

