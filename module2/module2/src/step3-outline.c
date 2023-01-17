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

void callback(u32 button) {
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

int main() {
  init_platform();

  gic_init();

  led_init();

  io_btn_init(callback);

  printf("[hello]\n"); /* so we are know its alive */
  pushes=0;
  while(pushes<5); /* do nothing and handle interrups */


  printf("\n[done]\n");

  io_btn_close();
  cleanup_platform();					/* cleanup the hardware platform */
  return 0;
}

