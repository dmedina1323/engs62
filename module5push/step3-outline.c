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
#include "adc.h"
#include "servo.h"
#include "ttc.h"

#define CONFIGURE 0
#define PING 1
#define UPDATE 2
#define TIMERFREQ 1

#define SERVO_RANGE 0.0225F
#define HIGH (INIT_DUTYCYCLE + SERVO_RANGE)
#define LOW (INIT_DUTYCYCLE - SERVO_RANGE)

typedef struct {
	int type;
	int id;
} ping_t;

typedef struct {
	int type;
	int id;
	int value;
} update_request_t;

typedef struct {
	int type;
	int id;
	int average;
	int values[30];
} update_response_t;

bool done = false;
u8 mode = CONFIGURE;

float pot_value;

static XUartPs uartPort_0;
static XUartPs uartPort_1;
static u8 buffer;

int count1;
int count2;
u8* png;
u8* resp;

ping_t p;
ping_t p_send;
//ping_t p_recv;

update_response_t r;
update_request_t request;
//update_response_t response;


void btn_callback(u32 button) {
	if (button == 8){ // button 3
		done = true;

	} else if (button == 4) { // button 2
		mode = UPDATE;
		pot_value = adc_get_pot();
		pot_value /= 2.91;
		request.value = (int)(pot_value * 100);
		count2 = sizeof(update_response_t);
		resp = (u8*)&r;

		XUartPs_Send(&uartPort_0, &request, sizeof(update_request_t));	// sending to wifi module
		printf("[UPDATE]\n\r");

	} else if (button == 2) { // button 1
		printf("[PING]\n");
		mode = PING;
		count1 = sizeof(ping_t);

		png = (u8*)&p;
		XUartPs_Send(&uartPort_0, &p_send, sizeof(ping_t));	// sending to wifi module

	} else if (button == 1) { // button 0
		printf("You can now interact with the WiFi module: \n");
		mode = CONFIGURE;
	}

}

void sw_callback(u32 sw) {
	led_toggle(sw);
}


void Initialize(void);
void Closeout(void);
static void uart_0_handler(void* CallBackRef, u32 Event, u32 EventData);
static void uart_1_handler(void* CallBackRef, u32 Event, u32 EventData);

int main() {
  init_platform();

  Initialize();

  setvbuf(stdin,NULL,_IONBF,0);

  printf("[hello]\n"); /* so we are know its alive */
  p_send.type = PING;
  p_send.id = 12;

  request.type = UPDATE;
  request.id = 12;


  while (!done) {
	switch (mode) {
		case CONFIGURE:
			led_set(0, LED_ON);
			led_set(1, LED_OFF);
			led_set(2, LED_OFF);
			led_set(3, LED_OFF);
			break;
		case PING:
//			XUartPs_Recv(&uartPort_0, &buffer, 1);
			led_set(1, LED_ON);
			led_set(0, LED_OFF);
			led_set(2, LED_OFF);
			led_set(3, LED_OFF);
			break;
		case UPDATE:
			led_set(2, LED_ON);
			led_set(0, LED_OFF);
			led_set(1, LED_OFF);
			led_set(3, LED_OFF);
			break;
	}
	sleep(1);
  }

  printf("[done]\n");
  sleep(1);

  Closeout();

  cleanup_platform();					/* cleanup the hardware platform */

  return 0;

}

void ttc_callback(void) {
	led_toggle(4);
}

void Initialize(){
	gic_init();
	led_init();

	io_btn_init(btn_callback);
	io_sw_init(sw_callback);
	io_uart_init();
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
	io_uart_close();
	gic_close();
}

void io_uart_init(){

	/****************** UART0 ********************/
	XUartPs_Config* uartConfig_0 = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
	XUartPs_CfgInitialize(&uartPort_0, uartConfig_0, uartConfig_0->BaseAddress);
	XUartPs_SetBaudRate(&uartPort_0, WIFI_BAUD);
	XUartPs_SetFifoThreshold(&uartPort_0, 1);
	XUartPs_SetInterruptMask(&uartPort_0,XUARTPS_IXR_RXOVR);
	XUartPs_SetHandler(&uartPort_0, (XUartPs_Handler) uart_0_handler, &uartPort_0);
	gic_connect(XPAR_XUARTPS_0_INTR, (Xil_ExceptionHandler)XUartPs_InterruptHandler, &uartPort_0);

	/****************** UART1 ********************/
	XUartPs_Config* uartConfig_1 = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
	XUartPs_CfgInitialize(&uartPort_1, uartConfig_1, uartConfig_1->BaseAddress);
	XUartPs_SetFifoThreshold(&uartPort_1, 1);
	XUartPs_SetInterruptMask(&uartPort_1,XUARTPS_IXR_RXOVR);
	XUartPs_SetHandler(&uartPort_1, (XUartPs_Handler) uart_1_handler, &uartPort_1);
	gic_connect(XPAR_XUARTPS_1_INTR, (Xil_ExceptionHandler)XUartPs_InterruptHandler, &uartPort_1);

}

void io_uart_close(void) {
	XUartPs_DisableUart(&uartPort_1);
	gic_disconnect(XPAR_XUARTPS_1_INTR);
}


static void uart_0_handler(void* CallBackRef, u32 Event, u32 EventData){ // wifi module interrupt
	XUartPs* uart0 = (XUartPs*)CallBackRef;
	if (Event == XUARTPS_EVENT_RECV_DATA) {
		switch (mode){
			case CONFIGURE:
				XUartPs_Recv(uart0, &buffer, 1);
				if (buffer == '\r') {
					buffer = '\n';
					XUartPs_Send(&uartPort_1, &buffer, 1);
					buffer = '\r';
				}
				XUartPs_Send(&uartPort_1, &buffer, 1);
				break;
			case PING:;
				u8 msg_buff1;
				XUartPs_Recv(uart0, &msg_buff1, 1);
				*png = msg_buff1;
				png++;
				count1--;
				if (count1 == 0){
					printf("Received PING of id %d.\n\r", p.id);
				}
				break;
			case UPDATE:;

				u8 msg_buff2;
				XUartPs_Recv(uart0, &msg_buff2, 1);
				*resp = msg_buff2;
				resp++;
				count2--;

				if (count2 == 0){
					printf("Received update of average %d with id %d with value %d.\n\r", r.average, r.id, r.values[12]);
					double dc = (double)((HIGH-LOW)*((double)r.values[12]/100.) + LOW);
//					printf("dc = %f\n", dc);

					servo_set(dc);
				}

				break;
		}
	}

}

static void uart_1_handler(void* CallBackRef, u32 Event, u32 EventData){ // terminal input interrupt
	XUartPs* uart1 = (XUartPs*)CallBackRef;
	if (Event == XUARTPS_EVENT_RECV_DATA ) {
		if (mode == CONFIGURE){
			XUartPs_Recv(uart1, &buffer, 1);
			XUartPs_Send(&uartPort_0, &buffer, 1);
		}
	}



}
