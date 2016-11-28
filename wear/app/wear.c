/*
 * wear.c
 *
 * Created: 2014/8/10 13:27:37
 *  Author: Harifun
 */ 

#include "asf.h"
#include <ws2812.h>
#include <timer_app.h>
#include <config_app.h>
#include <uart.h>
#include <adc_app.h>
#include <mtch6301.h>
#include "eeprom.h"
//1720------4.20V
//1670------4.08V
//1300------3.17V
#define VBAT_FS					1690
#define VBAT_OFFSET				1300
#define VBAT_INT				(VBAT_FS-VBAT_OFFSET)/2
#define VBAT_FS_THIRD			(VBAT_OFFSET + VBAT_INT)						//3.49v
//#define VBAT_FS_TWOTHIRD		(VBAT_FS_THIRD + VBAT_INT)						//3.80v
#define LED_CHARGE_MAX			128

volatile uint16_t chargeLVL0 = 0;
volatile uint16_t chargeLVL1 = 2400;
volatile uint16_t chargeLVL2 = 4800;
volatile uint16_t chargeLVL3 = 7200;
volatile uint16_t chargeLVL4 = 10800;
volatile uint16_t chargeLVL5 = 12000;
volatile uint16_t chargeLVL6 = 14400;
volatile uint8_t  chargeBrightness = 96;

// all rgb led data
volatile led_struct m_led_struct[LED_NUMBER];
uint8_t flag_ledRefresh=0;

// charging, fft_l, fft_r status
volatile uint8_t core_status = 0;
volatile uint8_t led_status = 0;
volatile uint8_t old_core_status = 0;

// charge adc value
int16_t value_charge = 0;
int16_t value_light = 0;
int16_t value_batt = 0;

// flag of light sensor
volatile uint8_t flag_light = 0;
volatile uint8_t flag_charge = 0;
volatile uint8_t flag_batt = 0;
volatile uint8_t flag_initcharge = 0;


// threshhold of charging
uint8_t flag_thresh_charging = 0;

// charge status and old status
bool charge_status = false;
bool old_charge_status = false;

//1s twinkle flag
volatile uint8_t flag_twinkle = 0,cflag_twinkle = 0;		

extern uint8_t fht_flag;

/*
	turn off the mcu
*/
void init_turnoffpin(void)
{
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 5), IOPORT_DIR_INPUT | IOPORT_RISING | IOPORT_PULL_UP);
	PORTCFG.MPCMASK = 0x20;
	PORTC.INTMASK = 0x20;
	PORTC.INTCTRL = PMIC_HILVLEN_bm;
	pmic_enable_level(PMIC_HILVLEN_bm);
	cpu_irq_enable();
}

/*
	* \param pin The pin to configure
	IOPORT_CREATE_PIN(PORTR, 1) as example
*/
void init_chargingpin(port_pin_t pin)
{
	ioport_configure_pin(pin, IOPORT_DIR_INPUT | IOPORT_INIT_HIGH);
}

/*
	judge whether the terminal was charging
*/
bool ischarging(void)
{
#ifndef DEMO	
	if (ioport_pin_is_high(CHARGING_PIN))
	{
		return true;
	}
	else
	{
		return false;
	}
#else
      return false;
#endif
}

/*
	judge whether the terminal was playing
*/
bool isplaying(void)
{
	if (ioport_pin_is_high(PLAYING_PIN))
	{
		return true;
	}
	else
	{
		return true;
	}
}

/* 
	judge whether changed from charging to no charging or no charging to charging
*/
bool ischanged(void)
{
	charge_status = ischarging();
	
	if (old_charge_status == charge_status)
	{
		return false;
	} 
	else
	{
		old_charge_status = charge_status;
		return true;
	}
}

void detect_charging(void)
{
	charge_status = ischarging();
}

void detect_led(void)
{
	if (ischarging())
	{
		if (led_status != STATUS_LED_CHARGE)
		{
			led_status = STATUS_LED_CHARGE;	
//			usart_putchar(M_USART, 0x03);	
			return;
		}
		else
		{
//			usart_putchar(M_USART, 0x23);
			return;
		}
	}
	else if (isplaying())
	{
		if ((led_status == STATUS_LED_CHARGE) || (led_status == STATUS_LED_OFF) || (led_status == STATUS_LED_UART))
		{
			led_status = STATUS_LED_FFT;
			tc45_disable(&TCC5);
//			usart_putchar(M_USART, 0x13);
			return;
		}
		else
		{
//			usart_putchar(M_USART, 0x13);
			return;
		}
	}
	else if (led_status == STATUS_LED_UART)
	{
//		usart_putchar(M_USART, 0x33);
		return;
	}
	else if (led_status != STATUS_LED_OFF)
	{
		led_status = STATUS_LED_OFF;
		tc45_disable(&TCC5);
//		usart_putchar(M_USART, 0x03);
		return;
	}
}

void twinkle(uint8_t value_r, uint8_t value_g, uint8_t value_b)
{
	
	for(uint8_t i=0; i<6; i++)
	{
		m_led_struct[i].r = value_r;
		m_led_struct[i].g = value_g;
		m_led_struct[i].b = value_b;
	}
	
	set_flash_ws2812(m_led_struct, 6);
}

int map_value(int in_min, int value_convert)
{
	return (value_convert - in_min) * (LED_CHARGE_MAX) / (VBAT_INT);
}

void rainbow(void)
{
	m_led_struct[0].r = 255;
	m_led_struct[0].g = 0;
	m_led_struct[0].b = 0;
	
	m_led_struct[1].r = 0;
	m_led_struct[1].g = 255;
	m_led_struct[1].b = 0;
	
	m_led_struct[2].r = 0;
	m_led_struct[2].g = 0;
	m_led_struct[2].b = 255;
	
	m_led_struct[3].r = 255;
	m_led_struct[3].g = 0;
	m_led_struct[3].b = 0;
	
	m_led_struct[4].r = 0;
	m_led_struct[4].g = 255;
	m_led_struct[4].b = 0;

	m_led_struct[5].r = 0;
	m_led_struct[5].g = 0;
	m_led_struct[5].b = 255;
	
	set_flash_ws2812(m_led_struct, 6);
}

void set_led_brightness(int led1brightness, int led2brightness, int led3brightness, uint8_t full){
	
	if (led1brightness == 0)
	{
		led1brightness = 1;
	}

	if (full == 0)
	{
		m_led_struct[0].r = led1brightness;
		m_led_struct[0].g = 0;
		m_led_struct[0].b = 0;
		
		m_led_struct[1].r = led2brightness;
		m_led_struct[1].g = 0;
		m_led_struct[1].b = 0;
		
		m_led_struct[2].r = led3brightness;
		m_led_struct[2].g = 0;
		m_led_struct[2].b = 0;
		
		m_led_struct[3].r = led1brightness;
		m_led_struct[3].g = 0;
		m_led_struct[3].b = 0;
		
		m_led_struct[4].r = led2brightness;
		m_led_struct[4].g = 0;
		m_led_struct[4].b = 0;

		m_led_struct[5].r = led3brightness;
		m_led_struct[5].g = 0;
		m_led_struct[5].b = 0;
	}
	else if (full == 1)
	{
		m_led_struct[0].r = 0;
		m_led_struct[0].g = LED_CHARGE_MAX;
		m_led_struct[0].b = 0;
		
		m_led_struct[1].r = 0;
		m_led_struct[1].g = LED_CHARGE_MAX;
		m_led_struct[1].b = 0;
		
		m_led_struct[2].r = 0;
		m_led_struct[2].g = LED_CHARGE_MAX;
		m_led_struct[2].b = 0;
		
		m_led_struct[3].r = 0;
		m_led_struct[3].g = LED_CHARGE_MAX;
		m_led_struct[3].b = 0;
		
		m_led_struct[4].r = 0;
		m_led_struct[4].g = LED_CHARGE_MAX;
		m_led_struct[4].b = 0;

		m_led_struct[5].r = 0;
		m_led_struct[5].g = LED_CHARGE_MAX;
		m_led_struct[5].b = 0;
	}
	else 
	{
		//	m_led_struct[0].r = 0;
		//	m_led_struct[0].g = 0;
		//	m_led_struct[0].b = LED_CHARGE_MAX;
		
		m_led_struct[1].r = 0;
		m_led_struct[1].g = 0;
		m_led_struct[1].b = LED_CHARGE_MAX;
		
		m_led_struct[2].r = 0;
		m_led_struct[2].g = 0;
		m_led_struct[2].b = LED_CHARGE_MAX;
		
		m_led_struct[3].r = 0;
		m_led_struct[3].g = 0;
		m_led_struct[3].b = LED_CHARGE_MAX;
		
		m_led_struct[4].r = 0;
		m_led_struct[4].g = 0;
		m_led_struct[4].b = LED_CHARGE_MAX;

		m_led_struct[5].r = 0;
		m_led_struct[5].g = 0;
		m_led_struct[5].b = LED_CHARGE_MAX;
	}
		
	set_flash_ws2812(m_led_struct, 6);
}

void clear_led(void)
{
	for (uint8_t i; i < LED_NUMBER; i++)
	{
		m_led_struct[i].r=0;
		m_led_struct[i].g=0;
		m_led_struct[i].b=0;
	}
	set_flash_ws2812(m_led_struct, 6);
}

void load_data_color_led(void) {
	m_led_struct[0].r = colors[0];
	m_led_struct[0].g = colors[1];
	m_led_struct[0].b = colors[2];
	
	m_led_struct[1].r = colors[3];
	m_led_struct[1].g = colors[4];
	m_led_struct[1].b = colors[5];
	
	m_led_struct[2].r = colors[6];
	m_led_struct[2].g = colors[7];
	m_led_struct[2].b = colors[8];
	
	m_led_struct[3].r = colors[0];
	m_led_struct[3].g = colors[1];
	m_led_struct[3].b = colors[2];
	
	m_led_struct[4].r = colors[3];
	m_led_struct[4].g = colors[4];
	m_led_struct[4].b = colors[5];

	m_led_struct[5].r = colors[6];
	m_led_struct[5].g = colors[7];
	m_led_struct[5].b = colors[8];

	set_flash_ws2812(m_led_struct, 6);
}

/*
	update rgb led when charging function
*/
void update_led_charge(void)
{
	flag_timer5 = false;
	
	if (flag_initcharge) {
		if (value_batt > 1375 && value_batt <= 1470) {
			value_charge = chargeLVL2;
		}
		else if (value_batt > 1200 && value_batt <= 1375) {
			value_charge = chargeLVL4;
		}
		else {
			//avoid too dark and give user a prompt
			if(chargeLVL0==0)
			    value_charge = 800;
			else
			value_charge = chargeLVL0;
		}
		flag_initcharge = false;
	}
	
	if (ischarging() == false&&core_status==STATUS_CHARGE)
	{
		old_core_status = core_status;
		core_status = STATUS_BATT_READ;
		change_adc_channel(core_status);
		tc45_enable(&TCC4);
		tc45_disable(&TCC5);
		load_data_color_led();
	}
	
	if (value_charge < chargeLVL1) {
		clear_led();
		m_led_struct[0].r = (value_charge * chargeBrightness) / chargeLVL1;
		m_led_struct[0].g = 0;
		m_led_struct[0].b = 0;
			
		m_led_struct[1].r = 0;
		m_led_struct[1].g = 0;
		m_led_struct[1].b = 0;
			
		m_led_struct[2].r = 0;
		m_led_struct[2].g = 0;
		m_led_struct[2].b = 0;
			
		m_led_struct[3].r = (value_charge * chargeBrightness) / chargeLVL1;
		m_led_struct[3].g = 0;
		m_led_struct[3].b = 0;
			
		m_led_struct[4].r = 0;
		m_led_struct[4].g = 0;
		m_led_struct[4].b = 0;

		m_led_struct[5].r = 0;
		m_led_struct[5].g = 0;
		m_led_struct[5].b = 0;
		set_flash_ws2812(m_led_struct, 6);
	}
	else if (value_charge < chargeLVL3) {
		clear_led();
		m_led_struct[0].r = chargeBrightness;
		m_led_struct[0].g = 0;
		m_led_struct[0].b = 0;
			
		m_led_struct[1].r = 0;
		m_led_struct[1].g = 0;
		m_led_struct[1].b = 0;
			
		m_led_struct[2].r = (value_charge - chargeLVL1) * chargeBrightness / chargeLVL3;
		m_led_struct[2].g = 0;
		m_led_struct[2].b = 0;
			
		m_led_struct[3].r = chargeBrightness;
		m_led_struct[3].g = 0;
		m_led_struct[3].b = 0;
			
		m_led_struct[4].r = 0;
		m_led_struct[4].g = 0;
		m_led_struct[4].b = 0;

		m_led_struct[5].r = (value_charge - chargeLVL1) * chargeBrightness / chargeLVL3;
		m_led_struct[5].g = 0;
		m_led_struct[5].b = 0;
		set_flash_ws2812(m_led_struct, 6);
	}
	else if (value_charge < chargeLVL5) {
		clear_led();
		m_led_struct[0].r = chargeBrightness;
		m_led_struct[0].g = 0;
		m_led_struct[0].b = 0;
			
		m_led_struct[1].r = (value_charge - chargeLVL3) * chargeBrightness / chargeLVL5;
		m_led_struct[1].g = 0;
		m_led_struct[1].b = 0;
			
		m_led_struct[2].r = chargeBrightness;
		m_led_struct[2].g = 0;
		m_led_struct[2].b = 0;
			
		m_led_struct[3].r = chargeBrightness;
		m_led_struct[3].g = 0;
		m_led_struct[3].b = 0;
			
		m_led_struct[4].r = (value_charge - chargeLVL3) * chargeBrightness / chargeLVL5;
		m_led_struct[4].g = 0;
		m_led_struct[4].b = 0;

		m_led_struct[5].r = chargeBrightness;
		m_led_struct[5].g = 0;
		m_led_struct[5].b = 0;
		set_flash_ws2812(m_led_struct, 6);
	}
	else if (value_charge < chargeLVL6) {
		clear_led();
		m_led_struct[0].r = 0;
		m_led_struct[0].g = (value_charge - chargeLVL4) * chargeBrightness / chargeLVL6;
		m_led_struct[0].b = 0;
			
		m_led_struct[1].r = 0;
		m_led_struct[1].g = (value_charge - chargeLVL4) * chargeBrightness / chargeLVL6;
		m_led_struct[1].b = 0;
			
		m_led_struct[2].r = 0;
		m_led_struct[2].g = (value_charge - chargeLVL4) * chargeBrightness / chargeLVL6;
		m_led_struct[2].b = 0;
			
		m_led_struct[3].r = 0;
		m_led_struct[3].g = (value_charge - chargeLVL4) * chargeBrightness / chargeLVL6;
		m_led_struct[3].b = 0;
			
		m_led_struct[4].r = 0;
		m_led_struct[4].g = (value_charge - chargeLVL4) * chargeBrightness / chargeLVL6;
		m_led_struct[4].b = 0;

		m_led_struct[5].r = 0;
		m_led_struct[5].g = (value_charge - chargeLVL4) * chargeBrightness / chargeLVL6;
		m_led_struct[5].b = 0;
		set_flash_ws2812(m_led_struct, 6);
	}
	else {
		clear_led();
		m_led_struct[0].r = 0;
		m_led_struct[0].g = chargeBrightness;
		m_led_struct[0].b = 0;
			
		m_led_struct[1].r = 0;
		m_led_struct[1].g = chargeBrightness;
		m_led_struct[1].b = 0;
			
		m_led_struct[2].r = 0;
		m_led_struct[2].g = chargeBrightness;
		m_led_struct[2].b = 0;
			
		m_led_struct[3].r = 0;
		m_led_struct[3].g = chargeBrightness;
		m_led_struct[3].b = 0;
			
		m_led_struct[4].r = 0;
		m_led_struct[4].g = chargeBrightness;
		m_led_struct[4].b = 0;

		m_led_struct[5].r = 0;
		m_led_struct[5].g = chargeBrightness;
		m_led_struct[5].b = 0;
		set_flash_ws2812(m_led_struct, 6);
	}/*
	else 
	{
		m_led_struct[0].r = 0;
		m_led_struct[0].g = 96;
		m_led_struct[0].b = 0;
			
		m_led_struct[1].r = 0;
		m_led_struct[1].g = 96;
		m_led_struct[1].b = 0;
			
		m_led_struct[2].r = 0;
		m_led_struct[2].g = 96;
		m_led_struct[2].b = 0;
			
		m_led_struct[3].r = 0;
		m_led_struct[3].g = 96;
		m_led_struct[3].b = 0;
			
		m_led_struct[4].r = 0;
		m_led_struct[4].g = 96;
		m_led_struct[4].b = 0;

		m_led_struct[5].r = 0;
		m_led_struct[5].g = 96;
		m_led_struct[5].b = 0;
		set_flash_ws2812(m_led_struct, 6);
	}*/
	/*
	if (flag_timer5 == true)
	{
		flag_timer5 = false;
		int brightness = 0;
		
		if (flag_thresh_charging)
		{
			value_charge += 5;
		}
		
		if (value_charge > 1690)
		{
			//	clear_led();
			//rainbow();
			m_led_struct[0].r = 32;
			m_led_struct[0].g = 0;
			m_led_struct[0].b = 0;
			
			m_led_struct[1].r = 8;
			m_led_struct[1].g = 0;
			m_led_struct[1].b = 0;
			
			m_led_struct[2].r = 8;
			m_led_struct[2].g = 0;
			m_led_struct[2].b = 0;
			
			m_led_struct[3].r = 32;
			m_led_struct[3].g = 0;
			m_led_struct[3].b = 0;
			
			m_led_struct[4].r = 8;
			m_led_struct[4].g = 0;
			m_led_struct[4].b = 0;

			m_led_struct[5].r = 8;
			m_led_struct[5].g = 0;
			m_led_struct[5].b = 0;
			
			set_flash_ws2812(m_led_struct, 6);
			//			set_led_brightness(1, 0, 0, 0);//set_led_brightness(1, 0, 0, 1);
		}
		else if (value_charge <= 1690 && value_charge > 1675)
		{
			//	clear_led();
			//rainbow();
			if ((value_charge + 5) <= 1690)
			{
				flag_thresh_charging = 0;
			}
			else
			{
				flag_thresh_charging = 1;
			}
			m_led_struct[0].r = 96;
			m_led_struct[0].g = 0;
			m_led_struct[0].b = 0;
			
			m_led_struct[1].r = 32;
			m_led_struct[1].g = 0;
			m_led_struct[1].b = 0;
			
			m_led_struct[2].r = 8;
			m_led_struct[2].g = 0;
			m_led_struct[2].b = 0;
			
			m_led_struct[3].r = 96;
			m_led_struct[3].g = 0;
			m_led_struct[3].b = 0;
			
			m_led_struct[4].r = 32;
			m_led_struct[4].g = 0;
			m_led_struct[4].b = 0;

			m_led_struct[5].r = 8;
			m_led_struct[5].g = 0;
			m_led_struct[5].b = 0;
			
			set_flash_ws2812(m_led_struct, 6);
//			set_led_brightness(1, 0, 0, 0);//set_led_brightness(1, 0, 0, 1);
		}
		else if ((value_charge <= 1675) && (value_charge > 1660))
		{
			if ((value_charge + 5) <= 1675)
			{
				flag_thresh_charging = 0;
			}
			else
			{
				flag_thresh_charging = 1;
			}
			m_led_struct[0].r = 96;
			m_led_struct[0].g = 0;
			m_led_struct[0].b = 0;
			
			m_led_struct[1].r = 96;
			m_led_struct[1].g = 0;
			m_led_struct[1].b = 0;
			
			m_led_struct[2].r = 32;
			m_led_struct[2].g = 0;
			m_led_struct[2].b = 0;
			
			m_led_struct[3].r = 96;
			m_led_struct[3].g = 0;
			m_led_struct[3].b = 0;
			
			m_led_struct[4].r = 96;
			m_led_struct[4].g = 0;
			m_led_struct[4].b = 0;

			m_led_struct[5].r = 32;
			m_led_struct[5].g = 0;
			m_led_struct[5].b = 0;
			
			set_flash_ws2812(m_led_struct, 6);
//			brightness = map_value(VBAT_OFFSET, value_charge);
//			set_led_brightness(brightness, 0, 0, 0);
		}
		else if ((value_charge <= 1660) && (value_charge > 1638))
		{
			if ((value_charge + 5) <= 1638)
			{
				flag_thresh_charging = 0;
			}
			else
			{
				flag_thresh_charging = 1;
			}
			
			m_led_struct[0].r = 96;
			m_led_struct[0].g = 0;
			m_led_struct[0].b = 0;
			
			m_led_struct[1].r = 96;
			m_led_struct[1].g = 0;
			m_led_struct[1].b = 0;
			
			m_led_struct[2].r = 96;
			m_led_struct[2].g = 0;
			m_led_struct[2].b = 0;
			
			m_led_struct[3].r = 96;
			m_led_struct[3].g = 0;
			m_led_struct[3].b = 0;
			
			m_led_struct[4].r = 96;
			m_led_struct[4].g = 0;
			m_led_struct[4].b = 0;

			m_led_struct[5].r = 96;
			m_led_struct[5].g = 0;
			m_led_struct[5].b = 0;
			
			set_flash_ws2812(m_led_struct, 6);
//			brightness = map_value(VBAT_FS_THIRD, value_charge);
//			set_led_brightness(LED_CHARGE_MAX, brightness, 0, 0);
		}
		else if (value_charge <= 1645)
		{
			if ((value_charge + 10) <= 1645)
			{
				flag_thresh_charging = 0;
			}
			else
			{
				flag_thresh_charging = 1;
			}
			
			m_led_struct[0].r = 0;
			m_led_struct[0].g = 96;
			m_led_struct[0].b = 0;
			
			m_led_struct[1].r = 0;
			m_led_struct[1].g = 96;
			m_led_struct[1].b = 0;
			
			m_led_struct[2].r = 0;
			m_led_struct[2].g = 96;
			m_led_struct[2].b = 0;
			
			m_led_struct[3].r = 0;
			m_led_struct[3].g = 96;
			m_led_struct[3].b = 0;
			
			m_led_struct[4].r = 0;
			m_led_struct[4].g = 96;
			m_led_struct[4].b = 0;

			m_led_struct[5].r = 0;
			m_led_struct[5].g = 96;
			m_led_struct[5].b = 0;
			
			set_flash_ws2812(m_led_struct, 6);

		}
	}*/
	value_charge += 1;
}


void test(void)
{
	m_led_struct[0].r = 255;
	m_led_struct[0].g = 0;
	m_led_struct[0].b = 0;
	
	m_led_struct[1].r = 0;
	m_led_struct[1].g = 255;
	m_led_struct[1].b = 0;
	
	m_led_struct[2].r = 0;
	m_led_struct[2].g = 0;
	m_led_struct[2].b = 255;
	
	set_flash_ws2812(m_led_struct, 6);
}

ISR(PORTD_INT_vect)
{
      unsigned int count=0;
	PORTD.INTFLAGS = 0x20;

	clear_led();
	while(1)
	{
#ifdef LIMIT_LOOP
           if(count++>10000)
		   	break;
#endif
		;
	}
}

/* Sends a 2 byte response back to CSR consisting of message ID and payload */
void send_response (uint8_t message_ID, uint8_t payload)
{
	char resp[2] = {message_ID, payload};
	uart_send_bytes(resp, 2);
}

void send_light_data(void)
{
	uint8_t resp[3] = {0};
	
	resp[2] = value_light & 0x00FF;
	resp[1] = (value_light & 0xFF00) >> 8;
	resp[0] = UART_GET_AMBIENT;
	
	uart_send_bytes(resp, 3);
}

void send_battery_data(void)
{
	uint8_t resp[2] = {0};
	
	resp[1] = (uint8_t) (value_batt / 10);
	resp[0] = UART_GET_BATTERY;
	
	uart_send_bytes(resp, 2);
}

void send_charging_data(void)
{
	uint8_t resp[2] = {0};
	
	resp[1] = (uint8_t) ischarging();
	resp[0] = UART_GET_CHARGING;
	
	uart_send_bytes(resp, 2);
}

void send_mtch_data(uint8_t index, uint8_t offset)
{
	uint8_t resp[4] = {0};
	
	resp[3] = read_register(buffer_data[2], buffer_data[3]);
	resp[2] = offset;
	resp[1] = index;
	resp[0] = UART_GET_MTCH;
	
    uart_send_bytes(resp, 4);
}

void send_mtch_update_status(void) {
	uint8_t resp[2] = {0};
	
	resp[1] = EEPROM_MTCH_UPDATE_BOOL;
	resp[0] = UART_GET_MTCH;
	
	uart_send_bytes(resp, 2);
}

void send_color_data(void)
{
	uint8_t resp[10] = {0};
	
	resp[0] = UART_GET_COLOR;
	for (uint8_t i=1; i<9; i++) {
		resp[i] = colors[i];
	}
	
	uart_send_bytes(resp, 10);
}

void send_pulse_data(void)
{
	uint8_t resp[2] = {0};

	resp[1] = get_pulse_state();
	resp[0] = UART_GET_PULSE;
	
	uart_send_bytes(resp, 2);
}

void load_buff_data_color_led(void)
{
	m_led_struct[0].r = buffer_data[2];
	m_led_struct[0].g = buffer_data[3];
	m_led_struct[0].b = buffer_data[4];
	
	m_led_struct[1].r = buffer_data[5];
	m_led_struct[1].g = buffer_data[6];
	m_led_struct[1].b = buffer_data[7];
	
	m_led_struct[2].r = buffer_data[8];
	m_led_struct[2].g = buffer_data[9];
	m_led_struct[2].b = buffer_data[10];
	
	m_led_struct[3].r = buffer_data[2];
	m_led_struct[3].g = buffer_data[3];
	m_led_struct[3].b = buffer_data[4];
	
	m_led_struct[4].r = buffer_data[5];
	m_led_struct[4].g = buffer_data[6];
	m_led_struct[4].b = buffer_data[7];

	m_led_struct[5].r = buffer_data[8];
	m_led_struct[5].g = buffer_data[9];
	m_led_struct[5].b = buffer_data[10];

	set_flash_ws2812(m_led_struct, 6);
//	set_flash_ws2812(m_led_struct, 6);
}

void restore_led_from_eeprom(void)
{	
	/* Read LED data bytes from EEPROM to internal buffer */
	/* EEPROM_LED1_R_BYTE is the start address of the bytes , load into buffer_data*/
//	nvm_eeprom_read_buffer((uint16_t)EEPROM_LED1_R_BYTE, &buffer_data[2], 9);
	
	colors[0] = EEPROM_LED1_R_BYTE;
	colors[1] = EEPROM_LED1_G_BYTE;
	colors[2] = EEPROM_LED1_B_BYTE;

	colors[3] = EEPROM_LED2_R_BYTE;
	colors[4] = EEPROM_LED2_G_BYTE;
	colors[5] = EEPROM_LED2_B_BYTE;
	
	colors[6] = EEPROM_LED3_R_BYTE;
	colors[7] = EEPROM_LED3_G_BYTE;
	colors[8] = EEPROM_LED3_B_BYTE;
	
	led_set_from_colors();
//usart_putchar(&USARTD0, 0x25);
}

void save_led_to_eeprom(void) {
	for (uint8_t i=0; i++; i<9) {
		nvm_eeprom_write_byte(i+1, colors[i]);
	}
}

void restore_state_eep(void)
{
	/* If music visualization was enabled */
	if (EEPROM_VISUALIZATION_STATE)
	{
		/* Do not change adc channel or start the timer if ADC is not enabled yet!*/
		fht_flag = true;
	}
	/* If visualization wasn't on then load solid colors that are saved including no color saved */
	else 
	{
		fht_flag = false;
		restore_led_from_eeprom();
	}
}

void wdt_kick(void) {
	if (flag_timerd5) {
		flag_timerd5 = false;
		wdt_reset();
	}
}

void init_charge() {
	core_status = STATUS_CHARGE;
	if (value_batt > 1375 && value_batt <= 1470) {
		value_charge = chargeLVL3;
	}
	else if (value_batt > 1200 && value_batt <= 1375) {
		value_charge = chargeLVL5;
	}
	else {
		value_charge = chargeLVL0;
	}
}

