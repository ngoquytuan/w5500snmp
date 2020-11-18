
/********************************************************************************************
 * SNMP : User Customization Part
 *  - OID Registration
 *  - User defined functions for OID related
 *  	+ Integer value, String
 *  	+ I/O control / Chip registers
 *  	+ Network Informations
 *  	+ Etc.
 *
 *********************************************************************************************/
#include "stm32f10x.h"                  // Device header
#include "snmp_custom.h"
//Update gps and power status
#include "crc16CCITT.h"  
extern int8_t lostSignal;
int8_t dmeconn = 0;
int8_t vorconn = 0;
int8_t vorPmdt = 0;
int8_t dmeTtl = 0;
#include "ioconfig.h"

void setMyValue(int value)	//snmpset -v 1 -c public 192.168.1.246 .1.3.6.1.4.1.6.1.2 i 123456
{
	printf("Got my value :%d\r\n",value);
}
/*
void getMyValue()						//snmpget -v 1 -c public 192.168.1.246 .1.3.6.1.4.1.6.1.2
{
	printf("getMyValue\r\n");
	//return 301188;
}*/
void get_conn_stt(void *ptr, uint8_t *len);
void get_POWER_stt(void *ptr, uint8_t *len);

void get_conn_stt(void *ptr, uint8_t *len)//Kiem tra ket noi den VOR/DME
{
	*len = sprintf((char *)ptr, "VOR1150A: %s;DME1119A: %s;VOR PMDT: %s;DME1119: %s", vorconn?"Connected":"NC", dmeconn?"Connected":"NC", vorPmdt?"Connected":"NC", dmeTtl?"Connected":"NC");
}
/*
void get_POWER_stt(void *ptr, uint8_t *len)
{
	uint8_t gps1_stt;
	*len = sprintf((char *)ptr, "POWER1: [%s]; POWER2: [%s]", gps1_stt?"ON":"OFF", gps1_stt?"ON":"OFF");
}

void get_gpsmaster_stt(void *ptr, uint8_t *len)
{
	*len = sprintf((char *)ptr, "GPS master status: %s", lostSignal?"OK":"LOST SIGNAL");
}
*/
void getDevice(void *ptr, uint8_t *len)
{
//	uint8_t stt;
	//*len = sprintf((char *)ptr, "POWER1: [%s]; POWER2: [%s]", stt?"ON":"OFF", stt?"ON":"OFF");
	*len = sprintf((char *)ptr, "VOR => COM1, DME => COM2");
}
void DmeAlertLocal(void *ptr, uint8_t *len)
{
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	
	*len = sprintf((char *)ptr, "%s %s", dmestatus.MaintenanceAlert?"Alert":"", dmestatus.LocalControl?"Local":"");
}
void DmeMain(void *ptr, uint8_t *len)
{
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	
	*len = sprintf((char *)ptr, "Main %s", dmestatus.Tx1Main?"Tx1":"Tx2");
}
void DmeAntenna(void *ptr, uint8_t *len)
{
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	if((dmestatus.Tx1Ant == 1)&&(dmestatus.Tx1On == 1)) 					*len = sprintf((char *)ptr, "Antenna Tx1");
	else if((dmestatus.Tx1Ant == 0) &&(dmestatus.Tx2On == 1))			*len = sprintf((char *)ptr, "Antenna Tx2");
	else *len = sprintf((char *)ptr, "Antenna not connect");
}
void DmeLoad(void *ptr, uint8_t *len)
{
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	if((dmestatus.i9b5 == 1)&&(dmestatus.Tx2On == 1)) 						*len = sprintf((char *)ptr, "LOAD Tx2");
	else if((dmestatus.i9b5 == 0)&&(dmestatus.Tx1On == 1))				*len = sprintf((char *)ptr, "LOAD Tx1");
	else *len = sprintf((char *)ptr, "Load not connect");
}
void DmeOff(void *ptr, uint8_t *len)
{
	if((dmeconn ==0) && (dmeTtl == 0)) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	  if(dmeconn == 1)
		{
			if((dmestatus.Tx1On == 0)&&(dmestatus.Tx2On == 0)) 	*len = sprintf((char *)ptr, "OFF All");
			else if(dmestatus.Tx1On == 0)												*len = sprintf((char *)ptr, "OFF Tx1");		
			else if(dmestatus.Tx2On == 0)												*len = sprintf((char *)ptr, "OFF Tx2");
			else *len = sprintf((char *)ptr, "On All");
		}
		if(dmeTtl == 1)
		{
			if(dmestatus.Tx1On == 0) *len = sprintf((char *)ptr, "DME OFF");
			else *len = sprintf((char *)ptr, "DME ON");
		}
}
void DmeIntegral(void *ptr, uint8_t *len)
{
	if((dmeconn ==0) && (dmeTtl == 0)) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	
		if(dmeconn == 1)
		{
			if(dmestatus.Tx1On == 0) {*len = sprintf((char *)ptr, " ");return;}
			if(dmestatus.IntPriAlarm == 1) *len = sprintf((char *)ptr, "Integral PriAlarm, %s, %s",dmestatus.i11b0?"SecAlarm":"",dmestatus.IntMonBypass?"Bypass":"");
		  else *len = sprintf((char *)ptr, "Integral Normal, %s, %s",dmestatus.i11b0?"SecAlarm":"",dmestatus.IntMonBypass?"Bypass":"");
		}
		if(dmeTtl == 1)
		{
			if(dmestatus.Tx1On == 0) {*len = sprintf((char *)ptr, " ");return;}
			//if(dmestatus.IntPriAlarm == 1) *len = sprintf((char *)ptr, "Alarm, %s",dmestatus.IntMonBypass?"Bypass":"");
			//else                           *len = sprintf((char *)ptr, "Normal, %s",dmestatus.IntMonBypass?"Bypass":"");
			
			if(LGB3Actived == 0) 
			{
				//printf("Alarm\r\n");//DB9 -3
				*len = sprintf((char *)ptr, "Alarm, %s",dmestatus.IntMonBypass?"Bypass":"");
			}
			else *len = sprintf((char *)ptr, "Normal, %s",dmestatus.IntMonBypass?"Bypass":"");
		}
		
}
void DmeStandby(void *ptr, uint8_t *len)
{
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	
		
		if(dmestatus.i10b3 == 1) *len = sprintf((char *)ptr, "Standby PriAlarm, %s, %s",dmestatus.i11b1?"SecAlarm":"",dmestatus.i10b5?"Bypass":"");
		else *len = sprintf((char *)ptr, "Standby Normal, %s, %s",dmestatus.i11b1?"SecAlarm":"",dmestatus.i10b5?"Bypass":"");
		
}
void DmeDelayUs(void *ptr, uint8_t *len)
{
	float value;
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	value = dmestatus.DelayUs/100.0;
	*len = sprintf((char *)ptr, "Delay: %.2f us", value);
}

void dmeSpacing(void *ptr, uint8_t *len)
{
	float value;
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	value = dmestatus.SpacingUs/100.0;
	*len = sprintf((char *)ptr, "Spacing: %.2f us", value);
}

void dmeTxPower(void *ptr, uint8_t *len)
{
	float value;
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	value = dmestatus.TxPowerWatts/1.0;
	*len = sprintf((char *)ptr, "TxPower: %.0f Watts", value);
}

void dmeERP(void *ptr, uint8_t *len)
{
	float value;
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	value = dmestatus.ERP/10.0;
	*len = sprintf((char *)ptr, "ERP: %.1f dB", value);
}
void dmeEfficiency(void *ptr, uint8_t *len)
{
	float value;
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	value = dmestatus.Efficiency/10.0;
	*len = sprintf((char *)ptr, "Efficiency: %.1f %%", value);
}
void dmePRF(void *ptr, uint8_t *len)
{
	float value;
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}
	value = dmestatus.PRFppps/1.0;
	*len = sprintf((char *)ptr, "PRF: %.0f ppps", value);
}

void dmeInfo(void *ptr, uint8_t *len)
{
	if(dmeconn ==0) //ko co ket noi den DME
		{
			*len = sprintf((char *)ptr, "No connection to DME");
			return;
		}	
	*len = sprintf((char *)ptr, "Dme info %2d/%2d/%2d %2d:%2d:%2d", dmestatus.month,dmestatus.date,dmestatus.year,dmestatus.hour,dmestatus.min,dmestatus.sec);
}
/***************************************************************VOR***********************************************************************************************/
//vorPmdt = 1
void vorAlertLocal(void *ptr, uint8_t *len)
{
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
		if		 (vorconn == 1) *len = sprintf((char *)ptr, "%s %s", vorstatus.Alert?"Alert":"", vorstatus.LocalControl?"Local":"");
		else if(vorPmdt == 1) *len = sprintf((char *)ptr, "%s %s", vorstatus.Alert?"Alert":"", vorstatus.LocalControl?"Local":"");
}
void vorMain(void *ptr, uint8_t *len)
{
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
	
	*len = sprintf((char *)ptr, "Main %s", vorstatus.Tx1Main?"Tx1":"Tx2");
}

void vorAntenna(void *ptr, uint8_t *len)
{
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
	if((vorstatus.Tx1Ant == 1)&&(vorstatus.Tx1On == 1)) 					*len = sprintf((char *)ptr, "Antenna Tx1");
	else if((vorstatus.Tx1Ant == 0) &&(vorstatus.Tx2On == 1))			*len = sprintf((char *)ptr, "Antenna Tx2");
	else *len = sprintf((char *)ptr, "Antenna not connect");
}
void vorLoad(void *ptr, uint8_t *len)
{
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
		
	if     ((vorstatus.i9b5 == 1)&&(vorstatus.Tx2On == 1)&&(vorconn == 1)) 						*len = sprintf((char *)ptr, "LOAD Tx2");
	else if((vorstatus.i9b5 == 0)&&(vorstatus.Tx1On == 1)&&(vorconn == 1))				    *len = sprintf((char *)ptr, "LOAD Tx1");
	else if((vorstatus.i10b2 == 1)&&(vorstatus.Tx2On == 1)&&(vorPmdt == 1)) 					*len = sprintf((char *)ptr, "LOAD Tx2");
	else if((vorstatus.i10b2 == 1)&&(vorstatus.Tx1On == 1)&&(vorPmdt == 1))				    *len = sprintf((char *)ptr, "LOAD Tx1");
	else *len = sprintf((char *)ptr, "Load not connect");
}
void vorOff(void *ptr, uint8_t *len)
{
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
	
		if((vorstatus.Tx1On == 0)&&(vorstatus.Tx2On == 0)) 	*len = sprintf((char *)ptr, "OFF All");
		else if(vorstatus.Tx1On == 0)												*len = sprintf((char *)ptr, "OFF Tx1");		
		else if(vorstatus.Tx2On == 0)												*len = sprintf((char *)ptr, "OFF Tx2");
		else *len = sprintf((char *)ptr, "On All");
}
void vorIntegral(void *ptr, uint8_t *len)
{
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
	
		
		
		if(vorconn == 1)
			{
				//if(vorstatus.IntPriAlarm == 1) *len = sprintf((char *)ptr, "Integral PriAlarm, %s, %s","SecAlarm ??",vorstatus.IntMonBypass?"Bypass":"");
				//else *len = sprintf((char *)ptr, "Integral Normal, %s, %s","SecAlarm ??",vorstatus.IntMonBypass?"Bypass":"");
				//Chua lam ro dc SecAlarm
				if(vorstatus.IntPriAlarm == 1) *len = sprintf((char *)ptr, "Integral PriAlarm, %s",vorstatus.IntMonBypass?"Bypass":"");
				else *len = sprintf((char *)ptr, "Integral Normal, %s",vorstatus.IntMonBypass?"Bypass":"");
			}
		if(vorPmdt == 1)
			{
				if(vorstatus.i10b4 == 1) *len = sprintf((char *)ptr, "Alarm, %s",vorstatus.Mon1Bypass?"Bypass":"");
				else *len = sprintf((char *)ptr, "Normal, %s",vorstatus.Mon1Bypass?"Bypass":"");
			}
		
}

void vorAzimuthAngle(void *ptr, uint8_t *len)
{
	float value;
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
	value = vorstatus.AzimuthAngle/100.0;
	*len = sprintf((char *)ptr, "Azimuth: %.2f degrees", value);
}

void vorMod30Hz(void *ptr, uint8_t *len)
{
	float value;
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
	value = vorstatus.Mod30Hz/10.0;
	*len = sprintf((char *)ptr, "30Hz Modulation: %.1f %%", value);
}

void vorMod9960(void *ptr, uint8_t *len)
{
	float value;
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
	value = vorstatus.Mod9960Hz/10.0;
	*len = sprintf((char *)ptr, "9960Hz Modulation: %.1f %%", value);
}

void vorDeviation(void *ptr, uint8_t *len)
{
	float value;
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
	value = vorstatus.Deviation/10.0;
	*len = sprintf((char *)ptr, "9960Hz Deviation: %.2f ratio", value);
}

void vorRFlevel(void *ptr, uint8_t *len)
{
	float value;
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}
	value = vorstatus.RFlevel/10.0;
	*len = sprintf((char *)ptr, "RF level: %.1f dB", value);
}
void vorInfo(void *ptr, uint8_t *len)
{
	if((vorconn == 0) && (vorPmdt == 0)) //ko co ket noi den Vor
		{
			*len = sprintf((char *)ptr, "No connection to Vor");
			return;
		}	
	*len = sprintf((char *)ptr, "Vor info %2d/%2d/%2d %2d:%2d:%2d", vorstatus.month,vorstatus.date,vorstatus.year,vorstatus.hour,vorstatus.min,vorstatus.sec);
}
dataEntryType snmpData[] =
{
    // System MIB
	// SysDescr Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 1, 0},
	SNMPDTYPE_OCTET_STRING, 30, {"VOR, DME SNMPv1"},
	NULL, NULL},

	// SysObjectID Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 2, 0},
	SNMPDTYPE_OBJ_ID, 8, {"\x2b\x06\x01\x02\x01\x01\x02\x00"},
	NULL, NULL},

	// SysUptime Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 3, 0},
	SNMPDTYPE_TIME_TICKS, 0, {""},
	currentUptime, NULL},

	// sysContact Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 4, 0},
	SNMPDTYPE_OCTET_STRING, 30, {"https://attech.com.vn/lien-he-voi-chung-toi/"},
	NULL, NULL},

	// sysName Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 5, 0},
	SNMPDTYPE_OCTET_STRING, 30, {"https://attech.com.vn/"},
	NULL, NULL},

	// Location Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 6, 0},
	SNMPDTYPE_OCTET_STRING, 30, {"NCPT"},
	NULL, NULL},
	// SysServices
	{8, {0x2b, 6, 1, 2, 1, 1, 7, 0},
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, NULL},
  /*{8, {0x2b, 6, 1, 4, 1, 26, 1, 0},//Kiem tra ket noi den COM1,COM2
	SNMPDTYPE_OCTET_STRING, 40, {""},
	get_conn_stt, NULL},
	{8, {0x2b, 6, 1, 4, 1, 27, 1, 0},//Thong tin cau hinh hoat dong
	SNMPDTYPE_OCTET_STRING, 40, {"devide info"},
	NULL, NULL},
  {8, {0x2b, 6, 1, 4, 1, 27, 1, 1},//Trang thai hoat dong cua thiet bi
	SNMPDTYPE_OCTET_STRING, 40, {"ok"},
	NULL, NULL},*/
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 0},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	dmeInfo, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 1},//DmeAlertLocal
	SNMPDTYPE_OCTET_STRING, 40, {""},
	DmeAlertLocal, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 2},//DmeMain
	SNMPDTYPE_OCTET_STRING, 40, {""},
	DmeMain, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 3},//DmeAntenna
	SNMPDTYPE_OCTET_STRING, 40, {""},
	DmeAntenna, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 4},//DmeLoad
	SNMPDTYPE_OCTET_STRING, 40, {""},
	DmeLoad, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 5},//DmeOff
	SNMPDTYPE_OCTET_STRING, 40, {""},
	DmeOff, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 6},//DmeIntegral
	SNMPDTYPE_OCTET_STRING, 40, {""},
	DmeIntegral, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 7},//DmeStandby
	SNMPDTYPE_OCTET_STRING, 40, {""},
	DmeStandby, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 8},//Delay
	SNMPDTYPE_OCTET_STRING, 40, {""},
	DmeDelayUs, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 9},//Spacing
	SNMPDTYPE_OCTET_STRING, 40, {""},
	dmeSpacing, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 10},//TxPower
	SNMPDTYPE_OCTET_STRING, 40, {""},
	dmeTxPower, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 11},//ERP
	SNMPDTYPE_OCTET_STRING, 40, {""},
	dmeERP, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 12},//Efficiency
	SNMPDTYPE_OCTET_STRING, 40, {""},
	dmeEfficiency, NULL},
	{8, {0x2b, 6, 1, 4, 1, 28, 1, 13},//PRF
	SNMPDTYPE_OCTET_STRING, 40, {""},
	dmePRF, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 0},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorInfo, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 1},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorAlertLocal, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 2},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorMain, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 3},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorAntenna, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 4},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorLoad, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 5},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorOff, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 6},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorIntegral, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 7},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorAzimuthAngle, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 8},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorMod30Hz, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 9},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorMod9960, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 10},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorDeviation, NULL},
	{8, {0x2b, 6, 1, 4, 1, 29, 1, 11},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	vorRFlevel, NULL},
	
#ifdef _USE_WIZNET_W5500_EVB_
	// Get the WIZnet W5500-EVB LED Status
	{8, {0x2b, 6, 1, 4, 1, 6, 1, 0},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	get_gpsStatus_all, NULL},
	// Get the my Status
	//{0x0a, {0x2b, 0x06, 0x01, 0x04, 0x01, 0x85, 0xb6, 0x38, 0x01, 0x00},
	//SNMPDTYPE_OCTET_STRING, 40, {""},
	//get_gpsStatus_all, NULL},
	// Set the LED_R (RGB LED)
	{8, {0x2b, 6, 1, 4, 1, 6, 1, 1},
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, set_LEDStatus_R},
	// Set the my value 
	{8, {0x2b, 6, 1, 4, 1, 6, 1, 2},
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, setMyValue},
	
	// Get the WIZnet W5500-EVB LED Status
	{8, {0x2b, 6, 1, 4, 1, 6, 1, 0},
	SNMPDTYPE_OCTET_STRING, 40, {""},
	get_gpsStatus_all, NULL},

	// Set the LED_R (RGB LED)
	{8, {0x2b, 6, 1, 4, 1, 6, 1, 1},
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, set_LEDStatus_R},

	// Set the LED_G (RGB LED)
	{8, {0x2b, 6, 1, 4, 1, 6, 1, 2},
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, set_LEDStatus_G},

	// Set the LED_B (RGB LED)
	{8, {0x2b, 6, 1, 4, 1, 6, 1, 3},
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, set_LEDStatus_B},
		// OID Test #1 (long-length OID example, 19865)
	{0x0a, {0x2b, 0x06, 0x01, 0x04, 0x01, 0x81, 0x9b, 0x19, 0x01, 0x00},
	SNMPDTYPE_OCTET_STRING, 30, {"long-length OID Test #1 tuannq"},
	NULL, NULL},
	
	// OID Test #1 (long-length OID example, 19865)
	{0x0a, {0x2b, 0x06, 0x01, 0x04, 0x01, 0x85, 0xb6, 0x38, 0x01, 0x00},
	SNMPDTYPE_OCTET_STRING, 30, {"long-length OID Test #11 tuannq"},
	NULL, NULL},

	// OID Test #2 (long-length OID example, 22210)
	{0x0a, {0x2b, 0x06, 0x01, 0x04, 0x01, 0x81, 0xad, 0x42, 0x01, 0x00},
	SNMPDTYPE_OCTET_STRING, 35, {"long-length OID Test #2"},
	NULL, NULL},
	// OID Test #2: SysObjectID Entry
	{0x0a, {0x2b, 0x06, 0x01, 0x04, 0x01, 0x81, 0xad, 0x42, 0x02, 0x00},
	SNMPDTYPE_OBJ_ID, 0x0a, {"\x2b\x06\x01\x04\x01\x81\xad\x42\x02\x00"},
	NULL, NULL},
#endif



	
	
};

const int32_t maxData = (sizeof(snmpData) / sizeof(dataEntryType));

void initTable()
{
	// Example integer value for [OID 1.3.6.1.2.1.1.7.0]
	/*
	sysServices OBJECT-TYPE
	SYNTAX INTEGER (0..127)
	MAX-ACCESS read-only
	STATUS current
	DESCRIPTION
	"A value which indicates the set of services that this
	entity may potentially offer. The value is a sum.
	This sum initially takes the value zero. Then, for
	each layer, L, in the range 1 through 7, that this node
	performs transactions for, 2 raised to (L - 1) is added
	to the sum. For example, a node which performs only
	routing functions would have a value of 4 (2^(3-1)).
	In contrast, a node which is a host offering application
	services would have a value of 72 (2^(4-1) + 2^(7-1)).
	Note that in the context of the Internet suite of
	protocols, values should be calculated accordingly:
	layer functionality
	1 physical (e.g., repeaters)
	2 datalink/subnetwork (e.g., bridges)
	3 internet (e.g., supports the IP)
	4 end-to-end (e.g., supports the TCP)
	7 applications (e.g., supports the SMTP)
	For systems including OSI protocols, layers 5 and 6
	may also be counted."
	*/
	snmpData[6].u.intval = 72;
	//snmpData[14].u.intval = 72;
}

void get_gpsStatus_all(void *ptr, uint8_t *len)
{
	uint8_t led_status[3] = {0, };

	led_status[LED_R] = 1;
	led_status[LED_G] = 0;
	led_status[LED_B] = 1;

	*len = sprintf((char *)ptr, "GPS 1 [%s] / 2 [%s] ", led_status[LED_R]?"On":"Off", led_status[LED_G]?"On":"Off");
}

void Board_LED_Set(int led, int i)
{
	printf("Board_LED_Set\r\n");
}
void set_LEDStatus_R(int32_t val)
{
	printf("set_LEDStatus_R :%d\r\n",val);
	if(val == 0) 	Board_LED_Set(LED_R, 0);
	else 			Board_LED_Set(LED_R, 1);
}

// W5500-EVB: LED Control ///////////////////////////////////////////////////////////////////////////
#ifdef _USE_WIZNET_W5500_EVB_
void get_gpsStatus_all(void *ptr, uint8_t *len)
{
	uint8_t led_status[3] = {0, };

	//led_status[LED_R] = (uint8_t)Board_LED_Test(LED_R);
	//led_status[LED_G] = (uint8_t)Board_LED_Test(LED_G);
	//led_status[LED_B] = (uint8_t)Board_LED_Test(LED_B);
	led_status[LED_R] = 1;
	led_status[LED_G] = 0;
	led_status[LED_B] = 1;

	*len = sprintf((char *)ptr, "LED R [%s] / G [%s] / B [%s]", led_status[LED_R]?"On":"Off", led_status[LED_G]?"On":"Off", led_status[LED_B]?"On":"Off");
}

void set_LEDStatus_R(int32_t val)
{
	printf("set_LEDStatus_R :%d\r\n",val);
	if(val == 0) 	Board_LED_Set(LED_R, 0);
	else 			Board_LED_Set(LED_R, 1);
}

void set_LEDStatus_G(int32_t val)
{
	printf("set_LEDStatus_G\r\n");
	if(val == 0) 	Board_LED_Set(LED_G, 0);
	else 			Board_LED_Set(LED_G, 1);
}

void set_LEDStatus_B(int32_t val)
{
	printf("set_LEDStatus_B\r\n");
	if(val == 0) 	Board_LED_Set(LED_B, 0);
	else 			Board_LED_Set(LED_B, 1);
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////

void initial_Trap(uint8_t * managerIP, uint8_t * agentIP)
{
	// SNMP Trap: WarmStart(1) Trap
	{
		dataEntryType enterprise_oid = {0x0a, {0x2b, 0x06, 0x01, 0x04, 0x01, 0x81, 0x9b, 0x19, 0x01, 0x00},
												SNMPDTYPE_OBJ_ID, 0x0a, {"\x2b\x06\x01\x04\x01\x81\x9b\x19\x10\x00"},	NULL, NULL};
		// Generic Trap: warmStart COMMUNITY
		snmp_sendTrap(managerIP, agentIP, (void *)COMMUNITY, enterprise_oid, SNMPTRAP_WARMSTART, 0, 0);
	}

}
