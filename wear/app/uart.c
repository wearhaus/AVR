/*
 * uart.c
 *
 * Created: 2014/8/6 20:49:44
 *  Author: Harifun
 */ 

#include "asf.h"
#include <config_app.h>
#include <eeprom.h>
#include <uart.h>
#include <wear.h>
#include <adc_app.h>

uint8_t rxmode=0,num_rx=0;
uint8_t uart_length = 6;
uint8_t rxdata[24];
bool uart_start_flag = false;
uint8_t count_uart = 0;
bool uart_done_flag = false;

/* 
   Buffer to load incoming UART data 
   The first byte shall be the COM_ID
   The second byte shall be the size of the payload
   The rest shall be data 
*/
volatile uint8_t buffer_data[11] = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128};
volatile uint8_t colors[9] = {255, 0, 0, 0, 255, 0, 0, 0, 255};
	
bool pulse_state = false;
bool pulse_state_changed = false;
bool shutdown_received = false;
	
void init_uart(void)
{
#ifdef ENABLE_USARTD0
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 3), IOPORT_DIR_OUTPUT	| IOPORT_INIT_HIGH);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 2), IOPORT_DIR_INPUT);
#endif

	// USART options.
	static usart_rs232_options_t usart_serial_options = {
		.baudrate = 38400,
		.charlength = USART_CHSIZE_8BIT_gc,
		.paritytype = USART_PMODE_DISABLED_gc,
		.stopbits = false
	};
	
	// Initialize usart driver in RS232 mode
#ifdef ENABLE_USARTD0
	usart_init_rs232(&USARTD0, &usart_serial_options);
#endif

	//Enable Recieve Complete Interrrupt
#ifdef ENABLE_USARTD0
	usart_set_rx_interrupt_level(&USARTD0, USART_INT_LVL_HI);
#endif

	pmic_enable_level(PMIC_LVL_HIGH);
	cpu_irq_enable();
}

void uart_send_status(uint8_t status_uart)
{
	usart_putchar(M_USART, status_uart);
}

void uart_start(void)
{
	uart_start_flag = true;
}

void uart_stop(void)
{
	uart_start_flag = false;
}

uint8_t uart_getflag(void)
{
	return uart_start_flag;
}

static inline uint8_t uart_check(uint8_t * rxvalue)
{
	uint8_t sum = 0;
	
	for (uint8_t i = 0; i < rxvalue[0]; i++)
	{
		sum += rxvalue[i];
	}
	
	return sum;
}

void uart_clear(void)
{
	num_rx = 0;
}

static inline void uart_decode(uint8_t * rxvalue)
{
	switch(rxvalue[UART_CMD])
	{
		case WRITE_EEPROM:
			uart_send_status(write_byte_eeprom(rxvalue[UART_EEPROM_ADDR], rxvalue[UART_EEPROM_DATA]));
			break;
		default:
			break;
	}
}

void uart_check_cmd(uart_package_t * my_uart_command)
{
	uint8_t sum = 0;
	
	for (uint8_t i = 0; i < (my_uart_command->length-3); i++)
	{
		sum += my_uart_command->data[i];
	}
	
	sum += my_uart_command->length;
	sum += my_uart_command->command;
	
	my_uart_command->check = 256-sum;
}

void uart_send_command(uart_package_t * my_uart_command)
{
	usart_putchar(M_USART, my_uart_command->start);
	usart_putchar(M_USART, my_uart_command->length);
	usart_putchar(M_USART, my_uart_command->command);
	for (uint8_t i; i<(my_uart_command->length-3);i++)
	{
		usart_putchar(M_USART, my_uart_command->data[i]);
	}
	usart_putchar(M_USART, my_uart_command->check);
	usart_putchar(M_USART, my_uart_command->stop);
}

void test_uart(void)
{
	uart_package_t * m_uart_command;
	
	m_uart_command->start = 0x0F;
	m_uart_command->stop = 0xF0;
	m_uart_command->length = 4;
	m_uart_command->command = 0;
	m_uart_command->data[0] = 1;
	
	uart_check_cmd(m_uart_command);
	uart_send_command(m_uart_command);
}

/*
	���ݸ�ʽ����һ������Ϊ  0x0F�����һ��ȷ������Ϊ   0xF0
	��ʼ֡	���ݳ���	�����־	���ݰ�	У��	����֡
	
	���ݳ��� = ���������峤�� - ��ʼ֡ - ����֡
	
	У�飺 ���ݳ���+�����־+���ݰ�+У��=00
	
	0F 04 00 01 FB F0
*/
static inline void uart_protocal(uint8_t rxvalue)
{
	if (num_rx == 0)
	{
		if (rxvalue == 0x0F)
		{
			num_rx ++;
			uart_length = 6;
			uart_start();
		} 
		else
		{
			uart_clear();
		}
	} 
	else if (num_rx == uart_length+1)
	{
		uart_clear();
		
		if ((rxvalue == 0xF0) && (uart_check(rxdata) == CHECK_OK))
		{
			uart_stop();
			uart_send_status(UART_OK);
		} 
		else
		{
			uart_stop();
			uart_send_status(UART_FAIL);
		}
	} 
	else
	{
		rxdata[num_rx-1] = rxvalue;
		num_rx ++;
		
		if (uart_length != rxdata[UART_LENGTH])
		{
			uart_length = rxdata[UART_LENGTH];
		}
	}	
}

static inline void set_color_from_buffer(void) {
	m_led_struct[0].r = buffer_data[2];
	m_led_struct[0].g = buffer_data[3];
	m_led_struct[0].b = buffer_data[4];
	m_led_struct[3].r = buffer_data[2];
	m_led_struct[3].g = buffer_data[3];
	m_led_struct[3].b = buffer_data[4];
	
	m_led_struct[1].r = buffer_data[5];
	m_led_struct[1].g = buffer_data[6];
	m_led_struct[1].b = buffer_data[7];
	m_led_struct[4].r = buffer_data[5];
	m_led_struct[4].g = buffer_data[6];
	m_led_struct[4].b = buffer_data[7];
	
	m_led_struct[2].r = buffer_data[8];
	m_led_struct[2].g = buffer_data[9];
	m_led_struct[2].b = buffer_data[10];
	m_led_struct[5].r = buffer_data[8];
	m_led_struct[5].g = buffer_data[9];
	m_led_struct[5].b = buffer_data[10];
	
	for (int i=0; i<9; i++) {
		colors[i] = buffer_data[i+2];
		nvm_eeprom_write_byte(i+1, colors[i]);
	}
	
	if (!ischarging()) {
		set_flash_ws2812(m_led_struct, 6);
	}
}

void led_set_from_colors(void) {
	m_led_struct[0].r = colors[0];
	m_led_struct[0].g = colors[1];
	m_led_struct[0].b = colors[2];
	m_led_struct[3].r = colors[0];
	m_led_struct[3].g = colors[1];
	m_led_struct[3].b = colors[2];
	
	m_led_struct[1].r = colors[3];
	m_led_struct[1].g = colors[4];
	m_led_struct[1].b = colors[5];
	m_led_struct[4].r = colors[3];
	m_led_struct[4].g = colors[4];
	m_led_struct[4].b = colors[5];
	
	m_led_struct[2].r = colors[6];
	m_led_struct[2].g = colors[7];
	m_led_struct[2].b = colors[8];
	m_led_struct[5].r = colors[6];
	m_led_struct[5].g = colors[7];
	m_led_struct[5].b = colors[8];
	
	for (int i=0; i<9; i++) {
		nvm_eeprom_write_byte(i+1, colors[i]);
	}
	
	if (!ischarging()) {
		set_flash_ws2812(m_led_struct, 6);
	}
}

static inline void set_temp_color(uint8_t* tempcolor) {
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

static inline void set_pulse_from_buffer(void) {
	switch (buffer_data[2]) {
		case 0x00:
			pulse_state = false;
			pulse_state_changed = true;
			break;
		case 0x01:
			pulse_state = true;
			pulse_state_changed = true;
			break;
		default:
			pulse_state = true;
			pulse_state_changed = true;
			break;
	}
}

uint8_t* get_current_colors(void) {
	return colors;
}

bool get_pulse_state(void) {
	return pulse_state;
}

bool get_and_clear_pulse_state_changed(void) {
	bool changed = pulse_state_changed;
	pulse_state_changed = false;
	return changed;
}

void trigger_pulse_state_changed(void) {
	pulse_state_changed = true;
}

static void interpret_message(void) {
	switch(buffer_data[0]) {
		case UART_SET_COLOR:
			send_response(UART_SET_COLOR, 0xff);
			set_color_from_buffer();
			break;
			
		case UART_SET_PULSE:
			send_response(UART_SET_PULSE, 0xff);
			set_pulse_from_buffer();
			break;
			
		case UART_SET_SHUTDOWN:
			if (pulse_state) {
				pulse_state = false;
				pulse_state_changed = true;	
			}
			shutdown_received = true;
			send_response(UART_SET_SHUTDOWN, 0xff);
			break;
			
		case UART_SET_CHG_LVL:
			switch (buffer_data[2]) {
				case 0:
					chargeLVL0 = buffer_data[3];
					break;
					
				case 1:
					chargeLVL1 = buffer_data[3];
					break;
					
				case 2:
					chargeLVL2 = buffer_data[3];
					break;
					
				case 3:
					chargeLVL3 = buffer_data[3];
					break;
					
				case 4:
					chargeLVL4 = buffer_data[3];
					break;
					
				case 5:
					chargeLVL5 = buffer_data[3];
					break;
					
				case 6:
					chargeLVL6 = buffer_data[3];
					break;
			}
			break;
			
		case UART_SET_CHG_BRIGHT:
			chargeBrightness = buffer_data[2];
			break;
			
		case UART_SET_LOW_DIVIDER:
			LOW_DIVIDER = buffer_data[2];
			break;
			
		case UART_SET_MID_DIVIDER:
			MID_DIVIDER = buffer_data[2];
			break;
			
		case UART_SET_HIGH_DIVIDER:
			HIGH_DIVIDER = buffer_data[2];
			break;
			
		case UART_GET_AMBIENT:
			send_light_data();
			break;
			
		case UART_GET_BATTERY:
			send_battery_data();
			break;
			
		case UART_GET_PULSE:
			send_pulse_data();
			break;
			
		case UART_GET_COLOR:
			send_color_data();
			break;
			
		case UART_GET_CHARGING:
			send_charging_data();
			break;		
		
		default:
			break;
	}
	return;
}

/*
brief RX complete interrupt service routine.
*/
ISR(USARTC0_RXC_vect)
{
	//twinkle(255, 0, 0);
	uart_protocal(usart_getchar(&USARTC0));
}

/*
brief RX complete interrupt service routine.
*/
ISR(USARTD0_RXC_vect)
{
	// twinkle(buffer_data[0], buffer_data[1], buffer_data[2]);
	for (int i=0; i<11; i++) {
		buffer_data[i] = usart_getchar(&USARTD0);
	}
	
	interpret_message();
	
	uart_done_flag = true;
	
	//twinkle(usart_getchar(&USARTD0), usart_getchar(&USARTD0), usart_getchar(&USARTD0));
//	usart_putchar(&USARTD0, 0x0f);
	
	/* 11 bytes sent everytime, with every command
	   Fault tolerant against hanging as watchdog will reset the
	   ATMEL*/
	/*
	if (ischarging())
	{
		usart_getchar(&USARTD0);;
		return;
	}
	
	else
	{
		if (buffer_data[0] == LED_COLOR_ID)
		{
			buffer_data[count_uart ++] = usart_getchar(&USARTD0);
	//		usart_putchar(&USARTD0, buffer_data[count_uart - 1]);
			if (count_uart > 10)
			{
				count_uart = 0;
				uart_done_flag = true;
			}			
		}
		else
		{
			buffer_data[0] = usart_getchar(&USARTD0);
	//		usart_putchar(&USARTD0, buffer_data[0]);
			count_uart = 1;
			if (buffer_data[0] != LED_COLOR_ID)
			{
				uart_done_flag = true;
			}			
		}
		while(count_uart < 11)
		{
			while(!(USARTD0_STATUS & USART_RXCIF_bm));
			
			buffer_data[count] = USARTD0_DATA;
			count_uart = count_uart + 1;
			usart_putchar(&USARTD0, buffer_data[0]);
			usart_putchar(&USARTD0, count_uart);
		}
		}
	*/
}

void uart_send_bytes(char * byte_array, unsigned int len)
{
	for (int i = 0; i < len; i++){
		usart_putchar(&USARTD0, byte_array[i]);
	}
}

bool new_message_exists(void){

/*	if (buffer_data[0] == 128){
		return false;
	}
	else return true;*/
	if (uart_done_flag == true)
	{
		uart_done_flag = false;
		return true;
	} 
	else
	{
		return false;
	}
}

uint8_t get_message_ID(){

	uint8_t ID;

	if (!new_message_exists){
		return 0xFF;
	}
	else{
		ID = buffer_data[0];
		/* Once read, clear it */
		buffer_data[0] = 128;
		return ID;
	}

}
