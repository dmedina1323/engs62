/*
 * main.c -- A program to print a dot each time button 0 is pressed.
 *
 *  Some useful values:
 *  -- XPAR_AXI_GPIO_1_DEVICE_ID -- xparameters.h
 *  -- XPAR_FABRIC_GPIO_1_VEC_ID -- xparameters.h
 *  -- XGPIO_IR_CH1_MASK         -- xgpio_l.h (included by xgpio.h)
 */

#include <stdio.h>		/* getchar,printf */
#include <stdlib.h>		/* strtod */
#include <stdbool.h>		/* type bool */
#include <unistd.h>		/* sleep */
#include <string.h>
#include "led.h"
#include "platform.h"		/* ZYBO board interface */
#include "xil_types.h"		/* u32, s32 etc */
#include "xparameters.h"	/* constants used by hardware */

#include "gic.h"		/* interrupt controller interface */
#include "xgpio.h"		/* axi gpio interface */
#include "io.h"
#include "ttc.h"
#include "xtmrctr.h"
#include "servo.h"
#include "xadcps.h"
#include "adc.h"

#define CHANNEL1 1
#define TIMERFREQ 1
#define SERVO_RANGE 0.0225F
#define HIGH (INIT_DUTYCYCLE+SERVO_RANGE)	// 0.105
#define LOW (INIT_DUTYCYCLE-SERVO_RANGE)	// 0.06

/* hidden private state */



void btn_callback(u32 button) {
	if (button == 1){
		led_toggle(0);

		float temperature = adc_get_temp();
		printf("[Temp = %.2fC]\n", temperature);
	} else if (button == 2){
		led_toggle(1);

		float voltage = adc_get_vccint();
		printf("[VccInt = %.2fV]\n", voltage);
	} else if (button == 4){
		led_toggle(2);

		float pot_value = adc_get_pot();
		pot_value /= 1.5;
		printf("[Pot = %.2fV]\n", pot_value);
	} else if (button == 8){
		led_toggle(3);

		float pot_value = adc_get_pot();
		pot_value /= 3;

		printf("pot_value = %.2f\n", pot_value);

		double dc = (double)((HIGH-LOW)*pot_value + LOW);
		servo_set(dc);
		printf("Duty cycle: %f\n", dc*100);

	}
}

void sw_callback(u32 sw) {
	led_toggle(sw);
}

void ttc_callback(void) {
	led_toggle(4);
}


void Initialize(void);
void Closeout(void);
void getLine(char* word);

int main() {
  init_platform();

  Initialize();

  double dutycycle = INIT_DUTYCYCLE;


  setvbuf(stdin,NULL,_IONBF,0);

  	printf("[hello]\n"); /* so we are know its alive */

  	char word[20];

  	while (1){
  		// start a new input line and echo
  		getLine(word);
  		printf("\n\r");

  		// quit on q
  		if (strlen(word) == 1 && word[0] == 'q'){
			/* Turn off all LEDs on exit */
  			printf("The program was quit.\n\r");
  			break;
  		}

  		if (strcmp(word, "low") == 0){
  			dutycycle = LOW;
  			printf("Duty cycle: %f\n", dutycycle*100);
  			servo_set(dutycycle);
  		}
  		else if (strcmp(word, "high") == 0){
  			dutycycle = HIGH;
  			printf("Duty cycle: %f\n", dutycycle*100);
  			servo_set(dutycycle);
  		}
  		else if (strcmp(word, "a") == 0) {
  			if (dutycycle >= LOW-0.0001 && dutycycle < HIGH-0.001) {
  				dutycycle += 0.0025;
  				printf("Duty cycle: %f\n", dutycycle*100);
  				servo_set(dutycycle);
  			}
		}
		else if (strcmp(word, "s") == 0) {
			if (dutycycle > LOW+0.001 && dutycycle <= HIGH+0.0001) {
				dutycycle -= 0.0025;
				printf("Duty cycle: %f\n", dutycycle*100);
				servo_set(dutycycle);
			}
		}
  		// only print when one number was input and it's [0,3]
  		// i is the number of times that a key has been entered
  		// ex: 1\r would be i=2
  		if (strlen(word) == 1 && word[0] >= '0' && word[0] <= '3'){

  			int led_int = atoi(word);
  			led_toggle(led_int);
  			bool led_status = led_get(led_int);
  			char* led_status_string = (led_status) ? "on" : "off";
  			printf("[%c %s]\n\r", word[0], led_status_string);

  		}
  	}

  	Closeout();

	cleanup_platform();					/* cleanup the hardware platform */

	printf("\n[done]\n");

	return 0;
}

void getLine(char* word){
	char c = '\0';
	int i = 0;

	printf(">");
	c = getchar();
	word[i] = c;

	printf("%c", c);
	i++;

	// keep reading until a \r is encountered
	while (c != '\r'){
		// get another character and echo for the loop
		c = getchar();
		word[i] = c;
		printf("%c", c);
		i++;
	}
	word[i-1] = '\0';
}

void Initialize(){
	gic_init();

	led_init();

	io_btn_init(btn_callback);
	io_sw_init(sw_callback);
	ttc_init(TIMERFREQ, ttc_callback);
	ttc_start();
	servo_init();
	adc_init();
}

void Closeout() {
  	led_set(ALL, LED_OFF);

	led_set(4, LED_OFF);
	io_btn_close();
	io_sw_close();
	ttc_stop();
	ttc_close();
	gic_close();
}
