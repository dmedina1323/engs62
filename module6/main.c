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
#define TIMERFREQ 10

bool done = false;

/**** States ****/
#define TM 			0
#define YELLOW 		1
#define TS 			2
#define PEDESTRIAN 	3
#define TRP			4
#define TRL			5
#define MT			6
#define MT_TR		7

int lightState = TM;

/**** Timing ****/
#define TRAFFIC_TC 		100		// 10 seconds at 1/10th seconds per interrupt
#define PEDESTRIAN_TC 	100		// 10 seconds at 1/10th seconds per interrupt
#define LIGHT_TC		30		// 3 seconds

int timerCount = 0;
int blueCount = 0;

/**** Flags ****/
int t3minute = 0;
int t3sec = 0;
int t20sec = 0;
int t30sec = 0;

int pedestrian = 0;
int train = 0;
int maintenance =0;

/**** Lights ****/
#define GREEN_LIGHT  0b010
#define YELLOW_LIGHT 0b110
#define RED_LIGHT    0b100
#define BLUE_LIGHT   0b001


/**** Train ****/
#define TRAIN_SW 0
#define GATE_OPEN HIGH
#define GATE_CLOSE LOW

/**** Maintenance ****/
#define MAINT_SW 1


/**** START OF WIFI VARIABLES AND STRUCTS ****/
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

u8 mode = UPDATE;

float pot_value;

static XUartPs uartPort_0;
static XUartPs uartPort_1;
static u8 buffer;

int count1;
int count2 = sizeof(update_response_t);;
u8* png;
ping_t p;
ping_t p_send;

update_response_t r;
update_request_t request;

u8* resp = (u8*)&r;
/**** END OF WIFI ****/

/**** Functions ****/
void Initialize(void);
void Closeout(void);
void btn_callback(u32 button);
void ttc_callback(void);
void sw_callback(u32 sw);
void change_state(void);

static void wifi_train(void);
static void wifi_maintenance(void);

static void uart_0_handler(void* CallBackRef, u32 Event, u32 EventData);
static void uart_1_handler(void* CallBackRef, u32 Event, u32 EventData);

int main() {
  init_platform();

  Initialize();

  setvbuf(stdin,NULL,_IONBF,0);

  printf("[hello]\n"); /* so we are know its alive */
  led_set_rgb(GREEN_LIGHT);
  p_send.type = PING;
  p_send.id = 12;

  request.type = UPDATE;
  request.id = 0;
  request.value = 0;


  while (!done) {
	  sleep(1);
  }

  printf("[done]\n");
  sleep(1);

  Closeout();

  cleanup_platform();					/* cleanup the hardware platform */

  return 0;

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

void btn_callback(u32 button) {
	if (button == 1 || button == 2) {
		printf("pedestrian requested\n");
		pedestrian = 1;
		if (lightState == TRP || lightState == MT) {
			led_set(4, LED_ON);
		}
		else {
			change_state();
		}
	}

}

void ttc_callback(void) {
	timerCount++;
	switch (lightState) {
		case (TM):
			if (timerCount == TRAFFIC_TC) {
				t3minute = 1;
				printf("3 minutes passed!\n");
				change_state();
			}
			break;
		case (YELLOW):
			if (timerCount == LIGHT_TC) {
				t3sec = 1;
				change_state();
			}
			break;
		case (TS):
			change_state();
			break;
		case (TRL):
			if (timerCount == PEDESTRIAN_TC) {
				t20sec = 1;
				change_state();
			}
			break;
		case (MT):;
			if (!train){
				float pot_value = adc_get_pot();
				pot_value /= 3;

				double dc = (double)((HIGH-LOW)*pot_value + LOW);
				servo_set(dc);
			}
			if (timerCount % 5 == 0) {
				blueCount++;
				if (blueCount % 2 != 0) {
					led_set_rgb(BLUE_LIGHT);
				}
				else {
					led_set_rgb(0);
				}
			}
			break;
		case (PEDESTRIAN):
			if (timerCount == PEDESTRIAN_TC) {
				t30sec = 1;
				change_state();
			}
			break;
	}

	/* Polling wifi module 10 times/sec */
	XUartPs_Send(&uartPort_0, &request, sizeof(update_request_t));	// sending to wifi module
}

void sw_callback(u32 sw) {

//	if (sw == TRAIN_SW) {
//		if (lightState == TRP) {
//			printf("train leaving\n\n");
//			train = 0;
//			change_state();
//		}
//		else if (lightState == MT) {
//			if (!train) {
//				printf("closing gate\n\n");
//				servo_set(GATE_CLOSE);
//				train = 1;
//			}
//			else {
//				servo_set(GATE_OPEN);
//				train = 0;
//			}
//		}
//		else {
//			train = 1;
//			if (lightState == TS || lightState == PEDESTRIAN){
//				printf("train arriving changing to red state\n\n");
//				lightState = TRP;
//				if (pedestrian) {
//					led_set(4, LED_ON);
//				}
//				servo_set(GATE_CLOSE);
//			}
//			else if (lightState != YELLOW){
//				printf("train arriving changing to yellow state\n\n");
//				lightState = YELLOW;
//				timerCount = 0;
//				led_set_rgb(YELLOW_LIGHT);
//				t3minute = 0;
//			}
//		}
//	}
//	else if (sw == MAINT_SW) {
//		if (lightState == MT) {
//			maintenance = 0;
//			change_state();
//		}
//		else if (lightState == TRP) {
//			lightState = MT;
//		}
//		else if (lightState != YELLOW){
//			maintenance = 1;
//			printf("maintenance arrived. changing to yellow state\n\n");
//			lightState = YELLOW;
//			timerCount = 0;
//			led_set_rgb(YELLOW_LIGHT);
//			t3minute = 0;
//		}
//		else if (lightState == YELLOW) {
//			maintenance = 1;
//		}
//	}
}

void change_state(void) {
	switch (lightState){
		case (TM):
			if (t3minute && pedestrian) {
				printf("In tm changing to yellow\n\n");
				lightState = YELLOW;
				led_set_rgb(YELLOW_LIGHT);
				timerCount = 0;
				t3minute = 0;
			}
			break;
		case (YELLOW):
			if (t3sec) {
				printf("In yellow changing to TS\n\n");
				lightState = TS;
				led_set_rgb(RED_LIGHT);
				timerCount = 0;
				t3sec = 0;
			}
			break;
		case (TS):
			if (maintenance && train) {
				lightState = MT;
				servo_set(GATE_CLOSE);
			}
			else if (maintenance) {
				lightState = MT;
			}
			else if (train) {
				printf("In ts changing to trp\n\n");
				lightState = TRP;
				if (pedestrian) {
					led_set(4, LED_ON);
				}
				servo_set(GATE_CLOSE);
			}
			else if (pedestrian) {
				printf("In ts changing to pedestrian\n\n");
				lightState = PEDESTRIAN;
				pedestrian = 0;
				led_set(4, LED_ON);
			}
			timerCount = 0;
			break;
		case (TRP):
			if (!train){
				lightState = TRL;
				timerCount = 0;
			}
			break;
		case (TRL):
			if (t20sec) {
				printf("In trl changing to tm\n\n");
				lightState = TM;

				led_set(4, LED_OFF);
				pedestrian = 0;
				servo_set(GATE_OPEN);
				timerCount = 0;
				t20sec = 0;
				led_set_rgb(GREEN_LIGHT);
			}
			break;
		case (MT):
			if (!maintenance) {
				if (train) {
					printf("In mt changing to trp\n\n");
					lightState = TRP;
					if (pedestrian) {
						led_set(4, LED_ON);
					}
					servo_set(GATE_CLOSE);
					led_set_rgb(RED_LIGHT);
				}
				else if (pedestrian) {
					lightState = PEDESTRIAN;
					printf("In mt changing to pedestrian\n\n");
					pedestrian = 0;
					led_set(4, LED_ON);
					led_set_rgb(RED_LIGHT);
				}
				else {
					printf("In mt changing to traffic moving\n\n");
					lightState = TM;
					t30sec = 0;
					led_set(4, LED_OFF);
					led_set_rgb(GREEN_LIGHT);
					servo_set(GATE_OPEN);
				}
				timerCount = 0;
			}
			break;
		case (PEDESTRIAN):
			if (t30sec) {
				printf("In pedestrian changing to tm\n\n");
				lightState = TM;
				timerCount = 0;
				t30sec = 0;
				led_set(4, LED_OFF);
				led_set_rgb(GREEN_LIGHT);
				servo_set(GATE_OPEN);
			}
			break;
	}
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
					resp = (u8*)&r;
					count2 = sizeof(update_response_t);
					if (r.values[18] == 1) {
						if (!train){
							wifi_train();
						}
					}
					else if (r.values[18] == 2){
						if (!maintenance) {
							wifi_maintenance();
						}
					}
					else if (r.values[18] == 3) {
						if (train) {
							wifi_train();
						}
					}
					else if (r.values[18] == 4) {
						if (maintenance) {
							wifi_maintenance();
						}
					}
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

static void wifi_train(void) {
	if (lightState == TRP) {
		printf("train leaving\n\n");
		train = 0;
		change_state();
	}
	else if (lightState == MT) {
		if (!train) {
			printf("closing gate\n\n");
			servo_set(GATE_CLOSE);
			train = 1;
		}
		else {
			servo_set(GATE_OPEN);
			train = 0;
		}
	}
	else {
		train = 1;
		if (lightState == TS || lightState == PEDESTRIAN){
			printf("train arriving changing to red state\n\n");
			lightState = TRP;
			if (pedestrian) {
				led_set(4, LED_ON);
			}
			servo_set(GATE_CLOSE);
		}
		else if (lightState != YELLOW){
			printf("train arriving changing to yellow state\n\n");
			lightState = YELLOW;
			timerCount = 0;
			led_set_rgb(YELLOW_LIGHT);
			t3minute = 0;
		}
	}
}

static void wifi_maintenance(void){
	if (lightState == MT) {
		maintenance = 0;
		change_state();
	}
	else if (lightState == TRP) {
		maintenance = 1;
		lightState = MT;
	}
	else if (lightState != YELLOW){
		maintenance = 1;
		printf("maintenance arrived. changing to yellow state\n\n");
		lightState = YELLOW;
		timerCount = 0;
		led_set_rgb(YELLOW_LIGHT);
		t3minute = 0;
	}
	else if (lightState == YELLOW) {
		maintenance = 1;
	}
}
