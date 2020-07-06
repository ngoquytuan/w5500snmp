#include <stdio.h>
#include "stm32f10x.h"                  // Device header
#include "GPIO_STM32F10x.h"             // Keil::Device:GPIO
#include "crc16CCITT.h"  
vor_Status vorstatus;
dme_Status dmestatus;
extern int8_t dmeconn;
extern int8_t vorconn;
extern int8_t vorPmdt;

const unsigned int   CRC16_CCITT_SHIFTER = 0x00FF;
const unsigned short CRC16_CCITT_TABLE[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D, 
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC, 
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, 
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067, 
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634, 
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3, 
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
unsigned short crc16CCITT;
unsigned short CRC16_CCITT_calc(uint8_t input[], int length, unsigned short crc /*=0x0000*/) {
	crc = 0;
    for(int i = 0; i < length; i++) {
        unsigned short  tableValue = CRC16_CCITT_TABLE[((crc >> 8) ^ *(char*)input++) & CRC16_CCITT_SHIFTER];
        crc = (crc << 8) ^ tableValue;
    }
 
    return crc;
}



void prasing_message(uint8_t *message)
{
	int offset;
	offset = 11;// de dem theo tai lieu
	
	if((message[0] == 0xF8)&&(message[1] == 0xF8)&&(message[9] == 0x97))//1150A VOR status message, 0x97 la do dai cua ban tin 
	{
		printf("\r\n---------1150A VOR---------------------\r\n");
		crc16CCITT = CRC16_CCITT_calc((message+2),160,crc16CCITT);
		if(((message[163]<<8)|message[162]) == crc16CCITT) 
		{
			//printf("Right CRC16\r\n");
			vorconn = 1;
			vorstatus.Tx1Ant						= bittest(message[offset+13],0);//13-b0
			vorstatus.Tx1Main 					= bittest(message[offset+13],1);//13-b1
			vorstatus.Tx1On							= bittest(message[offset+13],2);//13-b2
			vorstatus.Tx2On							= bittest(message[offset+13],3);//13-b3
			vorstatus.DualEquipment 			= bittest(message[offset+15],1);//15-b1
			vorstatus.LocalControl 				= bittest(message[offset+12],4);//12-b4
			vorstatus.IntPriAlarm 				= bittest(message[offset+10],6);//10-b6
			vorstatus.IntMonBypass 				= bittest(message[offset+11],0);//11-b0
			vorstatus.OnBattery 				= bittest(message[offset+8],2);//8-b2
			vorstatus.ACFail					= bittest(message[offset+8],3);//8-b3
			
			vorstatus.Alert					= bittest(message[offset+8],0);//8-b0
			
			vorstatus.i9b5						  = bittest(message[offset+9],5);//9-b5 load?

			
			//vorstatus.i13b4						  = bittest(message[offset+13],4);//13-b4 bypass all?
			
			
			vorstatus.year					=  message[11];
			vorstatus.month					=  message[12];
			vorstatus.date					=  message[13];
			vorstatus.hour					=  message[14];
			vorstatus.min	  				=  message[15];
			vorstatus.sec	  				=  message[16];
			
			vorstatus.AzimuthAngle 	= (message[52]<<8)|message[51] ;
			vorstatus.Mod30Hz 			= (message[54]<<8)|message[53] ;
			vorstatus.Mod9960Hz 		= (message[56]<<8)|message[55] ;
			vorstatus.Deviation 		= (message[58]<<8)|message[57] ;
			vorstatus.RFlevel 			= (message[60]<<8)|message[59] ;
			
			if(vorstatus.Alert == 1) 						printf("Alert\r\n");
			if(vorstatus.LocalControl == 1) 		printf("Local\r\n");
			
			if(vorstatus.Tx1Main == 1) 					printf("Main Tx1\r\n");
			else if(vorstatus.Tx1Main == 0) 		printf("Main Tx2\r\n");
			
			if(vorstatus.Tx1Ant == 1) 					printf("Ant Tx1\r\n");
			else if(vorstatus.Tx1Ant == 0) 			printf("Ant Tx2\r\n");
			
			
			if(vorstatus.Tx1On == 1) 						printf("Tx1 ON\r\n");
			else if(vorstatus.Tx1On == 0)				printf("Tx1 OFF\r\n");
			
			if(vorstatus.Tx2On == 1) 						printf("Tx2 ON\r\n");
			else if(vorstatus.Tx2On == 0) 			printf("Tx2 OFF\r\n");
			
			if((vorstatus.i9b5 == 1)&&(vorstatus.Tx2On == 1)) 						printf("Tx2 LOAD\r\n");
			else if((vorstatus.i9b5 == 0)&&(vorstatus.Tx1On == 1))				printf("Tx1 LOAD\r\n");
			
			if(vorstatus.DualEquipment == 1) 		printf("DualEquipment ON\r\n");
			
			
			
			if(vorstatus.OnBattery == 1) 				printf("OnBattery ON\r\n");
			if(vorstatus.ACFail == 1) 					printf("ACFail ON\r\n");
			
			if(vorstatus.IntPriAlarm == 1) 		  printf("Integral PriAlarm\r\n");
			//ko tim thay bit nao la Integral SecAlarm
			if(vorstatus.IntMonBypass == 1) 		printf("Integral Bypass\r\n");
	
	
			
			//if(vorstatus.i13b4 == 1) 					printf("13b4 bypass all?\r\n");
			
			
			if(((vorstatus.Tx1Ant==1) && (vorstatus.Tx1Main==1) && (vorstatus.Tx1On==1)) || 
											((vorstatus.Tx1Ant==0) && (vorstatus.Tx1Main==0) && (vorstatus.Tx2On==1) && (vorstatus.DualEquipment==1)))
			{
				//Status = eMain;
				printf("Vor eMain\r\n");
				vorstatus.Status = 1;
			}
			else if((dmestatus.DualEquipment==1) 
							&&(	((vorstatus.Tx1Ant==1) 	&& (vorstatus.Tx1Main == 0) && (vorstatus.Tx1On == 1))||
									((vorstatus.Tx1Ant == 0)&& (vorstatus.Tx1Main == 1) && (vorstatus.Tx2On == 1) )	)	)
			{
				//Status = eSTANDBY;
				printf("Vor STANDBY\r\n");
				vorstatus.Status = 2;
			}
			else if(((vorstatus.Tx1On == 0)  && (vorstatus.Tx2On == 0))||((vorstatus.Tx1Ant == 1) && (vorstatus.Tx1On == 0))||((vorstatus.Tx1Ant == 0) && (vorstatus.Tx2On == 0)))
			{
				//Status = eOFF;
				printf("eOFF\r\n");
				vorstatus.Status = 0;
			}
			
			if((vorstatus.Status != 0)&&
				(vorstatus.LocalControl == 0) &&
				(vorstatus.Mon1Bypass == 0) &&
				(vorstatus.OnBattery == 0) &&
				(vorstatus.ACFail == 0) )
				{
					//printf("VOR Go condition\r\n");
				}
			else
			{
				//printf("VOR No Go condition ALARM\r\n");
			}
			
			
			
			printf("Ngay %d thang %d nam %d Gio %d Phut %d Giay %d\r\n",vorstatus.date,vorstatus.month,vorstatus.year, vorstatus.hour, vorstatus.min, vorstatus.sec);
			printf("AzimuthAngle: %d \r\nMod30Hz: %d \r\nMod9960Hz: %d \r\nDeviation :%d \r\nRFlevel: %d\r\n",vorstatus.AzimuthAngle,vorstatus.Mod30Hz,vorstatus.Mod9960Hz,vorstatus.Deviation,vorstatus.RFlevel);
		}
	}
	if((message[0] == 0xF8)&&(message[1] == 0xF8)&&(message[9] == 0x19))//1150 VOR status message
	{
		printf("\r\n----------1150 VOR PMDT---------------\r\n");
		crc16CCITT = CRC16_CCITT_calc((message+2),34,crc16CCITT);
		if(((message[37]<<8)|message[36]) == crc16CCITT) 
		{
			//printf("Right CRC16");
			vorPmdt = 1;
			vorstatus.Tx1Ant 						= bittest(message[offset+11],3);//11-b3
			vorstatus.Tx1Main 					= bittest(message[offset+11],4);//11-b4
			vorstatus.Tx1On 						= bittest(message[offset+11],5);//11-b5
			vorstatus.Tx2On 						= bittest(message[offset+11],6);//11-b6
			vorstatus.DualEquipment 		= bittest(message[offset+24],0);//24-b0
			vorstatus.LocalControl 			= bittest(message[offset+11],2);//11-b2
			
			vorstatus.Alert					    = bittest(message[offset+6],0);//6-b0
			vorstatus.Mon1Bypass 			  = bittest(message[offset+10],3);//10-b3
			vorstatus.i10b4 			      = bittest(message[offset+10],4);//10-b4 alarm
			vorstatus.i10b2 			      = bittest(message[offset+10],2);//10-b2 load
			
			//vorstatus.i10b0 			      = bittest(message[offset+10],0);//10-b0 
			//vorstatus.i10b1 			      = bittest(message[offset+10],1);//10-b1 
			//vorstatus.i10b5 			      = bittest(message[offset+10],5);//10-b5 
			
			vorstatus.OnBattery 				= bittest(message[offset+6],1);//6-b1
			vorstatus.ACFail						= bittest(message[offset+6],2);//6-b2
			
			vorstatus.AzimuthAngle 	= (message[26]<<8)|message[25] ;
			vorstatus.Mod30Hz 			= (message[28]<<8)|message[27] ;
			vorstatus.Mod9960Hz 		= (message[30]<<8)|message[29] ;
			vorstatus.Deviation 		= (message[32]<<8)|message[31] ;
			vorstatus.RFlevel 			= (message[34]<<8)|message[33] ;
			
			vorstatus.year					=  message[11];
			vorstatus.month					=  message[12];
			vorstatus.date					=  message[13];
			vorstatus.hour					=  message[14];
			vorstatus.min	  				=  message[15];
			vorstatus.sec	  				=  message[16];
			
			
			if(vorstatus.Alert == 1) 						printf("Alert\r\n");
			if(vorstatus.LocalControl == 1) 		printf("Local\r\n");
			
			if(vorstatus.Tx1Main == 1) 					printf("Main Tx1\r\n");
			else if(vorstatus.Tx1Main == 0) 		printf("Main Tx2\r\n");
			
			//if(vorstatus.Tx1Ant == 1) 					printf("Ant Tx1\r\n");
			//else if(vorstatus.Tx1Ant == 0) 			printf("Ant Tx2\r\n");
			
			
			//if(vorstatus.Tx1On == 1) 						printf("Tx1 ON\r\n");
			//else if(vorstatus.Tx1On == 0)				printf("Tx1 OFF\r\n");
			if(vorstatus.Tx1On == 0)				printf("Tx1 OFF\r\n");
			//if(vorstatus.Tx2On == 1) 						printf("Tx2 ON\r\n");
			//else if(vorstatus.Tx2On == 0) 			printf("Tx2 OFF\r\n");
			if(vorstatus.Tx2On == 0) 			printf("Tx2 OFF\r\n");
			
			if((vorstatus.i10b2 == 1)&&(vorstatus.Tx2On == 1)) 						printf("TX2 LOAD\r\n");
			if((vorstatus.i10b2 == 1)&&(vorstatus.Tx1On == 1)) 						printf("TX1 LOAD\r\n");
			
			if((vorstatus.Tx1Ant == 1)&&(vorstatus.Tx1On == 1))  					printf("Ant Tx1\r\n");
			else if((vorstatus.Tx1Ant == 0)&&(vorstatus.Tx2On == 1)) 			printf("Ant Tx2\r\n");
			
			if(vorstatus.i10b4 == 1) 						printf("Alarm\r\n");
			
			//if(vorstatus.i10b0 == 1) 						printf("i10b0\r\n");
			//if(vorstatus.i10b1 == 1) 						printf("i10b1\r\n");
			//if(vorstatus.i10b5 == 1) 						printf("i10b5\r\n");
			
			//if(vorstatus.Tx1Ant == 1) 					printf("Tx1Ant ON\r\n");
			//else if(vorstatus.Tx1Ant == 0) 			printf("Tx2Ant ON\r\n");
			//if(vorstatus.Tx1Main == 1) 					printf("Tx1Main ON\r\n");
			//else if(vorstatus.Tx1Main == 0) 		printf("Tx2Main ON\r\n");
			//if(vorstatus.Tx1On == 1) 						printf("Tx1On ON\r\n");
			//if(vorstatus.Tx2On == 1) 						printf("Tx2On ON\r\n");
			//if(vorstatus.DualEquipment == 1) 		printf("DualEquipment ON\r\n");
			//if(vorstatus.LocalControl == 1) 		printf("LocalControl ON\r\n");
			if(vorstatus.Mon1Bypass == 1) 			printf("Bypass\r\n");
			//if(vorstatus.OnBattery == 1) 				printf("OnBattery ON\r\n");
			//if(vorstatus.ACFail == 1) 					printf("ACFail ON\r\n");
			
			if(((vorstatus.Tx1Ant==1) && (vorstatus.Tx1Main==1) && (vorstatus.Tx1On==1)) || 
											((vorstatus.Tx1Ant==0) && (vorstatus.Tx1Main==0) && (vorstatus.Tx2On==1) && (vorstatus.DualEquipment==1)))
			{
				//Status = eMain;
				//printf("Vor Main\r\n");
				vorstatus.Status = 1;
			}
			else if((dmestatus.DualEquipment==1) 
							&&(	((vorstatus.Tx1Ant==1) 	&& (vorstatus.Tx1Main == 0) && (vorstatus.Tx1On == 1))||
									((vorstatus.Tx1Ant == 0)&& (vorstatus.Tx1Main == 1) && (vorstatus.Tx2On == 1) )	)	)
			{
				//Status = eSTANDBY;
				//printf("Vor STANDBY\r\n");
				vorstatus.Status = 2;
			}
			else if(((vorstatus.Tx1On == 0)  && (vorstatus.Tx2On == 0))||((vorstatus.Tx1Ant == 1) && (vorstatus.Tx1On == 0))||((vorstatus.Tx1Ant == 0) && (vorstatus.Tx2On == 0)))
			{
				//Status = eOFF;
				//printf("eOFF\r\n");
				vorstatus.Status = 0;
			}
			
			if((vorstatus.Status != 0)&&
				(vorstatus.LocalControl == 0) &&
				(vorstatus.Mon1Bypass == 0) &&
				(vorstatus.OnBattery == 0) &&
				(vorstatus.ACFail == 0) )
				{
					//printf("VOR Go condition\r\n");
				}
			else
			{
				//printf("VOR No Go condition ALARM\r\n");
			}
			
			printf("Ngay %d thang %d nam %d Gio %d Phut %d Giay %d\r\n",vorstatus.date,vorstatus.month,vorstatus.year, vorstatus.hour, vorstatus.min, vorstatus.sec);
			printf("AzimuthAngle: %d \r\nMod30Hz: %d \r\nMod9960Hz: %d \r\nDeviation :%d \r\nRFlevel: %d \r\n",vorstatus.AzimuthAngle,vorstatus.Mod30Hz,vorstatus.Mod9960Hz,vorstatus.Deviation,vorstatus.RFlevel);
		}
	}
	
	if((message[0] == 0xF8)&&(message[1] == 0xF8)&&(message[9] == 0x34))//DME status message
	{
		printf("\r\n------------------DME---------------------------\r\n");
		crc16CCITT = CRC16_CCITT_calc((message+2),61,crc16CCITT);
		//printf("U1 CRC16 CCITT: 0x%4X %4X \r\n",crc16CCITT,(message[64]<<8)|message[63]);
		if(((message[64]<<8)|message[63]) == crc16CCITT) 
		{
			//printf("Right CRC16");
			dmeconn = 1;
			dmestatus.DelayUs 			= (message[32]<<8)|message[31] ;
			dmestatus.SpacingUs			= (message[34]<<8)|message[33];
			dmestatus.TxPowerWatts	= (message[36]<<8)|message[35];
			dmestatus.Efficiency 		= (message[38]<<8)|message[37];
			dmestatus.PRFppps				= (message[40]<<8)|message[39];
			dmestatus.ERP						=  message[52];
			dmestatus.year					=  message[11];
			dmestatus.month					=  message[12];
			dmestatus.date					=  message[13];
			dmestatus.hour					=  message[14];
			dmestatus.min	  				=  message[15];
			dmestatus.sec	  				=  message[16];
			
			dmestatus.MaintenanceAlert 	= bittest(message[offset+8],0); //8-b0
			dmestatus.Tx1Ant 						= bittest(message[offset+13],0);//13-b0
			dmestatus.Tx1Main 					= bittest(message[offset+13],1);//13-b1
			dmestatus.Tx1On 						= bittest(message[offset+13],2);//13-b2
			dmestatus.Tx2On 						= bittest(message[offset+13],3);//13-b3
			dmestatus.DualEquipment 		= bittest(message[offset+15],0);//15-b0
			dmestatus.LocalControl 			= bittest(message[offset+12],4);//12-b4
			dmestatus.IntPriAlarm 			= bittest(message[offset+10],0);//10-b0
			dmestatus.IntMonBypass 			= bittest(message[offset+10],2);//10-b2
			dmestatus.OnBattery 				= bittest(message[offset+8],2);//8-b2
			dmestatus.ACFail						= bittest(message[offset+8],3);//8-b3
			
			dmestatus.i9b5						  = bittest(message[offset+9],5);//9-b5
			//dmestatus.i10b2						  = bittest(message[offset+10],2);//10-b2
			dmestatus.i10b3						  = bittest(message[offset+10],3);//10-b3
			dmestatus.i10b5						  = bittest(message[offset+10],5);//10-b5
			dmestatus.i11b0						  = bittest(message[offset+11],0);//11-b0
			dmestatus.i11b1						  = bittest(message[offset+11],1);//11-b1
			//dmestatus.i13b4						  = bittest(message[offset+13],4);//13-b4
			
			if(dmestatus.MaintenanceAlert == 1) printf("Alert\r\n");
			if(dmestatus.LocalControl == 1) 		printf("Local\r\n");
			if(dmestatus.DualEquipment == 1) 		printf("Dual Equipment\r\n");
			
			if(dmestatus.Tx1Main == 1) 					printf("Main Tx1\r\n");
			else if(dmestatus.Tx1Main == 0) 		printf("Main Tx2\r\n");
			
			if((dmestatus.Tx1Ant == 1)&&(dmestatus.Tx1On == 1)) 					printf("Ant Tx1\r\n");
			else if((dmestatus.Tx1Ant == 0) &&(dmestatus.Tx2On == 1))			printf("Ant Tx2\r\n");
			
			if((dmestatus.i9b5 == 1)&&(dmestatus.Tx2On == 1)) 						printf("Tx2 LOAD\r\n");
			else if((dmestatus.i9b5 == 0)&&(dmestatus.Tx1On == 1))				printf("Tx1 LOAD\r\n");
			
			if(dmestatus.Tx1On == 1) 						printf("Tx1 ON\r\n");
			else if(dmestatus.Tx1On == 0)				printf("Tx1 OFF\r\n");
			
			if(dmestatus.Tx2On == 1) 						printf("Tx2 ON\r\n");
			else if(dmestatus.Tx2On == 0)				printf("Tx2 OFF\r\n");
			
			
			
			if(dmestatus.IntPriAlarm == 1) 			printf("Integral PriAlarm\r\n");
			if(dmestatus.i11b0 == 1) 					  printf("Integral SecAlarm\r\n");//?
			if(dmestatus.IntMonBypass == 1) 		printf("Integral Bypass\r\n");
			
			//if(dmestatus.i10b2 == 1) 					printf("Integral Bypass \r\n");
			
			if(dmestatus.i10b3 == 1) 					printf("Standby PriAlarm \r\n");
			if(dmestatus.i11b1 == 1) 					printf("Standby SecAlarm \r\n");
			if(dmestatus.i10b5 == 1) 					printf("Standby Bypass \r\n");
			//if(dmestatus.i13b4 == 1) 					printf("13b4 bypass all\r\n");
			if(dmestatus.OnBattery == 1) 				printf("OnBattery\r\n");
			if(dmestatus.ACFail == 1) 					printf("ACFail\r\n");
			
			if(((dmestatus.Tx1Ant==1) && (dmestatus.Tx1Main==1) && (dmestatus.Tx1On==1)) || 
											((dmestatus.Tx1Ant==0) && (dmestatus.Tx1Main==0) && (dmestatus.Tx2On==1) && (dmestatus.DualEquipment==1)))
			{
				//Status = eMain;
				printf("eMain\r\n");
				dmestatus.Status = 1;
			}
			else if((dmestatus.DualEquipment==1) 
							&&(	((dmestatus.Tx1Ant==1) && (dmestatus.Tx1Main == 0) && (dmestatus.Tx1On == 1))||
									((dmestatus.Tx1Ant == 0)&& (dmestatus.Tx1Main == 1) && (dmestatus.Tx2On == 1) )	)	)
			{
				//Status = eSTANDBY;
				printf("eSTANDBY\r\n");
				dmestatus.Status = 2;
			}
			else if(((dmestatus.Tx1On == 0)  && (dmestatus.Tx2On == 0))||((dmestatus.Tx1Ant == 1) && (dmestatus.Tx1On == 0))||((dmestatus.Tx1Ant == 0) && (dmestatus.Tx2On == 0)))
			{
				//Status = eOFF;
				printf("eOFF\r\n");
				dmestatus.Status = 0;
			}
			
			if((dmestatus.Status != 0)&&
				(dmestatus.LocalControl == 0) &&
				(dmestatus.IntPriAlarm == 0) &&
				(dmestatus.IntMonBypass == 0) &&
				(dmestatus.OnBattery == 0) &&
				(dmestatus.ACFail == 0) )
				{
					printf("Go condition\r\n");
				}
			else
			{
				printf("No Go condition ALARM\r\n");
			}
			printf("Ngay %d thang %d nam %d Gio %d Phut %d Giay %d\r\n",dmestatus.date,dmestatus.month,dmestatus.year, dmestatus.hour, dmestatus.min, dmestatus.sec);
			printf("DME Delay: %d us,Spacing: %d us,TxPower: %d Watts,Efficiency :%d %%,PRF: %d ppps,ERP: %d\r\n",dmestatus.DelayUs,dmestatus.SpacingUs,dmestatus.TxPowerWatts,dmestatus.Efficiency,dmestatus.PRFppps,dmestatus.ERP);
		}
	}
	
}


