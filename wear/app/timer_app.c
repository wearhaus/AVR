/*
 * timer_app.c
 *
 * Created: 2014/8/7 20:34:21
 *  Author: Harifun
 */ 
#include "asf.h"
#include <config_app.h>
#include <adc_app.h>
#include <wear.h>
#include "uart.h"

volatile bool flag_timer5;
volatile bool flag_timerd5;
volatile bool flag_timer4;

static inline void timer4_overflow_interrupt_callback(void)
{
	tc45_clear_overflow(&TCC4);

	flag_timer4 = true;

	adc_start_conversion(&ADCA, ADC_CH0);
}

static inline void timer5_overflow_interrupt_callback(void)
{
	flag_timer5 = true;/*
	if (flag_initcharge) {
		flag_initcharge = false;
		init_charge();
	}*/
	tc45_clear_overflow(&TCC5);
	update_led_charge();
	//usart_putchar(&USARTD0, 0x28);
}


static U16 disablePulseCount=200;

void resetDisablePulseCount(void)
{
     disablePulseCount=200;
     tempPulseDisabled=1;
}

static inline timerD5_overflow_interrupt_callback(void)
{
	flag_timerd5 = true;
	//wdt_reset(); // kick the watchdog - DEPRECATED
	
	if(disablePulseCount>=1)
	{
	    disablePulseCount--;
	    if(disablePulseCount==0)
	        tempPulseDisabled=0;
	}
	tc45_clear_overflow(&TCD5);
	
        //move led display here because of uart/led timing requirement.
	if(flag_ledRefresh)
	{
	     tc45_set_overflow_interrupt_level(&TCD5, TC45_INT_LVL_OFF);

           for (int i=0; i<9; i++) {
		   colors[i] = buffer_data[i+2];
		  // nvm_eeprom_write_byte(i+1, colors[i]);
           }

	    if (!ischarging()) {
		set_flash_ws2812(m_led_struct, 6);
	    }		   
	     tc45_set_overflow_interrupt_level(&TCD5, TC45_INT_LVL_MED);
	     flag_ledRefresh=0;
	}
	//adc_start_conversion(&ADCA, ADC_CH0);
}

void init_timer4(void)
{
	/*
	 * Configure TCC4 to generate 50ms overflow interrupt
	 * using 4MHz (0.25us) resolution clock (20us = 80 * 0.25us)
	 */
	/* Unmask clock for TCC4 */
	tc45_enable(&TCC4);
	/* Enable overflow interrupt */
	tc45_set_overflow_interrupt_level(&TCC4, TC45_INT_LVL_LO);
	/* Configure TC in normal mode */
	tc45_set_wgm(&TCC4, TC45_WG_NORMAL);
	/* Configure call back interrupt */
	tc45_set_overflow_interrupt_callback(&TCC4,	timer4_overflow_interrupt_callback);
	/* Configure TC period and resolution */
	tc45_write_period(&TCC4, 80);
	tc45_set_resolution(&TCC4, 4000000);
	
	pmic_enable_level(PMIC_LVL_LOW);
	cpu_irq_enable();
}

void init_timer5(void)
{
	/*
	 * Configure TCC4 to generate 50ms overflow interrupt
	 * using 500kHz (2us) resolution clock (20ms = 10000 * 2us)
	 */
	/* Unmask clock for TCC4 */
//	tc45_enable(&TCC5);
	/* Enable overflow interrupt */
	tc45_set_overflow_interrupt_level(&TCC5, TC45_INT_LVL_MED);
	/* Configure TC in normal mode */
	tc45_set_wgm(&TCC5, TC45_WG_NORMAL);
	/* Configure call back interrupt */
	tc45_set_overflow_interrupt_callback(&TCC5,	timer5_overflow_interrupt_callback);
	/* Configure TC period and resolution */
//	tc45_write_period(&TCC5, 10000);
//	tc45_set_resolution(&TCC5, 500000);
	tc45_write_period(&TCC5, 62500);
	tc45_set_resolution(&TCC5, 125000);
	
	pmic_enable_level(PMIC_LVL_MEDIUM);
	cpu_irq_enable();
}

void init_timerd5(void)
{
	tc45_enable(&TCD5);
	tc45_set_overflow_interrupt_level(&TCD5, TC45_INT_LVL_MED);
	/* Configure TC in normal mode */
	tc45_set_wgm(&TCD5, TC45_WG_NORMAL);
	/* Configure call back interrupt */
	tc45_set_overflow_interrupt_callback(&TCD5,	timerD5_overflow_interrupt_callback);
	/* Configure TC period and resolution */
	tc45_write_period(&TCD5, 162);//EVERY 5ms
	tc45_set_resolution(&TCD5, 31250);
	
	pmic_enable_level(PMIC_LVL_MEDIUM);
	cpu_irq_enable();
}
