/*
 * io.h -- switch and button module interface
 *
 */
#pragma once

#include <stdio.h>			/* printf for errors */
#include <stdbool.h>
#include <xgpio.h>		  	/* axi gpio */
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */

#define CHANNEL1 1
#define CHANNEL2 2
#define WIFI_BAUD 9600

/*
 * initialize the btns providing a callback
 */
void io_btn_init(void (*btn_callback)(u32 btn));

/*
 * close the btns
 */
void io_btn_close(void);


/*
 * initialize the switches providing a callback
 */
void io_sw_init(void (*sw_callback)(u32 sw));

/*
 * close the switches
 */
void io_sw_close(void);

void io_uart_init(void);

void io_uart_close(void);

