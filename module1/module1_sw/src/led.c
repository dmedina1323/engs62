#include "led.h"

/* global statics */
static XGpio port;
static XGpio port1;
static XGpioPs Ps_port;

void led_init(void){
	   XGpio_Initialize(&port, XPAR_AXI_GPIO_0_DEVICE_ID);	/* initialize device AXI_GPIO_0 */
	   XGpio_SetDataDirection(&port, CHANNEL, OUTPUT);	    /* set tristate buffer to output */
	   // for LED4
	   XGpioPs_Config* config = XGpioPs_LookupConfig(XPAR_AXI_GPIO_0_DEVICE_ID);
	   XGpioPs_CfgInitialize(&Ps_port, config, config->BaseAddr);
	   XGpioPs_SetDirectionPin(&Ps_port, LED4, Ps_OUTPUT);
	   XGpioPs_SetOutputEnablePin(&Ps_port, LED4, LED_ON);

	   XGpio_Initialize(&port1, XPAR_AXI_GPIO_1_DEVICE_ID);	/* initialize device AXI_GPIO_0 */
	   XGpio_SetDataDirection(&port1, CHANNEL, OUTPUT);	    /* set tristate buffer to output */


}

/*
 * Set <led> to one of {LED_ON,LED_OFF,...}
 *
 * <led> is either ALL or a number >= 0
 * Does nothing if <led> is invalid
 */
void led_set(u32 led, bool tostate) {

	if (led >= 0) {
		u32 current = XGpio_DiscreteRead(&port, CHANNEL);
		u32 mask = 0;
		if (led == ALL) {
			mask = 0xF;
		}
		else {
			mask = (1 << led);
		}


		if (tostate) {
			current |= mask;
		}
		else {
			current &= ~mask;
		}

		XGpio_DiscreteWrite(&port, CHANNEL, current);
	}
	if (led == 4){
		if (tostate){
			XGpioPs_WritePin(&Ps_port, LED4, LED_ON);
		} else {
			XGpioPs_WritePin(&Ps_port, LED4, LED_OFF);
		}
	}

}

/*
 * Set <led> to one of {LED_ON,LED_OFF,...}
 *
 * <led> is either ALL or a number >= 0
 * Does nothing if <led> is invalid
 */
void led_set_rgb(u32 led) {
	XGpio_DiscreteWrite(&port1, CHANNEL, led);
}


/*
 * Get the status of <led>
 *
 * <led> is a number >= 0
 * returns {LED_ON,LED_OFF,...}; LED_OFF if <led> is invalid
 */
bool led_get(u32 led) {
	if (led >= 0) {
		u32 current = XGpio_DiscreteRead(&port, CHANNEL);
		u32 mask = (1 << led);
		return ( (current & mask) > 0);	}
	else {
		return LED_OFF;
	}
}

/*
 * Toggle <led>
 *
 * <led> is a value >= 0
 * Does nothing if <led> is invalid
 */
void led_toggle(u32 led) {
	u32 current = XGpio_DiscreteRead(&port, CHANNEL);
	u32 mask = (1 << led);
	current ^= mask;
	XGpio_DiscreteWrite(&port, CHANNEL, current);
}


