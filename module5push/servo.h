/*
 * servo.h
 */
#pragma once

#include <stdio.h>
#include "xtmrctr.h"
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */

#define OPTIONS (XTC_PWM_ENABLE_OPTION | XTC_EXT_COMPARE_OPTION | XTC_DOWN_COUNT_OPTION)

#define FREQ            50e6     // Frequency of the native clock 
#define PERIOD          20e-3    // Total period of the waveform
#define INIT_DUTYCYCLE  0.0825    // Initial desired duty cycle of PWM waveform

/*
 * Initialize the servo, setting the duty cycle to 7.5%
 */
void servo_init(void);

/*
 * Set the dutycycle of the servo
 */
void servo_set(double dutycycle);



