/*
NOTE: Chua chuyen UART ve 9600
DVOR 1150A, DVOR 1150 PMDT:
- Trang thai: ON, OFF, alarm, alert, local, normal, main
- Ky thuat:
	+ Azimuth angle.
	+ 30Hz Modulation.
	+ 9960 Hz Modulation.
	+ Deviation.
	+ RF level
DME 1119A:
- Trang thai: ON, OFF, alarm, alert, local, normal.
- Ky thuat:
	+ Delay. 
	+ Spacing.
	+ Tx Power.
	+ ERP.
	+ Efficiency.
	+ PRF
Loai cu (VOR 1150 Hyper terminal, DME 1119): Tinh trang on/off, alarm, bypass

- 16/Jan/2020: chuyen project sang cho SMNP
- 9/Jan/2020: fix loi thoi gian nhan vao qua uart la UTC nhung thoi gian gui qua NTP la UTC + 7
							Them web config cua Tung va fullconfig.html
- 3/Jan/2020: Thoi gian GPS la UTC nen ko can tru 7h 
- Neu thoi gian cua module ma khac gio that qua nhieu thi win10 se ko cap nhat vi thoi gian khac nhieu qua
- 25/Dec/2019:
 + Add factory reset system
 + If lost master message, update to webserver => lost signal
- 24/Dec/2019: 
 + Sua loi phan nhan ban tin UART, cap nhat sai gio.
 + Time ref chuyen vao phan nhan ban tin UART
 + Add timeout of GPS master message
 + Cai tien phan kiem tra day mang co cam hay ko?
 + Them phan timeout of master vao SNMP table
- 20/Dec/2019: Fix mot so loi
	+ Phai chuyen toc do UART thanh 9600 de phu hop voi dong ho GPS clock
- 23/Oct/2019 : Make new project on NTP PCB new, STM32F103RBT6. Check...
								redefine STM32F10X_MD
								HSE = 8MHz
- 03/Sep/2019 : Make new project on STM32F103RCT6 WIZ550web https://www.wiznet.io/product-item/wiz550web/
- Xtal : 12Mhz trong file system_stm32f10x.c thay doi SetSysClockTo72() : RCC_CFGR_PLLMULL9 => RCC_CFGR_PLLMULL6 do XT = 12Mhz
- kiem tra printmcuclk() dam bao :
ADCCLK:36000000
HCLK:72000000
PCLK1:36000000
PCLK2:72000000
SYSCLK:72000000

- UART1 : debug ok
- STM32 Independent WatchDog : 13s
- Timer_Configuration() :
- LED 0 : CPU run, blinky one per second
- NTP time server
- SMNP Agent
- Get time from ntp server
- httpServer
**/
//#define HSE_VALUE    ((uint32_t)12000000) /*!< Value of the External oscillator in Hz */
//
#include "main.h"
#define _U1_DEBUG_ENABLE_

time_t timenow = 1579251699;
volatile uint8_t sec_cnt = 0;
int8_t ledstt;
// lost signal : if after timeOutLostSignal seconds without GPS master message => lost
int8_t lostSignal = LOST_GPS_MASTER;
int8_t timeOutLostSignal = 30;
uint8_t gps1_stt = 0;
uint8_t gps2_stt = 0;
uint8_t power1_stt = 0;
uint8_t power2_stt = 0;
uint8_t days = 0;
uint8_t months = 0;
uint8_t years = 0;
uint8_t hours = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;
struct tm currtime;
void factoryRST(void);

#include "crc16CCITT.h"
unsigned short crc16CCITT;
//uint8_t texttest[]={0xF8, 0xF8, 0xB0, 0xF0, 0x0A, 0x00, 0x00, 0x03, 0x02, 0x06, 0x00, 0x0F, 0x03, 0x6F, 0xFB, 0x2D, 0x35, 0x88, 0x75 };

uint8_t Vor_AngleH, Vor_AngleL;
uint8_t Vor_30ModH, Vor_30ModL;
uint8_t Vor_9960ModH, Vor_9960ModL;
uint8_t Vor_DevH, Vor_DevL;
uint8_t Vor_RFH, Vor_RFL;
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
		uint8_t  i9b5;//LOAD
		
		
		
		//uint8_t  i13b4;
}vor_Status;
vor_Status vorstatus;

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
dme_Status dmestatus;
/**************************************************************************/
int main(void)
{	
	int32_t ret = 0;
  int i;
	char x;
	
  SystemInit();
	SystemCoreClockUpdate();
	/* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(SystemCoreClock / 100))
  { 
    /* Capture error */ 
    while (1);
  }


	
	//Thu vien tang thap
	WIZ550webGPIO_config();
	Timer_Configuration();
	SPI2_Init();
	USART1_Init();
	//printmcuclk();
	
	//using stm32's flash to store data
	//EEPROM STM32 init
	sw_eeprom_stm32();
	test_eeprom();
	//delay_ms(1);
	//Thu vien tang cao
	//Neu ham nay ko hoat dong: kiem tra cac chan GPIO khai bao!
	//w5500_lib_init();


	#ifdef _U1_DEBUG_ENABLE_
	printf("Run, now is %s\r\n",ctime(&timenow));
	#endif
	/*
	for(i=0;i<15;i++)
	{
		printf("0x%X ",texttest[i]);
	}
	printf("\r\n");
	*/
	
	
	
	/*
	crc16CCITT = CRC16_CCITT_calc((texttest+2),15,crc16CCITT);
	printf("CRC16 CCITT: 0x%4X\r\n",crc16CCITT);
	x = 0x73;
	
	if(bittest(x,0) == 1) printf("Bit 0 is ON\r\n");
	if(bittest(x,1) == 1) printf("Bit 1 is ON\r\n");
	if(bittest(x,2) == 1) printf("Bit 2 is ON\r\n");	
	if(bittest(x,3) == 1) printf("Bit 3 is ON\r\n");
	if(bittest(x,4) == 1) printf("Bit 4 is ON\r\n");
	if(bittest(x,5) == 1) printf("Bit 5 is ON\r\n");
	if(bittest(x,6) == 1) printf("Bit 6 is ON\r\n");
	if(bittest(x,7) == 1) printf("Bit 7 is ON\r\n");
	*/
	#ifdef _GET_TIME_INTERNET_ENABLE_
	//Get time from ntp time server
	SNTP_init();
	#endif
	
	//ntpserverdefaultconfig();
	/* SNMP(Simple Network Management Protocol) Agent Initialize */
	// NMS (SNMP manager) IP address
	//snmpd_init(managerIP,agentIP,SOCK_agent,SOCK_trap);	
	//loadwebpages();
	
	// IWDG Initialization: STM32 Independent WatchDog
	IWDG_Config();
	//timeinfo = localtime( &timenow );
	//printf("Current local time and date: %s\r\n", asctime(timeinfo));
	/*****************************MAIN WHILE************************************/
	while(1)
	{
		IWDG_ReloadCounter(); // Feed IWDG
		/**********************************************************************/
		//networkSevices();
		/**********************************************************************/
		if(GPIO_PinRead(GPIOC,0) ==0 ) 
		{
			delay_ms(2000);
			if(GPIO_PinRead(GPIOC,0) ==0 ) printf("Reset factory setting\r\n");
			factoryRST();
			NVIC_SystemReset();
		}
		/**********************************************************************/
		// Tu chay dong ho va cap nhat thoi gian tu Internet
		if(sec_cnt >= 1)
		{
			sec_cnt = 0;
			#ifdef _GET_TIME_INTERNET_ENABLE_
			//timeinfo = localtime( &timenow );
			//printf("sec_cnt :%d, timenow :%d\r\n",sec_cnt,timenow);
			//Neu ko co mang cai nay se gay treo??? 19/Sep/2019
			SNTP_run();
			//GPIO_PinWrite(GPIOA, 8, 1);
			seconds++;
			if(seconds == 60) seconds = 0;
			#endif

			if(timeOutLostSignal) timeOutLostSignal--;
			if(timeOutLostSignal == 0) lostSignal = LOST_GPS_MASTER;
		}
		/**********************************************************************/
		//LED blinky : CPU run, neu ko nhay thi la co van de!  
		
		if((msec_cnt < 100) &&(ledstt != 1)) {ledstt = 1;GPIO_PinWrite(GPIOC, 1, 0);}
		else if((msec_cnt >= 100) &&(ledstt != 0)) {ledstt = 0 ; GPIO_PinWrite(GPIOC, 1, 1);}
		/**********************************************************************/
		//Xu ly thoi gian tu mach GPS master gui sang, neu ko nhan dc thi phai bao TIMEOUT
		usart1Process();
		
		

	}//end of main while

		
}//end of main

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
		gWIZNETINFO.ip[0] = 192;
		gWIZNETINFO.ip[1] = 168;
		gWIZNETINFO.ip[2] = 22;
		gWIZNETINFO.ip[3] = 165;
		//GW: 192.168.1.1
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
	
void networkSevices(void)
{
	int32_t ret = 0;	
	checklink();//Kiem tra day mang con cam ko
	// NTP UDP server chay dau tien cho nhanh
	if( (ret = NTPUDP(SOCK_UDPS)) < 0) {
			printf("SOCKET ERROR : %d\r\n", ret);
	}
	{	//SNMPv1 run
			//Run SNMP Agent Fucntion
			/* SNMP Agent Handler */
			//SMI Network Management Private Enterprise Codes: : moi cong ty phai dang ky 1 so rieng, 
			//tham khao : https://www.iana.org/assignments/enterprise-numbers/enterprise-numbers
			// Vi du Arduino : 36582
    	// SNMP Agent daemon process : User can add the OID and OID mapped functions to snmpData[] array in snmprun.c/.h
			// [net-snmp version 5.7 package for windows] is used for this demo.
			// * Command example
    	// [Command] Get:			  snmpget -v 1 -c public 192.168.1.165 .1.3.6.1.2.1.1.1.0 			// (sysDescr)
    	// [Command] Get: 			snmpget -v 1 -c public 192.168.1.165 .1.3.6.1.4.1.6.1.0 			// (Custom, get LED status)
    	// [Command] Get-Next: 	snmpwalk -v 1 -c public 192.168.1.165 .1.3.6.1
			// [Command] Set: 			snmpset -v 1 -c public 192.168.1.165 .1.3.6.1.4.1.6.1.1 i 1			// (Custom, LED 'On')
    	// [Command] Set: 			snmpset -v 1 -c public 192.168.1.165 .1.3.6.1.4.1.6.1.1 i 0			// (Custom, LED 'Off')
			snmpd_run();	
	}
	{	// web server 	
			httpServer_run(0);
			httpServer_run(1);
			httpServer_run(2);
		}
}
/**********************************************************************/
//Ham chuyen doi char sang int
uint8_t convert_atoi( uint8_t c)
{
	return (uint8_t)c-48;
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
			vorstatus.Tx1Ant					= bittest(message[offset+13],0);//13-b0
			vorstatus.Tx1Main 					= bittest(message[offset+13],1);//13-b1
			vorstatus.Tx1On						= bittest(message[offset+13],2);//13-b2
			vorstatus.Tx2On						= bittest(message[offset+13],3);//13-b3
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
					printf("VOR Go condition\r\n");
				}
			else
			{
				printf("VOR No Go condition ALARM\r\n");
			}
			
			
			
			printf("Ngay %d thang %d nam %d Gio %d Phut %d Giay %d\r\n",vorstatus.date,vorstatus.month,vorstatus.year, vorstatus.hour, vorstatus.min, vorstatus.sec);
			printf("AzimuthAngle: %d \r\nMod30Hz: %d \r\nMod9960Hz: %d \r\nDeviation :%d \r\nRFlevel: %d\r\n",vorstatus.AzimuthAngle,vorstatus.Mod30Hz,vorstatus.Mod9960Hz,vorstatus.Deviation,vorstatus.RFlevel);
		}
	}
	if((message[0] == 0xF8)&&(message[1] == 0xF8)&&(message[9] == 0x19))//1150 VOR status message
	{
		printf("\r\n1150 VOR\r\n");
		crc16CCITT = CRC16_CCITT_calc((message+2),34,crc16CCITT);
		if(((message[37]<<8)|message[36]) == crc16CCITT) 
		{
			//printf("Right CRC16");
			vorstatus.Tx1Ant 						= bittest(message[offset+11],3);//11-b3
			vorstatus.Tx1Main 					= bittest(message[offset+11],4);//11-b4
			vorstatus.Tx1On 						= bittest(message[offset+11],5);//11-b5
			vorstatus.Tx2On 						= bittest(message[offset+11],6);//11-b6
			vorstatus.DualEquipment 		= bittest(message[offset+24],0);//24-b0
			vorstatus.LocalControl 			= bittest(message[offset+11],2);//11-b2
			
			vorstatus.Mon1Bypass 			  = bittest(message[offset+10],3);//10-b3
			vorstatus.OnBattery 				= bittest(message[offset+6],1);//6-b1
			vorstatus.ACFail						= bittest(message[offset+6],2);//6-b2
			
			vorstatus.AzimuthAngle 	= (message[26]<<8)|message[25] ;
			vorstatus.Mod30Hz 			= (message[28]<<8)|message[27] ;
			vorstatus.Mod9960Hz 		= (message[30]<<8)|message[29] ;
			vorstatus.Deviation 		= (message[32]<<8)|message[31] ;
			vorstatus.RFlevel 			= (message[34]<<8)|message[33] ;
			
			if(vorstatus.Tx1Ant == 1) 					printf("Tx1Ant ON\r\n");
			else if(vorstatus.Tx1Ant == 0) 			printf("Tx2Ant ON\r\n");
			if(vorstatus.Tx1Main == 1) 					printf("Tx1Main ON\r\n");
			else if(vorstatus.Tx1Main == 0) 		printf("Tx2Main ON\r\n");
			if(vorstatus.Tx1On == 1) 						printf("Tx1On ON\r\n");
			if(vorstatus.Tx2On == 1) 						printf("Tx2On ON\r\n");
			if(vorstatus.DualEquipment == 1) 		printf("DualEquipment ON\r\n");
			if(vorstatus.LocalControl == 1) 		printf("LocalControl ON\r\n");
			if(vorstatus.Mon1Bypass == 1) 			printf("Mon1Bypass ON\r\n");
			if(vorstatus.OnBattery == 1) 				printf("OnBattery ON\r\n");
			if(vorstatus.ACFail == 1) 					printf("ACFail ON\r\n");
			
			if(((vorstatus.Tx1Ant==1) && (vorstatus.Tx1Main==1) && (vorstatus.Tx1On==1)) || 
											((vorstatus.Tx1Ant==0) && (vorstatus.Tx1Main==0) && (vorstatus.Tx2On==1) && (vorstatus.DualEquipment==1)))
			{
				//Status = eMain;
				printf("Vor Main\r\n");
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
					printf("VOR Go condition\r\n");
				}
			else
			{
				printf("VOR No Go condition ALARM\r\n");
			}
			
			
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
	
void u2_message_handle()
{
}
void u1_message_handle()
{//=> Ban tin GPS: $GPS034007060819AA10	
	int offset;
	offset = 11;
	//[10:27:01.503] F8 F8 
	//C0 F0 08 00 00 00 00 97 00 14 01
  //14 17 1B 36 00 00 C0 87 00 00 01 
	//88 06 2B 05 09 95 00 00 00 00 00 
	//00 00 00 00 00 00 00 00 00 00 00 
	//00 00 00 00 00 95 8C 30 01 25 01 
	//32 06 02 00 00 00 00 00 00 00 00 
	//00 00 00 00 00 00 00 00 00 00 00 
	//00 00 00 00 00 00 00 00 0C 00 00 
	//00 00 00 00 00 00 00 00 00 00 00 
	//00 00 00 00 00 00 00 00 00 00 00 
	//00 00 00 00 00 00 00 00 00 00 00 
	//00 00 00 00 00 00 00 00 00 00 00 
	//00 00 00 00 00 00 00 00 00 00 00 
	//00 00 00 00 00 48 00 00 00 B7 C5 
	//44 78 6F FB 2D 35 7F 11 
	if((USART1_rx_data_buff[0] == 0xF8)&&(USART1_rx_data_buff[1] == 0xF8)&&(USART1_rx_data_buff[9] == 0x97))//1150A VOR status message, 0x97 la do dai cua ban tin?
	{
		printf("\r\n1150A VOR\r\n");
		crc16CCITT = CRC16_CCITT_calc((USART1_rx_data_buff+2),160,crc16CCITT);
		if(((USART1_rx_data_buff[163]<<8)|USART1_rx_data_buff[162]) == crc16CCITT) 
		{
			//printf("Right CRC16");
			vorstatus.Tx1Ant 						= bittest(USART1_rx_data_buff[offset+13],0);//13-b0
			vorstatus.Tx1Main 					= bittest(USART1_rx_data_buff[offset+13],1);//13-b1
			vorstatus.Tx1On 						= bittest(USART1_rx_data_buff[offset+13],2);//13-b2
			vorstatus.Tx2On 						= bittest(USART1_rx_data_buff[offset+13],3);//13-b3
			vorstatus.DualEquipment 		= bittest(USART1_rx_data_buff[offset+15],1);//15-b1
			vorstatus.LocalControl 			= bittest(USART1_rx_data_buff[offset+12],4);//12-b4
			vorstatus.IntPriAlarm 			= bittest(USART1_rx_data_buff[offset+10],6);//10-b6
			vorstatus.IntMonBypass 			= bittest(USART1_rx_data_buff[offset+11],0);//11-b0
			vorstatus.OnBattery 				= bittest(USART1_rx_data_buff[offset+8],2);//8-b2
			vorstatus.ACFail						= bittest(USART1_rx_data_buff[offset+8],3);//8-b3
			
			vorstatus.year					=  USART1_rx_data_buff[11];
			vorstatus.month					=  USART1_rx_data_buff[12];
			vorstatus.date					=  USART1_rx_data_buff[13];
			vorstatus.hour					=  USART1_rx_data_buff[14];
			vorstatus.min	  				=  USART1_rx_data_buff[15];
			vorstatus.sec	  				=  USART1_rx_data_buff[16];
			
			if(vorstatus.Tx1Ant == 1) 					printf("Tx1Ant ON\r\n");
			else if(vorstatus.Tx1Ant == 0) 			printf("Tx2Ant ON\r\n");
			if(vorstatus.Tx1Main == 1) 					printf("Tx1Main ON\r\n");
			else if(vorstatus.Tx1Main == 0) 		printf("Tx2Main ON\r\n");
			if(vorstatus.Tx1On == 1) 						printf("Tx1On ON\r\n");
			if(vorstatus.Tx2On == 1) 						printf("Tx2On ON\r\n");
			if(vorstatus.DualEquipment == 1) 		printf("DualEquipment ON\r\n");
			if(vorstatus.LocalControl == 1) 		printf("LocalControl ON\r\n");
			if(vorstatus.IntMonBypass == 1) 		printf("IntMonBypass ON\r\n");
			if(vorstatus.IntPriAlarm == 1) 		  printf("IntPriAlarm ON\r\n");
			if(vorstatus.OnBattery == 1) 				printf("OnBattery ON\r\n");
			if(vorstatus.ACFail == 1) 					printf("ACFail ON\r\n");
			
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
					printf("VOR Go condition\r\n");
				}
			else
			{
				printf("VOR No Go condition ALARM\r\n");
			}
			//F8 F8 C0 F0 08 00 00 00 00 97 00
			//14 01 14 17 1B 36 00 00 C0 87 00
			//00 01 88 06 2B 05 09 95 00 00 00 
			//00 00 00 00 00 00 00 00 00 00 00 
			//00 00 00 00 00 00 00 95 8C 30 01 25 01 32 06 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0C 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 48 00 00 00 B7 C5 44 78 6F FB 2D 35 
			vorstatus.AzimuthAngle 	= (USART1_rx_data_buff[52]<<8)|USART1_rx_data_buff[51] ;
			vorstatus.Mod30Hz 			= (USART1_rx_data_buff[54]<<8)|USART1_rx_data_buff[53] ;
			vorstatus.Mod9960Hz 		= (USART1_rx_data_buff[56]<<8)|USART1_rx_data_buff[55] ;
			vorstatus.Deviation 		= (USART1_rx_data_buff[58]<<8)|USART1_rx_data_buff[57] ;
			vorstatus.RFlevel 			= (USART1_rx_data_buff[60]<<8)|USART1_rx_data_buff[59] ;
			
			printf("Ngay %d thang %d nam %d Gio %d Phut %d Giay %d\r\n",vorstatus.date,vorstatus.month,vorstatus.year, vorstatus.hour, vorstatus.min, vorstatus.sec);
			printf("AzimuthAngle: %d \r\nMod30Hz: %d \r\nMod9960Hz: %d \r\nDeviation :%d \r\nRFlevel: %d\r\n",vorstatus.AzimuthAngle,vorstatus.Mod30Hz,vorstatus.Mod9960Hz,vorstatus.Deviation,vorstatus.RFlevel);
		}
	}
	if((USART1_rx_data_buff[0] == 0xF8)&&(USART1_rx_data_buff[1] == 0xF8)&&(USART1_rx_data_buff[9] == 0x19))//1150 VOR status message
	{
		printf("\r\n1150 VOR\r\n");
		crc16CCITT = CRC16_CCITT_calc((USART1_rx_data_buff+2),34,crc16CCITT);
		if(((USART1_rx_data_buff[37]<<8)|USART1_rx_data_buff[36]) == crc16CCITT) 
		{
			//printf("Right CRC16");
			vorstatus.Tx1Ant 						= bittest(USART1_rx_data_buff[offset+11],3);//11-b3
			vorstatus.Tx1Main 					= bittest(USART1_rx_data_buff[offset+11],4);//11-b4
			vorstatus.Tx1On 						= bittest(USART1_rx_data_buff[offset+11],5);//11-b5
			vorstatus.Tx2On 						= bittest(USART1_rx_data_buff[offset+11],6);//11-b6
			vorstatus.DualEquipment 		= bittest(USART1_rx_data_buff[offset+24],0);//24-b0
			vorstatus.LocalControl 			= bittest(USART1_rx_data_buff[offset+11],2);//11-b2
			
			vorstatus.Mon1Bypass 			  = bittest(USART1_rx_data_buff[offset+10],3);//10-b3
			vorstatus.OnBattery 				= bittest(USART1_rx_data_buff[offset+6],1);//6-b1
			vorstatus.ACFail						= bittest(USART1_rx_data_buff[offset+6],2);//6-b2
			
			
			if(vorstatus.Tx1Ant == 1) 					printf("Tx1Ant ON\r\n");
			else if(vorstatus.Tx1Ant == 0) 			printf("Tx2Ant ON\r\n");
			if(vorstatus.Tx1Main == 1) 					printf("Tx1Main ON\r\n");
			else if(vorstatus.Tx1Main == 0) 		printf("Tx2Main ON\r\n");
			if(vorstatus.Tx1On == 1) 						printf("Tx1On ON\r\n");
			if(vorstatus.Tx2On == 1) 						printf("Tx2On ON\r\n");
			if(vorstatus.DualEquipment == 1) 		printf("DualEquipment ON\r\n");
			if(vorstatus.LocalControl == 1) 		printf("LocalControl ON\r\n");
			if(vorstatus.Mon1Bypass == 1) 			printf("Mon1Bypass ON\r\n");
			if(vorstatus.OnBattery == 1) 				printf("OnBattery ON\r\n");
			if(vorstatus.ACFail == 1) 					printf("ACFail ON\r\n");
			
			if(((vorstatus.Tx1Ant==1) && (vorstatus.Tx1Main==1) && (vorstatus.Tx1On==1)) || 
											((vorstatus.Tx1Ant==0) && (vorstatus.Tx1Main==0) && (vorstatus.Tx2On==1) && (vorstatus.DualEquipment==1)))
			{
				//Status = eMain;
				printf("Vor Main\r\n");
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
					printf("VOR Go condition\r\n");
				}
			else
			{
				printf("VOR No Go condition ALARM\r\n");
			}
			
			vorstatus.AzimuthAngle 	= (USART1_rx_data_buff[26]<<8)|USART1_rx_data_buff[25] ;
			vorstatus.Mod30Hz 			= (USART1_rx_data_buff[28]<<8)|USART1_rx_data_buff[27] ;
			vorstatus.Mod9960Hz 		= (USART1_rx_data_buff[30]<<8)|USART1_rx_data_buff[29] ;
			vorstatus.Deviation 		= (USART1_rx_data_buff[32]<<8)|USART1_rx_data_buff[31] ;
			vorstatus.RFlevel 			= (USART1_rx_data_buff[34]<<8)|USART1_rx_data_buff[33] ;
			printf("AzimuthAngle: %d \r\nMod30Hz: %d \r\nMod9960Hz: %d \r\nDeviation :%d \r\nRFlevel: %d \r\n",vorstatus.AzimuthAngle,vorstatus.Mod30Hz,vorstatus.Mod9960Hz,vorstatus.Deviation,vorstatus.RFlevel);
		}
	}
	if((USART1_rx_data_buff[0] == 0xF8)&&(USART1_rx_data_buff[1] == 0xF8)&&(USART1_rx_data_buff[9] == 0x34))//DME status message
	{
		printf("\r\nDME\r\n");
		crc16CCITT = CRC16_CCITT_calc((USART1_rx_data_buff+2),61,crc16CCITT);
		//printf("U1 CRC16 CCITT: 0x%4X %4X \r\n",crc16CCITT,(USART1_rx_data_buff[64]<<8)|USART1_rx_data_buff[63]);
		if(((USART1_rx_data_buff[64]<<8)|USART1_rx_data_buff[63]) == crc16CCITT) 
		{
			//printf("Right CRC16");
			dmestatus.DelayUs 			= (USART1_rx_data_buff[32]<<8)|USART1_rx_data_buff[31] ;
			dmestatus.SpacingUs			= (USART1_rx_data_buff[34]<<8)|USART1_rx_data_buff[33];
			dmestatus.TxPowerWatts	= (USART1_rx_data_buff[36]<<8)|USART1_rx_data_buff[35];
			dmestatus.Efficiency 		= (USART1_rx_data_buff[38]<<8)|USART1_rx_data_buff[37];
			dmestatus.PRFppps				= (USART1_rx_data_buff[40]<<8)|USART1_rx_data_buff[39];
			dmestatus.ERP						=  USART1_rx_data_buff[52];
			dmestatus.year					=  USART1_rx_data_buff[11];
			dmestatus.month					=  USART1_rx_data_buff[12];
			dmestatus.date					=  USART1_rx_data_buff[13];
			dmestatus.hour					=  USART1_rx_data_buff[14];
			dmestatus.min	  				=  USART1_rx_data_buff[15];
			dmestatus.sec	  				=  USART1_rx_data_buff[16];
			
			dmestatus.MaintenanceAlert 	= bittest(USART1_rx_data_buff[offset+8],0); //8-b0
			dmestatus.Tx1Ant 						= bittest(USART1_rx_data_buff[offset+13],0);//13-b0
			dmestatus.Tx1Main 					= bittest(USART1_rx_data_buff[offset+13],1);//13-b1
			dmestatus.Tx1On 						= bittest(USART1_rx_data_buff[offset+13],2);//13-b2
			dmestatus.Tx2On 						= bittest(USART1_rx_data_buff[offset+13],3);//13-b3
			dmestatus.DualEquipment 		= bittest(USART1_rx_data_buff[offset+15],0);//15-b0
			dmestatus.LocalControl 			= bittest(USART1_rx_data_buff[offset+12],4);//12-b4
			dmestatus.IntPriAlarm 			= bittest(USART1_rx_data_buff[offset+10],0);//10-b0
			dmestatus.IntMonBypass 			= bittest(USART1_rx_data_buff[offset+10],2);//10-b2
			dmestatus.OnBattery 				= bittest(USART1_rx_data_buff[offset+8],2);//8-b2
			dmestatus.ACFail						= bittest(USART1_rx_data_buff[offset+8],3);//8-b3
			
			if(dmestatus.MaintenanceAlert == 1) printf("MaintenanceAlert ON\r\n");
			if(dmestatus.Tx1Ant == 1) 					printf("Tx1Ant ON\r\n");
			else if(dmestatus.Tx1Ant == 0) 			printf("Tx2Ant ON\r\n");
			if(dmestatus.Tx1Main == 1) 					printf("Tx1Main ON\r\n");
			else if(dmestatus.Tx1Main == 0) 		printf("Tx2Main ON\r\n");
			if(dmestatus.Tx1On == 1) 						printf("Tx1On ON\r\n");
			if(dmestatus.Tx2On == 1) 						printf("Tx2On ON\r\n");
			if(dmestatus.DualEquipment == 1) 		printf("DualEquipment ON\r\n");
			if(dmestatus.LocalControl == 1) 		printf("LocalControl ON\r\n");
			if(dmestatus.IntPriAlarm == 1) 			printf("IntPriAlarm ON\r\n");
			if(dmestatus.IntMonBypass == 1) 		printf("IntMonBypass ON\r\n");
			if(dmestatus.OnBattery == 1) 				printf("OnBattery ON\r\n");
			if(dmestatus.ACFail == 1) 					printf("ACFail ON\r\n");
			
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
//Xu ly ban tin GPS
void usart1Process(void)
{
	int i;
	//UART1 RX process
			if(u1out == ONTIME)
			{
				u1out = STOP;// Da nhan du ban tin UART => Xy ly
				#ifdef _U1_DEBUG_ENABLE_
				//printf("UART1:%s\r\n",USART1_rx_data_buff);
				#endif
				//u1_message_handle();
				prasing_message(USART1_rx_data_buff);
				
				for(USART1_index=0;USART1_index<RX_BUFFER_SIZE0;USART1_index++)
															{
															USART1_rx_data_buff[USART1_index]=0;
															}  
															USART1_index=0;
			}
		//UART2 RX process	
			if(u2out == ONTIME)
			{
				u2out = STOP;// Da nhan du ban tin UART => Xy ly
				#ifdef _U1_DEBUG_ENABLE_
				//printf("UART2:%s\r\n",rx2_data_buff);
				#endif
				//u2_message_handle();
				prasing_message(rx2_data_buff);
				for(USART2_index=0;USART2_index<RX2_BUFFER_SIZE;USART2_index++)
															{
															rx2_data_buff[USART2_index]=0;
															}  
															USART2_index=0;
			}
}




