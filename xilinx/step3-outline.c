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

#define CHANNEL1 1

/* hidden private state */
static int pushes=0;	       /* variable used to count interrupts */

void btn_callback(u32 button) {
	if (button == 1){
		led_toggle(0);
	} else if (button == 2){
		led_toggle(1);
	} else if (button == 4){
		led_toggle(2);
	} else if (button == 8){
		led_toggle(3);
	}
}

void sw_callback(u32 sw) {
	led_toggle(sw);
}

int main() {
  init_platform();

  gic_init();

  led_init();

  io_btn_init(btn_callback);
  io_sw_init(sw_callback);

  setvbuf(stdin,NULL,_IONBF,0);

  printf("[hello]\n"); /* so we are know its alive */
  pushes=0;
  int firstChar;
  	int c;
  	int i=0;

  	while (1){
  		// start a new input line and echo
  		printf(">");
  		c = getchar();
  		printf("%c", c);
  		i++;

  		// keep reading until a \r is encountered
  		while (c != '\r'){
  			// we only care about the first number
  			if (i == 1){
  				firstChar = c;
  			}
  			// get another character and echo for the loop
  			c = getchar();
  			printf("%c", c);
  			i++;
  		}
  		printf("\n\r");
  		// quit on q
  		if (i == 2 && firstChar == 'q'){
  			/* Turn off all LEDs on exit */
  			led_set(ALL, LED_OFF);
  			printf("The program was quit.\n\r");
  			return 0;
  		}

  		// only print when one number was input and it's [0,3]
  		// i is the number of times that a key has been entered
  		// ex: 1\r would be i=2
  		if (i == 2 && firstChar >= '0' && firstChar <= '3'){
  			char c = (char)firstChar;

  			int led_int = atoi(&c);
  			led_toggle(led_int);
  			bool led_status = led_get(led_int);
  			char* led_status_string = (led_status) ? "on" : "off";
  			printf("[%c %s]\n\r", firstChar, led_status_string);

  		}

  		// reset i for the next input
  		i=0;
  	}


  printf("\n[done]\n");

  io_btn_close();
  io_sw_close();
  cleanup_platform();					/* cleanup the hardware platform */
  gic_close();
  return 0;
}

