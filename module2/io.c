#include "io.h"
#include "gic.h"

static void (*button_callback)(u32 btn);
static void (*switch_callback)(u32 btn);

XGpio btnport;
XGpio swport;
static bool buttonPushed = false;
static u32 sw_prev = 0;

/*
 * control is passed to this function when a button is pushed
 *
 * devicep -- ptr to the device that caused the interrupt
 */
static void btn_handler() {
	if (buttonPushed == false) {
		u32 btn = XGpio_DiscreteRead(&btnport, CHANNEL1);
		button_callback(btn);
		buttonPushed = true;
	} else {
		buttonPushed = false;
	}

	XGpio_InterruptClear(&btnport, XGPIO_IR_CH1_MASK);
}

static void sw_handler() {
	u32 sw_curr = XGpio_DiscreteRead(&swport, CHANNEL1);
	u32 sw_mask = sw_curr ^ sw_prev;
	int n = 0;
	while (n < 4) {
		if (sw_mask & (1 << n)) {
			break;
		}
		n++;
	}
	switch_callback(n);
	XGpio_InterruptClear(&swport, XGPIO_IR_CH1_MASK);
	sw_prev = sw_curr;

}

/*
 * initialize the btns providing a callback
 */
void io_btn_init(void (*btn_callback)(u32 btn))
{
	button_callback = btn_callback;

	XGpio_Initialize(&btnport, XPAR_AXI_GPIO_1_DEVICE_ID);

	XGpio_InterruptDisable(&btnport, XGPIO_IR_CH1_MASK);

	gic_connect(XPAR_FABRIC_GPIO_1_VEC_ID, (Xil_ExceptionHandler) btn_handler, &btnport);

	XGpio_InterruptEnable(&btnport, XGPIO_IR_CH1_MASK);

	XGpio_InterruptGlobalEnable(&btnport);

}

/*
 * close the btns
 */
void io_btn_close(void){
	/* disconnect the interrupts (c.f. gic.h) */
	gic_disconnect(XPAR_FABRIC_GPIO_1_VEC_ID);


//	XGpio_InterruptDisable(&btnport, XGPIO_IR_CH1_MASK);

}


/*
 * initialize the switches providing a callback
 */
void io_sw_init(void (*sw_callback)(u32 sw)){
	switch_callback = sw_callback;

	XGpio_Initialize(&swport, XPAR_AXI_GPIO_2_DEVICE_ID);

	XGpio_InterruptDisable(&swport, XGPIO_IR_CH1_MASK);

	gic_connect(XPAR_FABRIC_GPIO_2_VEC_ID, (Xil_ExceptionHandler)sw_handler, &swport);

	XGpio_InterruptEnable(&swport, XGPIO_IR_CH1_MASK);

	XGpio_InterruptGlobalEnable(&swport);
}

/*
 * close the switches
 */
void io_sw_close(void) {
	/* disconnect the interrupts (c.f. gic.h) */
	gic_disconnect(XPAR_FABRIC_GPIO_2_VEC_ID);

//	XGpio_InterruptDisable(&swport, XGPIO_IR_CH2_MASK);
}
