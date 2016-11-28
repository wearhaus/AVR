/*
 * config.h
 *
 * Created: 2014/8/6 18:35:03
 *  Author: Harifun
 */ 


#ifndef CONFIG_APP_H_
#define CONFIG_APP_H_

/*
	FFT config
	Fn = (n-1)Fs/FFT_N
*/
//#define LOG_OUT 	1		// use the log output function
//#define LIN_OUT8	1
//#define FFT_N		256		// set to 256 point ff

#define ENABLE_USARTD0
//#define ENABLE_USARTC0_REMAP

#define ENABLE_WDT
#define LIMIT_LOOP  //avoid loop too long time when problem happen.
#define NEW_UART_HANDLE  //a new way to handle uart data. This will not affect uart data receive.

/*
	WS2812 config
*/
#define DIN_CLR		ioport_set_pin_low(IOPORT_CREATE_PIN(PORTC, 6))
#define DIN_SET		ioport_set_pin_high(IOPORT_CREATE_PIN(PORTC, 6))
#define DIN_WS2812	IOPORT_CREATE_PIN(PORTC, 6)

/*
	LED defination
*/
#define LED_NUMBER	6
#define M_LED0		m_led_struct[0]
#define M_LED1		m_led_struct[1]
#define M_LED2		m_led_struct[2]
#define M_LED3		m_led_struct[3]
#define M_LED4		m_led_struct[4]
#define M_LED5		m_led_struct[5]

/**
 * \internal
 * \brief TWI master module
 */
#define TWI_MASTER			TWIC

/**
 * \internal
 * \brief TWI data transfer rate
 */
#define TWI_SPEED			400000

/**
 * \internal
 * \brief TWI master address
 */
#define TWI_MASTER_ADDR		0x50

/*
	charging pin
*/
#define CHARGING_PIN		IOPORT_CREATE_PIN(PORTR, 1)

/*
	playing pin
*/
#define PLAYING_PIN			IOPORT_CREATE_PIN(PORTD, 1)

/*
	led status piority
*/
#define STATUS_LED_OFF		0x00
#define STATUS_LED_CHARGE	0x01
#define STATUS_LED_FFT		0x02
#define STATUS_LED_UART		0x03

/*
	adc status config
*/
#define STATUS_NULL			0x00
#define STATUS_CHARGE		0x01
#define STATUS_FFT_L		0x02
#define STATUS_FFT_R		0x04
#define STATUS_LIGHT_READ	0x05
#define STATUS_BATT_READ 	0X06

//ADC Channel for charging
#define ADCCH_POS_CHARGE	ADCCH_POS_PIN4
//ADC Channel for L FFT
#define ADCCH_POS_FFT_L		ADCCH_POS_PIN1
#define ADCCH_NEG_FFT_L		ADCCH_NEG_PIN0
//ADC Channel for R FFT
#define ADCCH_POS_FFT_R		ADCCH_POS_PIN2
#define ADCCH_NEG_FFT_R		ADCCH_NEG_PIN3
//ADC Channel for Light sensor
#define ADCCH_POS_LIGHT		ADCCH_POS_PIN12



/*
	uart config
*/
#define START_FLAG			0x0F
#define STOP_FLAG			0xF0

#define UART_OK				0xFF
#define UART_FAIL			0x0C

#define CHECK_OK			0x00

#define WRITE_EEPROM		0x00
#define UART_LENGTH			0
#define UART_CMD			1
#define UART_EEPROM_ADDR	2
#define UART_EEPROM_DATA	3

#define M_USART				&USARTC0

#define BUBBLE_NUM			10

/*
	eeprom defines
*/
#define VISUALIZATION_STATE_BYTE 		0
#define EEPROM_VISUALIZATION_STATE		nvm_eeprom_read_byte(VISUALIZATION_STATE_BYTE)

/* This is the bytes where color saved values for the LED's */
/* Need to default the bytes at these locations to make headphones glow rainbow - Unboxing*/

/* LED1 */
#define EEPROM_LED1_R_BYTE				nvm_eeprom_read_byte(1)
#define EEPROM_LED1_G_BYTE				nvm_eeprom_read_byte(2)
#define EEPROM_LED1_B_BYTE				nvm_eeprom_read_byte(3)

/* LED2 */
#define EEPROM_LED2_R_BYTE				nvm_eeprom_read_byte(4)
#define EEPROM_LED2_G_BYTE				nvm_eeprom_read_byte(5)
#define EEPROM_LED2_B_BYTE				nvm_eeprom_read_byte(6)

/* LED2 */
#define EEPROM_LED3_R_BYTE				nvm_eeprom_read_byte(7)
#define EEPROM_LED3_G_BYTE				nvm_eeprom_read_byte(8)
#define EEPROM_LED3_B_BYTE				nvm_eeprom_read_byte(9)

/* MTCH6301 Writeable registers - in order listed on datasheet */
//GENERAL - INDEX 0x00
#define EEPROM_NUMRXCHANNELS			nvm_eeprom_read_byte(10)
#define EEPROM_NUMTXCHANNELS			nvm_eeprom_read_byte(11)
#define EEPROM_RXSCALE_7_0				nvm_eeprom_read_byte(12)
#define EEPROM_RXSCALE_15_8				nvm_eeprom_read_byte(13)
#define EEPROM_TXSCALE_7_0				nvm_eeprom_read_byte(14)
#define EEPROM_TXSCALE_15_8				nvm_eeprom_read_byte(15)

//RXMAP - INDEX 0x01
#define EEPROM_RXPIN0					nvm_eeprom_read_byte(16)
#define EEPROM_RXPIN1					nvm_eeprom_read_byte(17)
#define EEPROM_RXPIN2					nvm_eeprom_read_byte(18)
#define EEPROM_RXPIN3 					nvm_eeprom_read_byte(19)
#define EEPROM_RXPIN4					nvm_eeprom_read_byte(20)
#define EEPROM_RXPIN5 					nvm_eeprom_read_byte(21)
#define EEPROM_RXPIN6 					nvm_eeprom_read_byte(22)
#define EEPROM_RXPIN7  					nvm_eeprom_read_byte(23)
#define EEPROM_RXPIN8  					nvm_eeprom_read_byte(24)
#define EEPROM_RXPIN9   				nvm_eeprom_read_byte(25)
#define EEPROM_RXPIN10					nvm_eeprom_read_byte(26) 
#define EEPROM_RXPIN11					nvm_eeprom_read_byte(27)  
#define EEPROM_RXPIN12					nvm_eeprom_read_byte(28)     

//TXMAP - INDEX 0x02
#define EEPROM_TXPIN0					nvm_eeprom_read_byte(29)
#define EEPROM_TXPIN1   				nvm_eeprom_read_byte(30)     
#define EEPROM_TXPIN2       			nvm_eeprom_read_byte(31)
#define EEPROM_TXPIN3      				nvm_eeprom_read_byte(32)
#define EEPROM_TXPIN4       			nvm_eeprom_read_byte(33)
#define EEPROM_TXPIN5       			nvm_eeprom_read_byte(34)
#define EEPROM_TXPIN6       			nvm_eeprom_read_byte(35)
#define EEPROM_TXPIN7       			nvm_eeprom_read_byte(36)
#define EEPROM_TXPIN8       			nvm_eeprom_read_byte(37)
#define EEPROM_TXPIN9       			nvm_eeprom_read_byte(38)
#define EEPROM_TXPIN10					nvm_eeprom_read_byte(39)
#define EEPROM_TXPIN11     				nvm_eeprom_read_byte(40)
#define EEPROM_TXPIN12					nvm_eeprom_read_byte(41)
#define EEPROM_TXPIN13      			nvm_eeprom_read_byte(42)
#define EEPROM_TXPIN14       			nvm_eeprom_read_byte(43)
#define EEPROM_TXPIN15        			nvm_eeprom_read_byte(44)
#define EEPROM_TXPIN16       			nvm_eeprom_read_byte(45)
#define EEPROM_TXPIN17     				nvm_eeprom_read_byte(46)

//SELF - INDEX 0x10
#define EEPROM_SELFSCANTIME				nvm_eeprom_read_byte(47)
#define EEPROM_SELFTOUCHTHRES			nvm_eeprom_read_byte(48)

//MUTUAL - INDEX 0x20
#define EEPROM_MUTSCANTIME				nvm_eeprom_read_byte(49)
#define EEPROM_MUTTOUCHTHRES			nvm_eeprom_read_byte(50)

//DECODING - INDEX 0x30
#define EEPROM_FLIPSTATE				nvm_eeprom_read_byte(51)
#define EEPROM_NUMAVG					nvm_eeprom_read_byte(52)
#define EEPROM_MINTOUCHDIST				nvm_eeprom_read_byte(53)
#define EEPROM_PENDOWNTIME				nvm_eeprom_read_byte(54)
#define EEPROM_PENUPTIME				nvm_eeprom_read_byte(55)
#define EEPROM_TOUCHSUPPRESSION			nvm_eeprom_read_byte(56)

//GESTURES - INDEX 0x50
#define EEPROM_RXSWIPELEN				nvm_eeprom_read_byte(57)
#define EEPROM_TXSWIPELEN				nvm_eeprom_read_byte(58)
#define EEPROM_SWIPEBOUNDARY			nvm_eeprom_read_byte(59)
#define EEPROM_SWIPEHOLDTHRES			nvm_eeprom_read_byte(60)
#define EEPROM_SWIPETIME_7_0   			nvm_eeprom_read_byte(61)
#define EEPROM_SWIPETIME_15_8  			nvm_eeprom_read_byte(62)
#define EEPROM_TAPTIME_7_0     			nvm_eeprom_read_byte(63)
#define EEPROM_TAPTIME_15_8   			nvm_eeprom_read_byte(64)
#define EEPROM_TAPTHRES        			nvm_eeprom_read_byte(65)
#define EEPROM_MINSWIPEVELOCITY			nvm_eeprom_read_byte(66)
#define EEPROM_DOUBLETIME_7_0   		nvm_eeprom_read_byte(67)
#define EEPROM_DOUBLETIME_15_8  		nvm_eeprom_read_byte(68)
#define EEPROM_EDGEKEEPOUT     			nvm_eeprom_read_byte(69)

//CONFIG - INDEX 0xf0
#define EEPROM_SLEEP_7_0       			nvm_eeprom_read_byte(70)
#define EEPROM_SLEEP_15_8      			nvm_eeprom_read_byte(71)
#define EEPROM_SLEEP_23_16     			nvm_eeprom_read_byte(72)
#define EEPROM_SLEEP_31_24     			nvm_eeprom_read_byte(73)
#define EEPROM_TOUCHPACKETCFG  			nvm_eeprom_read_byte(74)
#define EEPROM_GESTUREPACKETCFG			nvm_eeprom_read_byte(75)
#define EEPROM_STATUSPACKETCFG			nvm_eeprom_read_byte(76)

//INDEX UPDATED FLAGS
#define EEPROM_INDEX_GENERAL			77
#define EEPROM_INDEX_RXMAP				78
#define EEPROM_INDEX_TXMAP				79
#define EEPROM_INDEX_SELF				80
#define EEPROM_INDEX_MUTUAL				81
#define EEPROM_INDEX_DECODING			82
#define EEPROM_INDEX_GESTURES			83
#define EEPROM_INDEX_CONFIG				84
#define EEPROM_INDEX_MTCH				85

#define EEPROM_GENERAL_UPDATE_BOOL		nvm_eeprom_read_byte(EEPROM_INDEX_GENERAL)	
#define EEPROM_RXMAP_UPDATE_BOOL		nvm_eeprom_read_byte(EEPROM_INDEX_RXMAP)	
#define EEPROM_TXMAP_UPDATE_BOOL		nvm_eeprom_read_byte(EEPROM_INDEX_TXMAP)	
#define EEPROM_SELF_UPDATE_BOOL			nvm_eeprom_read_byte(EEPROM_INDEX_SELF)
#define EEPROM_MUTUAL_UPDATE_BOOL		nvm_eeprom_read_byte(EEPROM_INDEX_MUTUAL)	
#define EEPROM_DECODING_UPDATE_BOOL		nvm_eeprom_read_byte(EEPROM_INDEX_DECODING)
#define EEPROM_GESTURES_UPDATE_BOOL		nvm_eeprom_read_byte(EEPROM_INDEX_GESTURES)		
#define EEPROM_CONFIG_UPDATE_BOOL		nvm_eeprom_read_byte(EEPROM_INDEX_CONFIG)	
#define EEPROM_MTCH_UPDATE_BOOL			nvm_eeprom_read_byte(EEPROM_INDEX_MTCH)

// Command ID's for all messages
#define LED_COLOR_ID 0x00
#define VISUAL_ON_ID 0x01
#define VISUAL_OFF_ID 0x02
#define VBAT_ID 0x03
#define LIGHT_ID 0x04
#define NO_MESSAGE 0xFF

#endif /* CONFIG_H_ */