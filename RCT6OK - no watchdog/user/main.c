/*
NOTE: Chua chuyen UART ve 9600
12/Feb/2020: Decode message ok
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
//#define _U1_DEBUG_ENABLE_

time_t timenow = 1581501278;
volatile uint8_t sec_cnt = 0;
int8_t ledstt;
// lost signal : if after timeOutLostSignal seconds without GPS master message => lost
int8_t lostSignal = LOST_GPS_MASTER;
int8_t timeOutLostSignal = 30;

void factoryRST(void);





/**************************************************************************/
int main(void)
{	
	//int32_t ret = 0;
  //int i;
	//char x;
	
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
	w5500_lib_init();


	#ifdef _U1_DEBUG_ENABLE_
	printf("Run, now is %s\r\n",ctime(&timenow));
	#endif
	
	/* SNMP(Simple Network Management Protocol) Agent Initialize */
	// NMS (SNMP manager) IP address
	snmpd_init(managerIP,agentIP,SOCK_agent,SOCK_trap);	
	loadwebpages();
	
	// IWDG Initialization: STM32 Independent WatchDog
	//IWDG_Config();
	//timeinfo = localtime( &timenow );
	//printf("Current local time and date: %s\r\n", asctime(timeinfo));
	/*****************************MAIN WHILE************************************/
	while(1)
	{
		//IWDG_ReloadCounter(); // Feed IWDG
		/**********************************************************************/
		networkSevices();
		/**********************************************************************/
		scanOptos();
		
		/**********************************************************************/
		if(sec_cnt >= 1)
		{
			sec_cnt = 0;

			if(timeOutLostSignal) timeOutLostSignal--;
			if(timeOutLostSignal == 0) lostSignal = LOST_GPS_MASTER;
		}
		/**********************************************************************/
		//LED blinky : CPU run, neu ko nhay thi la co van de!  
		
		if((msec_cnt < 100) &&(ledstt != 1)) {ledstt = 1;ledON();}
		else if((msec_cnt >= 100) &&(ledstt != 0)) {ledstt = 0 ; ledOFF();}
		/**********************************************************************/
		//Xu ly thoi gian tu mach GPS master gui sang, neu ko nhan dc thi phai bao TIMEOUT
		usart1Process();
		
		

	}//end of main while

		
}//end of main




	
void networkSevices(void)
{
	//int32_t ret = 0;	
	checklink();//Kiem tra day mang con cam ko
	
	{	//SNMPv1 run
			//Run SNMP Agent Fucntion
			/* SNMP Agent Handler */
			//SMI Network Management Private Enterprise Codes: : moi cong ty phai dang ky 1 so rieng, 
			//tham khao : https://www.iana.org/assignments/enterprise-numbers/enterprise-numbers
			// Vi du Arduino : 36582
    	// SNMP Agent daemon process : User can add the OID and OID mapped functions to snmpData[] array in snmprun.c/.h
			// [net-snmp version 5.7 package for windows] is used for this demo.
			// * Command example
    	// [Command] Get:			  snmpget -v 1 -c public 192.168.22.165 .1.3.6.1.2.1.1.1.0 			// (sysDescr)
    	// [Command] Get: 			snmpget -v 1 -c public 192.168.22.165 .1.3.6.1.4.1.6.1.0 			// (Custom, get LED status)
    	// [Command] Get-Next: 	snmpwalk -v 1 -c public 192.168.22.165 .1.3.6.1
			// [Command] Set: 			snmpset -v 1 -c public 192.168.22.165 .1.3.6.1.4.1.6.1.1 i 1			// (Custom, LED 'On')
    	// [Command] Set: 			snmpset -v 1 -c public 192.168.22.165 .1.3.6.1.4.1.6.1.1 i 0			// (Custom, LED 'Off')
			snmpd_run();	
	}
	{	// web server 	
			httpServer_run(0);
			httpServer_run(1);
			httpServer_run(2);
		}
}
/**********************************************************************/
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
				
				prasing_message(rx2_data_buff);
				for(USART2_index=0;USART2_index<RX2_BUFFER_SIZE;USART2_index++)
															{
															rx2_data_buff[USART2_index]=0;
															}  
															USART2_index=0;
			}
}




