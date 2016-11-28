/*
 * wear.h
 *
 * Created: 2014/8/10 13:27:55
 *  Author: Harifun
 */ 


#ifndef WEAR_H_
#define WEAR_H_

#include <ws2812.h>



extern volatile uint8_t core_status;
extern volatile uint8_t led_status;
extern volatile uint8_t old_core_status;
extern int16_t value_charge;
extern int16_t value_light;
extern int16_t value_batt;

extern volatile uint8_t flag_light;
extern volatile uint8_t flag_charge;
extern volatile uint8_t flag_batt;
extern volatile uint8_t flag_initcharge;

extern volatile uint16_t chargeLVL0;
extern volatile uint16_t chargeLVL1;
extern volatile uint16_t chargeLVL2;
extern volatile uint16_t chargeLVL3;
extern volatile uint16_t chargeLVL4;
extern volatile uint16_t chargeLVL5;
extern volatile uint16_t chargeLVL6;

extern volatile uint8_t  chargeBrightness;
extern uint8_t flag_ledRefresh;

// all rgb led data
extern volatile led_struct m_led_struct[LED_COUNT];

bool ischarging(void);
bool isplaying(void);
bool ischanged(void);
void detect_charging(void);
void detect_led(void);
void update_led_charge(void);
void clear_led(void);
void test(void);
void init_chargingpin(port_pin_t pin);
void init_turnoffpin(void);
void twinkle(uint8_t value_r, uint8_t value_g, uint8_t value_b);
void set_led_brightness(int led1brightness, int led2brightness, int led3brightness, uint8_t full);
void send_response (uint8_t message_ID, uint8_t payload);
void load_buff_data_color_led(void);
void restore_led_from_eeprom(void);
void restore_state_eep(void);
void send_light_data(void);
void send_battery_data(void);
void send_color_data(void);
void send_pulse_data(void);
void send_mtch_data(uint8_t index, uint8_t offset);
void load_data_color_led(void);
void rainbow(void);
void init_charge(void);
void save_led_to_eeprom(void);
void send_charging_data(void);
void send_mtch_update_status(void);
void wdt_kick(void);

#endif /* WEAR_H_ */