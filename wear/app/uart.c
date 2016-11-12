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
#include <mtch6301.h>
#include <timer_app.h>

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
volatile uint8_t buffer_data[13] = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,0,0};
volatile uint8_t colors[9] = {255, 0, 0, 0, 255, 0, 0, 0, 255};
	
bool pulse_state = false;
bool pulse_state_changed = false;
bool shutdown_received = false;
	
bool tempPulseDisabled=false;


#define UART_BUFF_SIZE 128
typedef struct{
unsigned char  uart_RxBuff[UART_BUFF_SIZE];
unsigned char  headp;
unsigned char  tailp;
}UartRxBuffStruct;

UartRxBuffStruct g_uartBuff;

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

	g_uartBuff.headp=0;
	g_uartBuff.tailp=0;
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
	数据格式：第一个数字为  0x0F，最后一个确认数字为   0xF0
	起始帧	数据长度	命令标志	数据包	校验	结束帧
	
	数据长度 = 数据流整体长度 - 起始帧 - 结束帧
	
	校验： 数据长度+命令标志+数据包+校验=00
	
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

static inline void set_mtch_register_from_buffer(void) {
	cmd_write_register(buffer_data[2], buffer_data[3], buffer_data[4]);
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
	#ifdef LIMIT_LOOP //only handle correct format	
			if(buffer_data[1]!=UART_COLOR_LEN)
				break;
	#endif	
			send_response(UART_SET_COLOR, 0xff);
			set_color_from_buffer();
			resetDisablePulseCount();
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
			
		case UART_SET_MTCH:
			nvm_eeprom_write_byte(buffer_data[2], buffer_data[3]);
			
			if (buffer_data[2] <= 15) {
				nvm_eeprom_write_byte(EEPROM_INDEX_GENERAL, 1);
			}
			else if (buffer_data[2] <= 28) {
				nvm_eeprom_write_byte(EEPROM_INDEX_RXMAP, 1);
			}
			else if (buffer_data[2] <= 46) {
				nvm_eeprom_write_byte(EEPROM_INDEX_TXMAP, 1);
			}
			else if (buffer_data[2] <= 48) {
				nvm_eeprom_write_byte(EEPROM_INDEX_SELF, 1);
			}
			else if (buffer_data[2] <= 50) {
				nvm_eeprom_write_byte(EEPROM_INDEX_MUTUAL, 1);
			}
			else if (buffer_data[2] <= 56) {
				nvm_eeprom_write_byte(EEPROM_INDEX_DECODING, 1);
			}
			else if (buffer_data[2] <= 69) {
				nvm_eeprom_write_byte(EEPROM_INDEX_GESTURES, 1);
			}
			else if (buffer_data[2] <= 76) {
				nvm_eeprom_write_byte(EEPROM_INDEX_CONFIG, 1);
			}
			nvm_eeprom_write_byte(EEPROM_INDEX_MTCH, 1);
			break;
			
		case UART_GET_MTCH:
			send_mtch_update_status();
			break;
			
		case UART_SET_RESTART:
			while(1) {barrier();}
			break;
			
		case UART_WRITE_MTCH:
			write_mtch_settings();
			send_mtch_update_status();
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


unsigned char uartCmdValid(unsigned char cmd)
{
    unsigned char retval=false;
    switch(cmd){
		case UART_SET_COLOR:
		case UART_SET_PULSE:
		case UART_SET_SHUTDOWN:
		case UART_SET_CHG_LVL:	
		case UART_SET_CHG_BRIGHT:	
		case UART_SET_LOW_DIVIDER:	
		case UART_SET_MID_DIVIDER:
		case UART_SET_HIGH_DIVIDER:
		case UART_GET_AMBIENT:
		case UART_GET_BATTERY:
		case UART_GET_PULSE:
		case UART_GET_COLOR:
		case UART_GET_CHARGING:
		case UART_SET_MTCH:
		case UART_GET_MTCH:
		case UART_SET_RESTART:
		case UART_WRITE_MTCH:
			retval=true;
			break;		
		
		default:
			break;
	}
	return retval;

}

/*
brief RX complete interrupt service routine.
*/
void uart_buffPosInc(unsigned char *p)
{
    if(*p<UART_BUFF_SIZE-1)
  	(*p)++;
    else
  	(*p)=0;
}

void uart_buffPosChange(unsigned char *p,unsigned char num)
{
    if(*p+num<=UART_BUFF_SIZE-1)
  	*p+=num;
    else
    {
  	*p+=num;
	*p-=UART_BUFF_SIZE;
    }
}

unsigned char uart_buffGetData(unsigned char p,unsigned char shift)
{
    if(p+shift<=UART_BUFF_SIZE-1)
  	return g_uartBuff.uart_RxBuff[p+shift];
    else
    {
  	p+=shift;
	return g_uartBuff.uart_RxBuff[p-UART_BUFF_SIZE];
    }

}


unsigned char uart_buffDataSize(void)
{
    unsigned char len;
    if(g_uartBuff.headp>=g_uartBuff.tailp)
    {
        len= g_uartBuff.headp-g_uartBuff.tailp;
    }
    else
    {
        len= g_uartBuff.headp+UART_BUFF_SIZE-g_uartBuff.tailp;
    }
    return len;	
}

//received data packet : cmd+ len + data
unsigned char uart_receivedData(void)
{
    unsigned char temp;

    //no data received.
    if(g_uartBuff.tailp==g_uartBuff.headp)
        return false;
	
    //sync with packet header and discard any invalid data.
    while(uartCmdValid(g_uartBuff.uart_RxBuff[g_uartBuff.tailp])==0)
    {
        uart_buffPosInc(&g_uartBuff.tailp);
    }

    //received enough data?
    temp=uart_buffDataSize();
   // if(temp > 2)
    if(temp >= 11)	//BT will send 11 bytes each time.	
    {
        //if(temp>=uart_buffGetData(g_uartBuff.tailp,1)+2)
           return true;
    }
	
    return false; 
} 

//handle the received data.
void uart_Task(void)
{
    unsigned char i,temp;
    if(uart_receivedData())
    {
        //temp=uart_buffGetData(g_uartBuff.tailp,1)+2;
	  temp=11; //BT will send 11 bytes each time.
	  for(i=0;i<temp;i++)
	  {
            buffer_data[i]=uart_buffGetData(g_uartBuff.tailp,0);
	      uart_buffPosInc(&g_uartBuff.tailp);
	  }
	   interpret_message();
    }
}


ISR(USARTD0_RXC_vect)
{
#if 1
    while(usart_rx_is_complete(&USARTD0) ) {
        g_uartBuff.uart_RxBuff[g_uartBuff.headp] =((uint8_t)(&USARTD0)->DATA);
	  uart_buffPosInc(&g_uartBuff.headp);
    }
#else     
	 unsigned int count=0;
	 unsigned char errflag=0;
	for (int i=0; i<11; i++) {
		while (usart_rx_is_complete(&USARTD0) == false) {
#ifdef LIMIT_LOOP //do not wait for ever.
               if(count++>10000)
	         {
		   	errflag=1;
		      break;
		   }
#endif		
	      }
		//do not read when error
		if(errflag)
			break;
	      buffer_data[i] =((uint8_t)(&USARTD0)->DATA);
	}
	
	// check data format. 
	if(uartCmdValid(buffer_data[0])&&(errflag==0||(errflag&&buffer_data[1]<9)))
	{
	interpret_message();
          errflag=0;
	}
	
	uart_done_flag = true;
	
#endif
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
