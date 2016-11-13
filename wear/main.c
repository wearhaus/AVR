/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include <asf.h>
#include <config_app.h>
#include <adc_app.h>
#include <timer_app.h>
#include <wear.h>
#include <uart.h>
#include <mtch6301.h>
#include <util/delay.h>
#include <eeprom.h>

uint8_t new_message_ID;
uint8_t fht_flag = false;		//judge if the fht led function was turned on or off
char response[2] = {0, 0};

/*
uint8_t playPauseColor[9] = {0, 128, 128, 0, 128, 128, 0, 128, 128};
uint8_t forwardColor[9] = {0, 128, 128, 0, 0, 0, 0, 0, 0};
uint8_t backwardColor[9] = {0, 0, 0, 0, 128, 128, 0, 0, 0};
uint8_t volumeUpColor[9] = {0, 0, 0, 0, 0, 0, 0, 128, 128};
uint8_t volumeDownColor[9] = {0, 128, 128, 0, 128, 128, 0, 0, 0};
uint8_t connDiscColor[9] = {0, 128, 0, 0, 0, 0, 0, 128, 0};
uint8_t inquiryColor[9] = {0, 0, 0, 128, 128, 0, 128, 128, 0};
*/

static void test_pin(void) {
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 6), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	twinkle(0, 255, 255);
	ioport_set_pin_high(IOPORT_CREATE_PIN(PORTA, 6));
	_delay_ms(2000);
	ioport_set_pin_low(IOPORT_CREATE_PIN(PORTA, 6));
	twinkle(0, 0, 0);
}

void set_temp_color(uint8_t* tempcolor) {
	m_led_struct[0].r = tempcolor[0];
	m_led_struct[0].g = tempcolor[1];
	m_led_struct[0].b = tempcolor[2];
	m_led_struct[3].r = tempcolor[0];
	m_led_struct[3].g = tempcolor[1];
	m_led_struct[3].b = tempcolor[2];
	
	m_led_struct[1].r = tempcolor[3];
	m_led_struct[1].g = tempcolor[4];
	m_led_struct[1].b = tempcolor[5];
	m_led_struct[4].r = tempcolor[3];
	m_led_struct[4].g = tempcolor[4];
	m_led_struct[4].b = tempcolor[5];
	
	m_led_struct[2].r = tempcolor[6];
	m_led_struct[2].g = tempcolor[7];
	m_led_struct[2].b = tempcolor[8];
	m_led_struct[5].r = tempcolor[6];
	m_led_struct[5].g = tempcolor[7];
	m_led_struct[5].b = tempcolor[8];

	set_flash_ws2812(m_led_struct, 6);
}


int main(void)
{
	//ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 6), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	
	// Insert system clock initialization code here (sysclk_init()).
	sysclk_init();

	/*
	nvm_eeprom_write_byte(10, 0x07);
	nvm_eeprom_write_byte(11, 0x05);
	nvm_eeprom_write_byte(12, 0x92);
	nvm_eeprom_write_byte(13, 0x24);
	nvm_eeprom_write_byte(14, 0x33);
	nvm_eeprom_write_byte(15, 0x33);
	
	nvm_eeprom_write_byte(16, 0x05);
	nvm_eeprom_write_byte(17, 0x06);
	nvm_eeprom_write_byte(18, 0x07);
	nvm_eeprom_write_byte(19, 0x01);
	nvm_eeprom_write_byte(20, 0x04);
	nvm_eeprom_write_byte(21, 0x03);
	nvm_eeprom_write_byte(22, 0x02);
	nvm_eeprom_write_byte(23, 0x01);
	nvm_eeprom_write_byte(24, 0x00);
	nvm_eeprom_write_byte(25, 0x09);
	nvm_eeprom_write_byte(26, 0x0a);
	nvm_eeprom_write_byte(27, 0x0b);
	nvm_eeprom_write_byte(28, 0x0c);
	
	nvm_eeprom_write_byte(29, 0x03);
	nvm_eeprom_write_byte(30, 0x06);
	nvm_eeprom_write_byte(31, 0x0d);
	nvm_eeprom_write_byte(32, 0x04);
	nvm_eeprom_write_byte(33, 0x02);
	nvm_eeprom_write_byte(34, 0x1e);
	nvm_eeprom_write_byte(35, 0x1d);
	nvm_eeprom_write_byte(36, 0x1c);
	nvm_eeprom_write_byte(37, 0x07);
	nvm_eeprom_write_byte(38, 0x0e);
	nvm_eeprom_write_byte(39, 0x0f);
	nvm_eeprom_write_byte(40, 0x10);
	nvm_eeprom_write_byte(41, 0x05);
	nvm_eeprom_write_byte(42, 0x08);
	nvm_eeprom_write_byte(43, 0x22);
	nvm_eeprom_write_byte(44, 0x21);
	nvm_eeprom_write_byte(45, 0x20);
	nvm_eeprom_write_byte(46, 0x1f);
	
	nvm_eeprom_write_byte(47, 0x07);
	nvm_eeprom_write_byte(48, 0x1d);
	
	nvm_eeprom_write_byte(49, 0x0a);
	nvm_eeprom_write_byte(50, 0x27);
	
	nvm_eeprom_write_byte(51, 0x01);
	nvm_eeprom_write_byte(52, 0x08);
	nvm_eeprom_write_byte(53, 0x96);
	nvm_eeprom_write_byte(54, 0x03);
	nvm_eeprom_write_byte(55, 0x03);
	nvm_eeprom_write_byte(56, 0x00);
	
	nvm_eeprom_write_byte(57, 0xA0);
	nvm_eeprom_write_byte(58, 0x96);
	nvm_eeprom_write_byte(59, 0x96);
	nvm_eeprom_write_byte(60, 0x46);
	nvm_eeprom_write_byte(61, 0xc8);
	nvm_eeprom_write_byte(62, 0x00);
	nvm_eeprom_write_byte(63, 0xf4);
	nvm_eeprom_write_byte(64, 0x01);
	nvm_eeprom_write_byte(65, 0x78);
	nvm_eeprom_write_byte(66, 0x03);
	nvm_eeprom_write_byte(67, 0x5e);
	nvm_eeprom_write_byte(68, 0x01);
	nvm_eeprom_write_byte(69, 0x80);
	
	nvm_eeprom_write_byte(70, 0x40);
	nvm_eeprom_write_byte(71, 0x1f);
	nvm_eeprom_write_byte(72, 0x00);
	nvm_eeprom_write_byte(73, 0x00);
	nvm_eeprom_write_byte(74, 0x01);
	nvm_eeprom_write_byte(75, 0x81);
	nvm_eeprom_write_byte(76, 0x01);
	*/
	
		/* Charging and watchdog timer - Using two compare channels with single timer */
	#ifdef ENABLE_WDT
		wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_2KCLK);
		wdt_reset();
		wdt_enable();
		init_timerd5();
	#endif
	
	tc45_disable(&TCC4);
	tc45_enable(&TCC5);
	
	init_ws2812(IOPORT_CREATE_PIN(PORTC, 6));
	clear_led();
	
	// init the mtch 6301
	init_mtchio();
	init_i2c();
	init_mtch6301();
	
	write_mtch_settings();
	
	/*
	//rainbow();
	if (EEPROM_MTCH_UPDATE_BOOL) {
		if (write_mtch_settings()) {
			restore_led_from_eeprom();
		}
		else {
			twinkle(255, 0, 0);
		}
	}
	*/
	//board_init();
//	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 3), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);

	init_uart();
	
	init_chargingpin(IOPORT_CREATE_PIN(PORTR, 1));
	/*
	volatile uint8_t eep0 = nvm_eeprom_read_byte(0);
	volatile uint8_t eep1 = nvm_eeprom_read_byte(1);
	volatile uint8_t eep2 = nvm_eeprom_read_byte(2);
	volatile uint8_t eep3 = nvm_eeprom_read_byte(3);
	volatile uint8_t eep4 = nvm_eeprom_read_byte(4);
	volatile uint8_t eep5 = nvm_eeprom_read_byte(5);
	volatile uint8_t eep6 = nvm_eeprom_read_byte(6);
	volatile uint8_t eep7 = nvm_eeprom_read_byte(7);
	volatile uint8_t eep8 = nvm_eeprom_read_byte(8);
	volatile uint8_t eep9 = nvm_eeprom_read_byte(9);
	*/
		
	// restore the state stored in the eeprom
	// restore_state_eep();
	
	
	/*
	volatile uint8_t eep0 = EEPROM_GENERAL_UPDATE_BOOL	;
	volatile uint8_t eep1 = EEPROM_GENERAL_UPDATE_BOOL	;
	volatile uint8_t eep2 = EEPROM_RXMAP_UPDATE_BOOL	;
	volatile uint8_t eep3 = EEPROM_TXMAP_UPDATE_BOOL	;
	volatile uint8_t eep4 = EEPROM_SELF_UPDATE_BOOL		;
	volatile uint8_t eep5 = EEPROM_MUTUAL_UPDATE_BOOL	;
	volatile uint8_t eep6 = EEPROM_DECODING_UPDATE_BOOL	;
	volatile uint8_t eep7 = EEPROM_GESTURES_UPDATE_BOOL	;
	volatile uint8_t eep8 = EEPROM_CONFIG_UPDATE_BOOL	;
	volatile uint8_t eep9 = EEPROM_MTCH_UPDATE_BOOL		;
	
	
	nvm_eeprom_write_byte(EEPROM_INDEX_GENERAL, 1);
	nvm_eeprom_write_byte(EEPROM_INDEX_RXMAP, 1);
	nvm_eeprom_write_byte(EEPROM_INDEX_TXMAP, 1);
	nvm_eeprom_write_byte(EEPROM_INDEX_SELF, 1);
	nvm_eeprom_write_byte(EEPROM_INDEX_MUTUAL, 1);
	nvm_eeprom_write_byte(EEPROM_INDEX_DECODING, 1);
	nvm_eeprom_write_byte(EEPROM_INDEX_GESTURES, 1);
	nvm_eeprom_write_byte(EEPROM_INDEX_CONFIG, 1);
	nvm_eeprom_write_byte(EEPROM_INDEX_MTCH, 1);
	*/
	
	restore_led_from_eeprom();
	
	core_status = STATUS_CHARGE;
	change_adc_channel(core_status);
	flag_initcharge = true;
	init_adc(core_status);
	adc_enable_interrupt();
	init_timer5();
	init_timer4();		
		
	if /*(ischarging())
	{
		old_core_status = core_status;
		core_status = STATUS_CHARGE;
		change_adc_channel(core_status);
		flag_initcharge = true;
		tc45_disable(&TCC4);
		tc45_enable(&TCC5);
	}
	else*/
	(!ischarging())
	{
		old_core_status = core_status;
		core_status = STATUS_BATT_READ;
		change_adc_channel(core_status);
		tc45_disable(&TCC5);
		tc45_enable(&TCC4);
	//	usart_putchar(M_USART, 0x23);
	}
	


	
	// Insert application code here, after the board has been initialized.
	while(1)
	{

		if (latest_gesture) {
			//twinkle(255, 0, 255);
			switch (latest_gesture) {
				case PLAY:
					ioport_set_pin_high(IOPORT_CREATE_PIN(PORTA, 5));
					//twinkle(255, 0, 0);
					//set_temp_color(playPauseColor);
					_delay_ms(2000); //twinkle(0, 0, 0);
					ioport_set_pin_low(IOPORT_CREATE_PIN(PORTA, 5));
					break;
				case NEXT:
					ioport_set_pin_high(IOPORT_CREATE_PIN(PORTA, 6));
					//twinkle(255, 255, 0);
					//set_temp_color(forwardColor);
					_delay_ms(2000); //twinkle(0, 0, 0);
					ioport_set_pin_low(IOPORT_CREATE_PIN(PORTA, 6));
					break;
				case PREV:
					ioport_set_pin_high(IOPORT_CREATE_PIN(PORTA, 7));
					//twinkle(0, 255, 0);
					//set_temp_color(backwardColor);
					_delay_ms(2000); //twinkle(0, 0, 0);
					ioport_set_pin_low(IOPORT_CREATE_PIN(PORTA, 7));
					break;
				case VOLUP:
					ioport_set_pin_high(IOPORT_CREATE_PIN(PORTD, 6));
					//twinkle(0, 255, 255);
					//set_temp_color(volumeUpColor);
					_delay_ms(2000); //twinkle(0, 0, 0);
					ioport_set_pin_low(IOPORT_CREATE_PIN(PORTD, 6));
					break;
				case VOLDOWN:
					ioport_set_pin_high(IOPORT_CREATE_PIN(PORTR, 0));
					//twinkle(0, 0, 255);
					//set_temp_color(volumeDownColor);
					_delay_ms(2000); //twinkle(0, 0, 0);
					ioport_set_pin_low(IOPORT_CREATE_PIN(PORTR, 0));
					break;
				case CONNDISC:
					ioport_set_pin_high(IOPORT_CREATE_PIN(PORTD, 0));
					//twinkle(255, 0, 255);
					//set_temp_color(connDiscColor);
					_delay_ms(2000); //twinkle(0, 0, 0);
					ioport_set_pin_low(IOPORT_CREATE_PIN(PORTD, 0));
					break;
				case INQUIRE:
					ioport_set_pin_high(IOPORT_CREATE_PIN(PORTD, 1));
					//twinkle(255, 255, 255);
					//set_temp_color(inquiryColor);
					_delay_ms(2000); //twinkle(0, 0, 0);
					ioport_set_pin_low(IOPORT_CREATE_PIN(PORTD, 1));
					break;
				default:
				break;
			}
			latest_gesture = NONE;
			//twinkle(0, 0, 0);
		}
	#ifdef NEW_UART_HANDLE	
	        uart_Task();
        #endif
	
		if (ischarging()) 
		{
			if (core_status != STATUS_CHARGE)
			{
				twinkle(0, 0, 255);
				shutdown_received = false;
				old_core_status = core_status;
				core_status = STATUS_CHARGE;
				change_adc_channel(core_status);
				flag_initcharge = true;
				tc45_disable(&TCC4);
				tc45_enable(&TCC5);
			}
		}
		else 	
		{
			if (core_status == STATUS_CHARGE) {
				old_core_status = core_status;
				core_status = STATUS_BATT_READ;
				change_adc_channel(core_status);
				tc45_disable(&TCC5);
				tc45_enable(&TCC4);
				load_data_color_led();
			}
			
			if (get_pulse_state() && core_status != STATUS_FFT_L && core_status != STATUS_FFT_R) {
				trigger_pulse_state_changed();
			}
			
			if (shutdown_received) {
				old_core_status = core_status;
				core_status = STATUS_NULL;
				tc45_disable(&TCC4);
				save_led_to_eeprom();
				twinkle(0, 0, 0);
				//turn off watch dog.
				wdt_disable();
				cpu_irq_disable();
				while(1);
			}
			else if (get_and_clear_pulse_state_changed()) {
				if (get_pulse_state()) {
					// If new message is to turn on visualization and if it is already on then ignore it
					
					// Initialize the ADC by changing to correct channel
					fht_flag = true;
					
					old_core_status = core_status;
					core_status = STATUS_FFT_L;
					change_adc_channel(core_status);
					// Switch on visualization interrupt
					tc45_enable(&TCC4);
					// save state to CSR- the fact that visualization is on
					write_byte_eeprom(VISUALIZATION_STATE_BYTE, 0x01);
					// respond to CSR
					//send_response(new_message_ID, 0x01);
				}
				else {
					fht_flag = false;
					old_core_status = core_status;
					core_status = STATUS_BATT_READ;
					change_adc_channel(core_status);
					clear_led();
					// save the state to eeprom (Clear visualization was on)
					write_byte_eeprom(VISUALIZATION_STATE_BYTE, 0x00);
					// Update LED from led_color_buf_data to last saved state
					load_data_color_led();
					//send_response(new_message_ID, 0x01);
				}
			}
			else {
				// if the fht led function was turned on
				if (fht_flag == true&&tempPulseDisabled==false)
				{
					if (flag_fft == true)
					{
						
						if (core_status == STATUS_FFT_L)
						{
							//	usart_putchar(M_USART, 0x55);

							run();
							old_core_status = core_status;
							core_status = STATUS_FFT_R;
							change_adc_channel(core_status);
							tc45_enable(&TCC4);
						}
						else if (core_status == STATUS_FFT_R)
						{
							//	usart_putchar(M_USART, 0x66);

							run();
							
							if (ischarging())
							{
								old_core_status = core_status;
								core_status = STATUS_CHARGE;
								change_adc_channel(core_status);
								flag_initcharge = true;
								tc45_disable(&TCC4);
								tc45_enable(&TCC5);
							}
							else
							{
								old_core_status = core_status;
								core_status = STATUS_FFT_L;
								change_adc_channel(core_status);
								tc45_enable(&TCC4);
							}
							
						}
						
						flag_fft = false;
					}
					
				}
				else {
					//load_data_color_led();
				}
			}
		}

	#ifdef ENABLE_WDT
		if (flag_timerd5) {
			flag_timerd5 = false;
			wdt_reset(); 
		}
      #endif			
		
		
		
		/*
//		detect_led();
		if (ischarging())
		{
			if (core_status != STATUS_CHARGE)
			{
				core_status = STATUS_CHARGE;
				change_adc_channel(core_status);
				tc45_enable(&TCC4);
				tc45_enable(&TCC5);
			}			
		} 
		else
		{
			if (new_message_exists())
			{
				// Check new message against existing LED_STATUS - if it is the exact same ignore
				new_message_ID = get_message_ID();
				
				switch(new_message_ID)
				{
					case VISUAL_ON_ID:
						// If new message is to turn on visualization and if it is already on then ignore it
						
						// Initialize the ADC by changing to correct channel
						fht_flag = true;
							
						core_status = STATUS_FFT_L;
						change_adc_channel(core_status);
						// Switch on visualization interrupt
						tc45_enable(&TCC4);
						// save state to CSR- the fact that visualization is on 
						write_byte_eeprom(VISUALIZATION_STATE_BYTE, 0x01);
						// respond to CSR 
						send_response(new_message_ID, 0x01);						
						break;
					case LED_COLOR_ID:
						// if current state is music visualization 
						// turn off fft interrupt 
						fht_flag = false;
						
						core_status = STATUS_NULL;						
						tc45_disable(&TCC4);
						clear_led();
						
						// update the LED's 
						load_buff_data_color_led();
						// save the state to EEPROM (including the latest LED colors) 
				//		nvm_eeprom_erase_and_write_buffer((uint16_t)EEPROM_LED1_R_BYTE, &buffer_data[2], 9);
						write_byte_eeprom(1, buffer_data[2]);
						write_byte_eeprom(2, buffer_data[3]);
						write_byte_eeprom(3, buffer_data[4]);
						
						write_byte_eeprom(4, buffer_data[5]);
						write_byte_eeprom(5, buffer_data[6]);
						write_byte_eeprom(6, buffer_data[7]);
						
						write_byte_eeprom(7, buffer_data[8]);
						write_byte_eeprom(8, buffer_data[9]);
						write_byte_eeprom(9, buffer_data[10]);
						// respond to CSR 
						send_response(new_message_ID, 0x01);
						break;
					case VISUAL_OFF_ID:
						core_status = STATUS_NULL;
						tc45_disable(&TCC4);
						clear_led();
						// save the state to eeprom (Clear visualization was on) 
						write_byte_eeprom(VISUALIZATION_STATE_BYTE, 0x00);
						// Update LED from led_color_buf_data to last saved state 
						restore_led_from_eeprom();
						send_response(new_message_ID, 0x01);
						break;
					case VBAT_ID:
						// DO NOT UPDATE FIRMWARE- HEADPHONES ARE NOT CHARGING 
						send_response(new_message_ID, 0x00);
						break;
					case LIGHT_ID:
						tc45_disable(&TCC4);
						
						old_core_status = core_status;
						core_status = STATUS_LIGHT;
						
						adc_flush(&ADCA);
						change_adc_channel(core_status);
						adc_start_conversion(&ADCA, ADC_CH0);
				
						while(flag_light == false);
						flag_light = false;
					
						send_light_data();
						
						core_status = old_core_status;
						change_adc_channel(core_status);						
						if ((core_status == STATUS_FFT_L) ||(core_status == STATUS_FFT_R))
						{
							tc45_enable(&TCC4);
						}						
						
						send_response(new_message_ID, 0x01);
						break;
					default:
						break;
				}
			} 
			else
			{
				// if the fht led function was turned on 
				if (fht_flag == true)
				{
					if (flag_fft == true)
					{
						if (core_status == STATUS_FFT_L)
						{
							//	usart_putchar(M_USART, 0x55);

							run();
							core_status = STATUS_FFT_R;
							change_adc_channel(core_status);
							tc45_enable(&TCC4);
						}
						else if (core_status == STATUS_FFT_R)
						{
							//	usart_putchar(M_USART, 0x66);

							run();
							if (ischarging())
							{
								core_status = STATUS_CHARGE;
								change_adc_channel(core_status);
								tc45_enable(&TCC4);
								tc45_enable(&TCC5);
							}
							else
							{
								core_status = STATUS_FFT_L;
								change_adc_channel(core_status);
								tc45_enable(&TCC4);
							}
							
						}
						flag_fft = false;
					}
				}
				
			}		
			
		}*/		
	}
}
