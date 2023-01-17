#include "io.h"
#include "gic.h"

static void (*saved_callback)(u32 btn);

XGpio btnport;
static bool buttonPushed = false;
/*
 * control is passed to this function when a button is pushed
 *
 * devicep -- ptr to the device that caused the interrupt
 */
static void btn_handler() {

	if (buttonPushed == false) {
		u32 btn = XGpio_DiscreteRead(&btnport, CHANNEL1);
		saved_callback(btn);
		buttonPushed = true;
	} else {
		buttonPushed = false;
	}

	XGpio_InterruptClear(&btnport, XGPIO_IR_CH1_MASK);
}

/*
 * initialize the btns providing a callback
 */
void io_btn_init(void (*btn_callback)(u32 btn))
{
	saved_callback = btn_callback;

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

	/* close the gic (c.f. gic.h) */
	gic_close();
	XGpio_InterruptDisable(&btnport, XGPIO_IR_CH1_MASK);

}


/*
 * initialize the switches providing a callback
 */
void io_sw_init(void (*sw_callback)(u32 sw));

/*
 * close the switches
 */
void io_sw_close(void);
