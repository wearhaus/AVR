/*
 * uart.h
 *
 * Created: 2014/8/6 20:49:28
 *  Author: Harifun
 */ 


#ifndef UART_H_
#define UART_H_

typedef struct
{
	//! Touch ID
	uint8_t start;
	//! TCH
	uint8_t length;
	//! PEN
	uint8_t command;
	//! X Coordinate
	uint8_t data[5];
	//! Y Coordinate
	uint8_t check;
	//! Gesture
	uint8_t stop;
} uart_package_t;

#define UART_SET_COLOR			0x43
#define UART_SET_PULSE			0x50
#define UART_SET_RESTART		0x52
#define UART_SET_SHUTDOWN		0x01

#define UART_SET_CHG_LVL		0x02
#define UART_SET_CHG_BRIGHT		0x03
#define UART_SET_LOW_DIVIDER	0x04
#define UART_SET_MID_DIVIDER	0x05
#define UART_SET_HIGH_DIVIDER	0x06
#define UART_SET_TEMP_COLOR     0x07

#define UART_GET_COLOR			0x63
#define UART_GET_PULSE			0x70
#define UART_GET_BATTERY		0x42
#define UART_GET_AMBIENT		0x4d
#define UART_GET_CHARGING		0x47	

#define UART_SET_MTCH			0x54
#define UART_GET_MTCH			0x48

#define UART_WRITE_MTCH			0x57

#define UART_COLOR_LEN      9
#define UART_PULSE_LEN      1
#define UART_LVL_LEN		2
#define UART_BRIGHT_LEN		1
#define UART_DIV_LEN		1
#define UART_TEMP_LEN       1
#define UART_SHUTDOWN_LEN   0


void init_uart(void);
void uart_send_status(uint8_t status_uart);
void uart_start(void);
void uart_stop(void);
uint8_t uart_getflag(void);
void uart_clear(void);

void uart_send_bytes(char * byte_array, unsigned int len);
bool new_message_exists(void);
/* only call if new_message_exists returns true */
uint8_t get_message_ID();
uint8_t* get_current_colors(void);
bool get_pulse_state(void);
bool get_and_clear_pulse_state_changed(void);
void trigger_pulse_state_changed(void);
void led_set_from_colors(void);
void uart_Task(void);

extern volatile uint8_t buffer_data[13];
extern volatile uint8_t colors[9];
extern bool shutdown_received;
extern bool uart_done_flag;
extern bool tempPulseDisabled;

#endif /* UART_H_ */