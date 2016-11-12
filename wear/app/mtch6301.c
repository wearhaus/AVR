/*
 * MTCH6301.c
 *
 * Created: 2014/7/12 18:55:10
 *  Author: Harifun
 */ 

#include "asf.h"
#include <mtch6301.h>
#include <util/delay.h>
#include <config_app.h>
#include <wear.h>

volatile uint8_t cmd_status = 0;
volatile uint8_t mapio_mtch6301 = 0;
touch_package_t touch_pkg;
touch_package_t * const m_touch_pkg = &touch_pkg;
uint8_t response_slaver[9]={0};
volatile uint8_t cmd_res = 0x01;	
status_code_t status_res = ERR_UNSUPPORTED_DEV;

volatile uint8_t latest_gesture = 0x00;

void cmd_read_register(uint8_t index, uint8_t offset);

void init_i2c(void)
{
	// TWI master options
	twi_options_t m_options = {
		.speed = TWI_SPEED,
		.chip  = TWI_MASTER_ADDR,
		.speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI_SPEED)
	};
	
	sysclk_enable_peripheral_clock(&TWI_MASTER);
	twi_master_init(&TWI_MASTER,&m_options);
	twi_master_enable(&TWI_MASTER);
	
	cpu_irq_enable();
}

static inline void wait_result(void)
{
      unsigned int count=0;
	while(cmd_res == 0x01)
	{
#ifdef LIMIT_LOOP
           if(count++>10000)
		   	break;
#endif
	    ;
	}
	
	cmd_res = 0x01;
}	

bool write_mtch_settings(void) {
	status_code_t write_status = STATUS_OK;
	if (!EEPROM_MTCH_UPDATE_BOOL) {
		return true;
	}
	#ifdef ENABLE_WDT
		wdt_kick();
	#endif
	
	twinkle(255, 0, 0);
	
	_delay_ms(16000);
	
	//turn off touch and save to NVRAM first
	cmd_mtch6301(0x01);
	_delay_ms(16000);
	cmd_mtch6301(0x17);
	_delay_ms(16000);

	//GENERAL - INDEX 0x00
	if (EEPROM_GENERAL_UPDATE_BOOL) {
		twinkle(255, 255, 0);
		write_status =  STATUS_OK;
		 write_status |= cmd_write_register(0x00, 0x01, EEPROM_NUMRXCHANNELS); //NUM RX CHANNELS
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x00, 0x02, EEPROM_NUMTXCHANNELS); //NUM TX CHANNELS
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x00, 0x04, EEPROM_RXSCALE_7_0); //RX SCALING [7:0]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x00, 0x05, EEPROM_RXSCALE_15_8); //RX SCALING [15:8]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x00, 0x06, EEPROM_TXSCALE_7_0); //TX SCALING [7:0]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x00, 0x07, EEPROM_TXSCALE_15_8); //TX SCALING [15:8]
		_delay_ms(8000);
		
		write_status |= cmd_write_register(0x00, 0x00, 0x00); //flag1
		_delay_ms(8000);
		write_status |= cmd_write_register(0x00, 0x03, 0xd0); //customFlag
		_delay_ms(8000);
		write_status |= cmd_write_register(0x00, 0x08, 0x00); //rxDiagChannel
		_delay_ms(8000);
		write_status |= cmd_write_register(0x00, 0x09, 0x00); //txDiagChannel
		_delay_ms(8000);
		/*
		write_status |= cmd_write_register(0x00, 0x0a, 0xfa); //baseUpdateTime
		_delay_ms(8000);
		write_status |= cmd_write_register(0x00, 0x0b, 0x32); //stuckThreshold
		_delay_ms(8000);
		write_status |= cmd_write_register(0x00, 0x0c, 0x70); //stuckTimeout [7:0]
		_delay_ms(8000);
		write_status |= cmd_write_register(0x00, 0x0d, 0x17); //stuckTimeout [15:8]
		_delay_ms(8000);
		*/
		cmd_mtch6301(0x17);
		_delay_ms(16000);
		nvm_eeprom_write_byte(EEPROM_INDEX_GENERAL, STATUS_OK != write_status);
	}
	
	
	//RXMAP - INDEX 0x01
	if (EEPROM_RXMAP_UPDATE_BOOL) {
		twinkle(0, 255, 0);
		write_status =  STATUS_OK;
		 write_status |= cmd_write_register(0x01, 0x00, EEPROM_RXPIN0);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x01, EEPROM_RXPIN1);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x02, EEPROM_RXPIN2);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x03, EEPROM_RXPIN3);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x04, EEPROM_RXPIN4);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x05, EEPROM_RXPIN5);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x06, EEPROM_RXPIN6);
		_delay_ms(8000);
		/*
		 write_status |= cmd_write_register(0x01, 0x07, EEPROM_RXPIN7);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x08, EEPROM_RXPIN8);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x09, EEPROM_RXPIN9);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x0a, EEPROM_RXPIN10);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x0b, EEPROM_RXPIN11);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x01, 0x0c, EEPROM_RXPIN12);
		_delay_ms(8000);
		*/
		cmd_mtch6301(0x17);
		_delay_ms(16000);
		nvm_eeprom_write_byte(EEPROM_INDEX_RXMAP, STATUS_OK != write_status);
	}
	
	
	//TXMAP - INDEX 0x02
	if (EEPROM_TXMAP_UPDATE_BOOL) {
		twinkle(0, 255, 255);
		write_status =  STATUS_OK;
		 write_status |= cmd_write_register(0x02, 0x00, EEPROM_TXPIN0);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x01, EEPROM_TXPIN1);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x02, EEPROM_TXPIN2);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x03, EEPROM_TXPIN3);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x04, EEPROM_TXPIN4);
		_delay_ms(8000);
		/*
		 write_status |= cmd_write_register(0x02, 0x05, EEPROM_TXPIN5);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x06, EEPROM_TXPIN6);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x07, EEPROM_TXPIN7);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x08, EEPROM_TXPIN8);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x09, EEPROM_TXPIN9);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x0a, EEPROM_TXPIN10);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x0b, EEPROM_TXPIN11);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x0c, EEPROM_TXPIN12);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x0d, EEPROM_TXPIN13);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x0e, EEPROM_TXPIN14);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x0f, EEPROM_TXPIN15);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x10, EEPROM_TXPIN16);
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x02, 0x11, EEPROM_TXPIN17);
		_delay_ms(8000);
		*/
		cmd_mtch6301(0x17);
		_delay_ms(16000);
		nvm_eeprom_write_byte(EEPROM_INDEX_TXMAP, STATUS_OK != write_status);
	}
	
	
	//SELF - INDEX 0x10
	if (EEPROM_SELF_UPDATE_BOOL) {
		twinkle(0, 0, 255);
		write_status =  STATUS_OK;
		 write_status |= cmd_write_register(0x10, 0x00, EEPROM_SELFSCANTIME); //SELF SCAN TIME
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x10, 0x01, EEPROM_SELFTOUCHTHRES); //SELF THRESHOLD
		_delay_ms(8000);
		
		write_status |= cmd_write_register(0x10, 0x02, 0x01); //selfSampleFreq
		_delay_ms(8000);
		write_status |= cmd_write_register(0x10, 0x03, 0x01); //stutterMult
		_delay_ms(8000);
		//cmd_write_register(0x11, 0x00, 0x00); //selfscanfinetune...
		//_delay_ms(8000);
		
		
		cmd_mtch6301(0x17);
		_delay_ms(16000);
		nvm_eeprom_write_byte(EEPROM_INDEX_SELF, STATUS_OK != write_status);
	}
	
	
	//MUTUAL - INDEX 0x20
	if (EEPROM_MUTUAL_UPDATE_BOOL) {
		twinkle(255, 0, 255);
		write_status =  STATUS_OK;
		 write_status |= cmd_write_register(0x20, 0x00, EEPROM_MUTSCANTIME); //MUTUAL SCAN TIME
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x20, 0x01, EEPROM_MUTTOUCHTHRES); //MUTUAL THRESHOLD
		_delay_ms(8000);
		
		 write_status |= cmd_write_register(0x20, 0x02, 0x16); //mutSampleFreq
		_delay_ms(8000);
		//cmd_write_register(0x21, 0x00, 0x00); //mutscanfinetune...
		//_delay_ms(8000);
		
		write_status |= cmd_write_register(0x30, 0x02, 0x05); //minCuspDelta
		_delay_ms(8000);
		write_status |= cmd_write_register(0x30, 0x03, 0xff); //weightThreshold
		_delay_ms(8000);
		write_status |= cmd_write_register(0x30, 0x09, 0x96); //largeActThres
		_delay_ms(8000);
		
		cmd_mtch6301(0x17);
		_delay_ms(16000);
		nvm_eeprom_write_byte(EEPROM_INDEX_MUTUAL, STATUS_OK != write_status);
	}
	
	
	//DECODING - INDEX 0x30
	if (EEPROM_DECODING_UPDATE_BOOL) {
		twinkle(0, 0, 255);
		write_status =  STATUS_OK;
		 write_status |= cmd_write_register(0x30, 0x00, EEPROM_FLIPSTATE); //FLIP STATE
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x30, 0x01, EEPROM_NUMAVG); //NUM AVERAGES
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x30, 0x04, EEPROM_MINTOUCHDIST); //MIN TOUCH DISTANCE
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x30, 0x05, EEPROM_PENDOWNTIME); //PEN DOWN TIME
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x30, 0x06, EEPROM_PENUPTIME); //PEN UP TIME
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x30, 0x07, EEPROM_TOUCHSUPPRESSION); //TOUCH SUPPRESSION (MAX TOUCH POINTS)
		_delay_ms(8000);
		cmd_mtch6301(0x17);
		_delay_ms(16000);
		nvm_eeprom_write_byte(EEPROM_INDEX_DECODING, STATUS_OK != write_status);
	}
	
	
	//GESTURES - INDEX 0x50
	if (EEPROM_GESTURES_UPDATE_BOOL) {
		twinkle(255, 0, 255);
		write_status =  STATUS_OK;
		 write_status |= cmd_write_register(0x50, 0x00, EEPROM_RXSWIPELEN); //RX SWIPE LENGTH
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x01, EEPROM_TXSWIPELEN); //TX SWIPE LENGTH
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x02, EEPROM_SWIPEBOUNDARY); //SWIPE BOUNDARY
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x03, EEPROM_SWIPEHOLDTHRES); //SWIPE HOLD THRESHOLD
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x04, EEPROM_SWIPETIME_7_0); //SWIPE TIME [7:0]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x05, EEPROM_SWIPETIME_15_8); //SWIPE TIME [15:8]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x06, EEPROM_TAPTIME_7_0); //TAP TIME [7:0]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x07, EEPROM_TAPTIME_15_8); //TAP TIME [15:8]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x08, EEPROM_TAPTHRES); //TAP THRESHOLD
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x09, EEPROM_MINSWIPEVELOCITY); //MIN SWIPE VELOCITY
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x0a, EEPROM_DOUBLETIME_7_0); //DOUBLE TAP TIME [7:0]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x0b, EEPROM_DOUBLETIME_15_8); //DOUBLE TAP [15:8]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0x50, 0x0c, EEPROM_EDGEKEEPOUT); //GESTURE EDGE KEEPOUT
		_delay_ms(8000);
		cmd_mtch6301(0x17);
		_delay_ms(16000);
		nvm_eeprom_write_byte(EEPROM_INDEX_GESTURES, STATUS_OK != write_status);
	}
	
	
	//CONFIG - INDEX 0xf0
	if (EEPROM_CONFIG_UPDATE_BOOL) {
		twinkle(255, 255, 255);
		write_status =  STATUS_OK;
		/*
		 write_status |= cmd_write_register(0xf0, 0x00, EEPROM_SLEEP_7_0); //SLEEP TIMEOUT [7:0]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0xf0, 0x01, EEPROM_SLEEP_15_8); //SLEEP TIMEOUT [15:8]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0xf0, 0x02, EEPROM_SLEEP_23_16); //SLEEP TIMEOUT [23:16]
		_delay_ms(8000);
		 write_status |= cmd_write_register(0xf0, 0x03, EEPROM_SLEEP_31_24); //SLEEP TIMEOUT [31:24]
		_delay_ms(8000);
		*/
		 write_status |= cmd_write_register(0xf0, 0x07, EEPROM_TOUCHPACKETCFG); //TOUCH PACKET CFG
		_delay_ms(8000);
		 write_status |= cmd_write_register(0xf0, 0x09, EEPROM_GESTUREPACKETCFG); //GESTURE PACKET CFG
		_delay_ms(8000);
		 write_status |= cmd_write_register(0xf0, 0x0a, EEPROM_STATUSPACKETCFG); //STATUS PACKET CFG
		_delay_ms(8000);

		/*
		cmd_write_register(0xf0, 0x04, 0x02); //sleepConfig
		_delay_ms(8000);
		cmd_write_register(0xf0, 0x05, 0x07); //wdtTimeout
		_delay_ms(8000);
		cmd_write_register(0xf0, 0x06, 0x81); //diagPacketConfig
		_delay_ms(8000);
		cmd_write_register(0xf0, 0x08, 0x81); //commandPacketConfig
		_delay_ms(8000);
		*/
				
		write_status |= cmd_write_register(0x30, 0x00, EEPROM_FLIPSTATE); //FLIP STATE
		_delay_ms(8000);
		
		cmd_mtch6301(0x17);
		_delay_ms(16000);
		nvm_eeprom_write_byte(EEPROM_INDEX_CONFIG, STATUS_OK != write_status);
	}
	/*
	cmd_write_register(0x40, 0x00, 0x12); //cpTimeOut
	_delay_ms(8000);
	cmd_write_register(0x40, 0x01, 0x19); //selfNoiseTresh
	_delay_ms(8000);
	cmd_write_register(0x40, 0x02, 0x17); //mutNoiseThresh
	_delay_ms(8000);
	cmd_write_register(0x40, 0x03, 0x14); //frequencyChanges
	_delay_ms(8000);
	cmd_write_register(0x40, 0x04, 0x05); //sampleSize
	_delay_ms(8000);
	cmd_write_register(0x40, 0x05, 0x04); //selfNoiseScanTime
	_delay_ms(8000);
	cmd_write_register(0x40, 0x06, 0x07); //mutNoiseScanTime
	_delay_ms(8000);
	cmd_write_register(0x40, 0x07, 0x02); //filterCoeff
	_delay_ms(8000);
	*/
	
	//Restore touch functionality and save to NVRAM
	cmd_mtch6301(0x00);
	_delay_ms(16000);
	cmd_mtch6301(0x17);
	_delay_ms(16000);
	
	nvm_eeprom_write_byte(EEPROM_INDEX_MTCH, (
		EEPROM_GENERAL_UPDATE_BOOL	|
		EEPROM_GENERAL_UPDATE_BOOL	|
		EEPROM_RXMAP_UPDATE_BOOL	|
		EEPROM_TXMAP_UPDATE_BOOL	|
		EEPROM_SELF_UPDATE_BOOL		|
		EEPROM_MUTUAL_UPDATE_BOOL	|
		EEPROM_DECODING_UPDATE_BOOL	|
		EEPROM_GESTURES_UPDATE_BOOL	|
		EEPROM_CONFIG_UPDATE_BOOL	
	));
	
	if (EEPROM_MTCH_UPDATE_BOOL) {
		twinkle(255, 0, 0);
		return false;
	}
	
	else {
		rainbow();
		return true;
	}
}
	
void init_mtch6301(void)
{
	/*
	iniatize the io interrupt pin pc5
	pc7 作为复位管脚，初始化为输出高电平
	*/
//	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 7), IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
		
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 5), IOPORT_DIR_INPUT | IOPORT_RISING | IOPORT_PULL_UP);
	PORTCFG.MPCMASK = 0x20;
	PORTC.INTMASK = 0x20;
	PORTC.INTCTRL = PMIC_LOLVLEN_bm;
	pmic_enable_level(PMIC_LOLVLEN_bm);
	cpu_irq_enable();
	/*
	_delay_ms(500);
	
	cmd_mtch6301(0x00);
	_delay_ms(250);
	cmd_mtch6301(0x17);
	_delay_ms(250);

	//GENERAL - INDEX 0x00
	cmd_write_register(0x00, 0x01, 0x07);	//NUM RX CHANNELS
	_delay_ms(250);
	cmd_write_register(0x00, 0x02, 0x05); //NUM TX CHANNELS
	_delay_ms(250);
	cmd_write_register(0x00, 0x04, 0x92); //RX SCALING [7:0]
	_delay_ms(250);
	cmd_write_register(0x00, 0x05, 0x24); //RX SCALING [15:8]
	_delay_ms(250);
	cmd_write_register(0x00, 0x06, 0x33); //TX SCALING [7:0]
	_delay_ms(250);
	cmd_write_register(0x00, 0x07, 0x33); //TX SCALING [15:8]
	_delay_ms(250);
	
	
	//RXMAP - INDEX 0x01
	cmd_write_register(0x01, 0x00, 0x05);
	_delay_ms(250);
	cmd_write_register(0x01, 0x01, 0x06);
	_delay_ms(250);
	cmd_write_register(0x01, 0x02, 0x07);
	_delay_ms(250);
	cmd_write_register(0x01, 0x03, 0x01);
	_delay_ms(250);
	cmd_write_register(0x01, 0x04, 0x04);
	_delay_ms(250);
	cmd_write_register(0x01, 0x05, 0x03);
	_delay_ms(250);
	cmd_write_register(0x01, 0x06, 0x02);
	_delay_ms(250);
	cmd_write_register(0x01, 0x07, 0x01);
	_delay_ms(250);
	cmd_write_register(0x01, 0x08, 0x00);
	_delay_ms(250);
	cmd_write_register(0x01, 0x09, 0x09);
	_delay_ms(250);
	cmd_write_register(0x01, 0x0a, 0x0a);
	_delay_ms(250);
	cmd_write_register(0x01, 0x0b, 0x0b);
	_delay_ms(250);
	cmd_write_register(0x01, 0x0c, 0x0c);
	_delay_ms(250);
	
	
	//TXMAP - INDEX 0x02
	cmd_write_register(0x02, 0x00, 0x03);
	_delay_ms(250);
	cmd_write_register(0x02, 0x01, 0x06);
	_delay_ms(250);
	cmd_write_register(0x02, 0x02, 0x0d);
	_delay_ms(250);
	cmd_write_register(0x02, 0x03, 0x04);
	_delay_ms(250);
	cmd_write_register(0x02, 0x04, 0x02);
	_delay_ms(250);
	cmd_write_register(0x02, 0x05, 0x1e);
	_delay_ms(250);
	cmd_write_register(0x02, 0x06, 0x1d);
	_delay_ms(250);
	cmd_write_register(0x02, 0x07, 0x1c);
	_delay_ms(250);
	cmd_write_register(0x02, 0x08, 0x07);
	_delay_ms(250);
	cmd_write_register(0x02, 0x09, 0x0e);
	_delay_ms(250);
	cmd_write_register(0x02, 0x0a, 0x0f);
	_delay_ms(250);
	cmd_write_register(0x02, 0x0b, 0x10);
	_delay_ms(250);
	cmd_write_register(0x02, 0x0c, 0x05);
	_delay_ms(250);
	cmd_write_register(0x02, 0x0d, 0x08);
	_delay_ms(250);
	cmd_write_register(0x02, 0x0e, 0x22);
	_delay_ms(250);
	cmd_write_register(0x02, 0x0f, 0x21);
	_delay_ms(250);
	cmd_write_register(0x02, 0x10, 0x20);
	_delay_ms(250);
	cmd_write_register(0x02, 0x11, 0x1f);
	_delay_ms(250);
	
	
	//SELF - INDEX 0x10
	cmd_write_register(0x10, 0x00, 0x07);	//SELF SCAN TIME
	_delay_ms(250);
	cmd_write_register(0x10, 0x01, 0x1d); //SELF THRESHOLD
	_delay_ms(250);
	
	
	//MUTUAL - INDEX 0x20
	cmd_write_register(0x20, 0x00, 0x0a);	//MUTUAL SCAN TIME
	_delay_ms(250);
	cmd_write_register(0x20, 0x01, 0x27); //MUTUAL THRESHOLD
	_delay_ms(250);
	
	
	//DECODING - INDEX 0x30
	cmd_write_register(0x30, 0x00, 0x01);	//FLIP STATE
	_delay_ms(250);
	cmd_write_register(0x30, 0x01, 0x08); //NUM AVERAGES
	_delay_ms(250);
	cmd_write_register(0x30, 0x04, 0x96); //MIN TOUCH DISTANCE
	_delay_ms(250);
	cmd_write_register(0x30, 0x05, 0x03); //PEN DOWN TIME
	_delay_ms(250);
	cmd_write_register(0x30, 0x06, 0x03); //PEN UP TIME
	_delay_ms(250);
	cmd_write_register(0x30, 0x07, 0x00); //TOUCH SUPPRESSION (MAX TOUCH POINTS)
	_delay_ms(250);
	
	
	//GESTURES - INDEX 0x50
	cmd_write_register(0x50, 0x00, 0x96); //RX SWIPE LENGTH
	_delay_ms(250);
	cmd_write_register(0x50, 0x01, 0x96); //TX SWIPE LENGTH
	_delay_ms(250);
	cmd_write_register(0x50, 0x02, 0x96); //SWIPE BOUNDARY
	_delay_ms(250);
	cmd_write_register(0x50, 0x03, 0x46); //SWIPE HOLD THRESHOLD
	_delay_ms(250);
	cmd_write_register(0x50, 0x04, 0xc8); //SWIPE TIME [7:0]
	_delay_ms(250);
	cmd_write_register(0x50, 0x05, 0x00); //SWIPE TIME [15:8]
	_delay_ms(250);
	cmd_write_register(0x50, 0x06, 0xf4); //TAP TIME [7:0]
	_delay_ms(250);
	cmd_write_register(0x50, 0x07, 0x01); //TAP TIME [15:8]
	_delay_ms(250);
	cmd_write_register(0x50, 0x08, 0x78); //TAP THRESHOLD
	_delay_ms(250);
	cmd_write_register(0x50, 0x09, 0x03); //MIN SWIPE VELOCITY
	_delay_ms(250);
	cmd_write_register(0x50, 0x0a, 0x5e); //DOUBLE TAP TIME [7:0]
	_delay_ms(250);
	cmd_write_register(0x50, 0x0b, 0x1); //DOUBLE TAP [15:8]
	_delay_ms(250);
	cmd_write_register(0x50, 0x0c, 0x80); //GESTURE EDGE KEEPOUT
	_delay_ms(250);
	
	
	//CONFIG - INDEX 0xf0
	cmd_write_register(0xf0, 0x00, 0x40);	//SLEEP TIMEOUT [7:0]
	_delay_ms(250);
	cmd_write_register(0xf0, 0x01, 0x1f);	//SLEEP TIMEOUT [15:8]
	_delay_ms(250);
	cmd_write_register(0xf0, 0x02, 0x00);	//SLEEP TIMEOUT [23:16]
	_delay_ms(250);
	cmd_write_register(0xf0, 0x03, 0x00);	//SLEEP TIMEOUT [31:24]
	_delay_ms(250);
	cmd_write_register(0xf0, 0x07, 0x01); //TOUCH PACKET CFG
	_delay_ms(250);
	cmd_write_register(0xf0, 0x09, 0x81); //GESTURE PACKET CFG
	_delay_ms(250);
	cmd_write_register(0xf0, 0x0a, 0x01); //STATUS PACKET CFG
	_delay_ms(250);
	
	
	cmd_mtch6301(0x01);
	_delay_ms(250);
	cmd_mtch6301(0x17);
	*/
	
	
	//cmd_mtch6301(0x17);
	//cmd_mtch6301(0x17);
	/*
	reset the mtch6301
	*/
/*	ioport_set_pin_low(IOPORT_CREATE_PIN(PORTC, 7));
	_delay_ms(10);
	ioport_set_pin_high(IOPORT_CREATE_PIN(PORTC, 7));
	while(ioport_pin_is_high(IOPORT_CREATE_PIN(PORTC, 2)));
	
	//disable touch
	cmd_mtch6301(0x01);
	wait_result();
	//enable gesture
	cmd_write_register(0xF0,0x09,0x81);
	wait_result();
	//disable touch package 
	cmd_write_register(0xF0,0x07,0x01);
	wait_result();
	//read device id
	cmd_mtch6301(0x83);
	wait_result();
	//enable touch
	cmd_mtch6301(0x00);
	wait_result();*/
}

static inline uint8_t decode_mtch6301(uint8_t m_gesture)
{
	//twinkle(255, 255, 255);
	uint8_t cmd;
	
	switch(m_gesture)
	{
		case SINGLE_TAP:
			cmd = 0x01;
			latest_gesture = PLAY;
			////twinkle(255, 0, 0);
			break;
		case SINGLE_TAP_HOLD:
			cmd = 0x02;
			latest_gesture = CONNDISC;
			//twinkle(255, 255, 0);
			break;
		case DOUBLE_TAP:
			cmd = 0x03;
			latest_gesture = INQUIRE;
			break;
			//twinkle(0, 255, 0);
		case UP_SWIPE:
			cmd = 0x04;
			latest_gesture = VOLUP;
			//twinkle(0, 255, 255);
			break;
		case UP_SWIPE_HOLD:
			cmd = 0x04;
			latest_gesture = VOLUP;
			//twinkle(0, 255, 255);
			break;
		case RIGHT_SWIPE:
			cmd = 0x05;
			latest_gesture = NEXT;
			//twinkle(0, 0, 255);
			break;
		case RIGHT_SWIPE_HOLD:
			cmd = 0x05;
			latest_gesture = NEXT;
			//twinkle(0, 0, 255);
			break;
		case DOWN_SWIPE:
			cmd = 0x06;
			latest_gesture = VOLDOWN;
			//twinkle(255, 0, 255);
			break;
		case DOWN_SWIPE_HOLD:
			cmd = 0x06;
			latest_gesture = VOLDOWN;
			//twinkle(255, 0, 255);
			break;
		case LEFT_SWIPE:
			cmd = 0x07;
			latest_gesture = PREV;
			//twinkle(255, 255, 255);
			break;
		case LEFT_SWIPE_HOLD:
			cmd = 0x07;
			latest_gesture = PREV;
			//twinkle(255, 255, 255);
			break;
		default:
			cmd = 0x00;
			latest_gesture = NONE;
			break;
	}
	
	return cmd;
}

void init_mtchio(void)
{
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 5), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 6), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 7), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 6), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTR, 0), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 0), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 1), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
}

void init_ledhack(void)
{
	//ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 0), IOPORT_DIR_INPUT | IOPORT_RISING | IOPORT_PULL_DOWN);
	//ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 1), IOPORT_DIR_INPUT | IOPORT_RISING | IOPORT_PULL_DOWN);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 5), IOPORT_DIR_INPUT | IOPORT_RISING | IOPORT_PULL_DOWN);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 7), IOPORT_DIR_INPUT | IOPORT_RISING | IOPORT_PULL_DOWN);
}

void mapio(uint8_t cmd)
{
	if (cmd == 0x01)
	{
		latest_gesture = 0x01;
	} 
	else
	{
		//ioport_set_pin_low(IOPORT_CREATE_PIN(PORTA, 5));
	}/*
	if (cmd == 0x02)
	{
		ioport_set_pin_high(IOPORT_CREATE_PIN(PORTA, 5));
	}
	else
	{
		ioport_set_pin_low(IOPORT_CREATE_PIN(PORTA, 5));
	}
	if (cmd == 0x03)
	{
		ioport_set_pin_high(IOPORT_CREATE_PIN(PORTA, 5));
	}
	else
	{
		ioport_set_pin_low(IOPORT_CREATE_PIN(PORTA, 5));
	}*/
	if (cmd == 0x05)
	{
		latest_gesture = 0x05;
	}
	else
	{
		//ioport_set_pin_low(IOPORT_CREATE_PIN(PORTA, 6));
	}
	if (cmd == 0x07)
	{
		latest_gesture = 0x07;
	}
	else
	{
		//ioport_set_pin_low(IOPORT_CREATE_PIN(PORTA, 7));
	}
	if (cmd == 0x04)
	{
		latest_gesture = 0x04;
	}
	else
	{
		//ioport_set_pin_low(IOPORT_CREATE_PIN(PORTD, 6));
	}
	if (cmd == 0x06)
	{
		latest_gesture = 0x06;
	}
	else
	{
		//ioport_set_pin_low(IOPORT_CREATE_PIN(PORTR, 0));
	}
}

static inline void mtch6301_recv_data(void)
{
	uint8_t recv_pattern[RECV_SIZE_MTCH6301] = {0};	
	
	twi_package_t package = {
		.addr_length	= 0,
		.chip			= ADDR_MTCH6301,
		.buffer			= (void *)recv_pattern,
		.length			= RECV_SIZE_MTCH6301,
		.no_wait		= false
	};
	
	//twinkle(255, 0, 0);
	
	twi_master_read(&TWI_MASTER, &package);
	m_touch_pkg->tch	= (recv_pattern[1] & 0x04) >> 2;
	
	if (m_touch_pkg->tch == 0)
	{/*
		m_touch_pkg->id		= (recv_pattern[1] & 0x78) >> 3;		
		m_touch_pkg->pen	= (recv_pattern[1] & 0x01) >> 0;
		m_touch_pkg->x		= ((recv_pattern[3] & 0x1F) << 7) | ((recv_pattern[2] & 0x7F) >> 0);
		m_touch_pkg->y		= ((recv_pattern[5] & 0x1F) << 7) | ((recv_pattern[4] & 0x7F) >> 0);	
		twinkle(0, 255, 0);	*/
	} 
	else
	{
		m_touch_pkg->id		= (recv_pattern[1] & 0x78) >> 3;
		m_touch_pkg->gesture= (recv_pattern[2] & 0x7F) >> 0;
		
		mapio_mtch6301 = decode_mtch6301(m_touch_pkg->gesture);
		//usart_putchar(M_USART, mapio_mtch6301);
		//mapio(mapio_mtch6301);
		//_delay_ms(200);
		//mapio(0);
	}

	return NULL;
}

bool cmd_write_register(uint8_t index, uint8_t offset, uint8_t value)
{
	status_res = ERR_UNSUPPORTED_DEV;
	uint8_t cmd_slaver[6]={0x55,0x04,0x15,index,offset,value};
		
	#ifdef ENABLE_WDT
		wdt_kick();
	#endif
	
	twi_package_t package = {
		.addr_length	= 0,
		.chip			= ADDR_MTCH6301,
		.buffer			= (void *)cmd_slaver,
		.length			= 6,
		.no_wait		= false
	};
	
	status_res = twi_master_write(&TWI_MASTER, &package);
	cmd_status = 1;
	return (status_res != STATUS_OK);
}

uint8_t read_register(uint8_t index, uint8_t offset)
{
	cmd_read_register(index, offset);
	wait_result();
	
	return REG_READ;
}

void cmd_read_register(uint8_t index, uint8_t offset)
{
	uint8_t cmd_slaver[5]={0x55,0x03,0x16,index,offset};
	
	twi_package_t package = {
		.addr_length	= 0,
		.chip			= ADDR_MTCH6301,
		.buffer			= (void *)cmd_slaver,
		.length			= 5,
		.no_wait		= false
	};
	
	twi_master_write(&TWI_MASTER, &package);
	cmd_status = 1;
}

void cmd_mtch6301(uint8_t cmd)
{
	status_res = ERR_UNSUPPORTED_DEV;
	uint8_t cmd_slaver[3]={0x55,0x01,cmd};
		
	#ifdef ENABLE_WDT
		wdt_kick();
	#endif
	
	twi_package_t package = {
		.addr_length	= 0,
		.chip			= ADDR_MTCH6301,
		.buffer			= (void *)cmd_slaver,
		.length			= 3,
		.no_wait		= false
	};
	
	status_res = twi_master_write(&TWI_MASTER, &package);
	cmd_status = 1;
}

static inline uint8_t response_cmd()
{
	uint8_t i;
	
	twi_package_t package = {
		.addr_length	= 0,
		.chip			= ADDR_MTCH6301,
		.buffer			= (void *)response_slaver,
		.length			= 9,
		.no_wait		= false
	};
	
	twi_master_read(&TWI_MASTER, &package);
	cmd_status = 0;
	
	for (i=0;i<9;i++)
	{
		//usart_putchar(&USARTD0, response_slaver[i]);
	}
	
	return response_slaver[3];
}


ISR(PORTC_INT_vect)
{	
	PORTC.INTFLAGS = 0x20;
//	usart_putchar(&USARTD0, 0xF0);
	//twinkle(255, 255, 255);
	
	switch(cmd_status)
	{
		case 0:			
			mtch6301_recv_data();
			break;
		case 1:
			cmd_res = response_cmd();
			break;
		default:
			return;
	}
}