/*
 * timer_app.h
 *
 * Created: 2014/8/7 20:34:30
 *  Author: Harifun
 */ 


#ifndef TIMER_APP_H_
#define TIMER_APP_H_

extern volatile bool flag_timer5;
extern volatile bool flag_timer4;
extern volatile bool flag_timerd5;

void init_timer4(void);
void init_timer5(void);
void init_timerd5(void);

void resetDisablePulseCount(void);

#endif /* TIMER_APP_H_ */