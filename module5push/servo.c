#include "servo.h"

static XTmrCtr pwmPtr;

/*
 * Initialize the servo, setting the duty cycle to 7.5%
 */
void servo_init(void) {
  /* Initializeing the timer counter */
  XTmrCtr_Initialize(&pwmPtr, XPAR_XTTCPS_0_DEVICE_ID);

  u32 value1 = (u32) FREQ*PERIOD;
  u32 value2 = (u32) value1*INIT_DUTYCYCLE;	// 7.5% duty cycle

  /* The amount of clock cycles each timer will pulse on for */
  XTmrCtr_SetResetValue(&pwmPtr, XTC_TIMER_0, value1);   // period
  XTmrCtr_SetResetValue(&pwmPtr, XTC_TIMER_1, value2);   // high pulse

  /* Put the timers in PWM mode */
  XTmrCtr_SetOptions(&pwmPtr, XTC_TIMER_0, OPTIONS);
  XTmrCtr_SetOptions(&pwmPtr, XTC_TIMER_1, OPTIONS);

  /* Start the timers */
  XTmrCtr_Start(&pwmPtr, XTC_TIMER_0);
  XTmrCtr_Start(&pwmPtr, XTC_TIMER_1);
}

/*
 * Set the dutycycle of the servo
 */
void servo_set(double dutycycle) {
   // Stop?
	u32 reset = FREQ*PERIOD*dutycycle;
	XTmrCtr_SetResetValue(&pwmPtr, XTC_TIMER_1, reset);
   // Start? 
}
