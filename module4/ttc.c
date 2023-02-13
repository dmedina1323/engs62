#include "ttc.h"


static void (*local_callback)();
static XTtcPs timerPort;

static void timer_handler(){
	local_callback();
	XTtcPs_ClearInterruptStatus(&timerPort, XTTCPS_IXR_INTERVAL_MASK);

}

/*
 * ttc_init -- initialize the ttc freqency and callback
 */
void ttc_init(u32 freq, void (*ttc_callback)(void)) {
	local_callback = ttc_callback;

	/* Configuration */
    XTtcPs_Config *timerConfig = XTtcPs_LookupConfig(XPAR_XTTCPS_0_DEVICE_ID);
  	XTtcPs_CfgInitialize(&timerPort, timerConfig, timerConfig->BaseAddress);

  	/* Disable interrupts temporarliy */
  	XTtcPs_DisableInterrupts(&timerPort, XTTCPS_IXR_INTERVAL_MASK);
  	gic_connect(XPAR_XTTCPS_0_INTR, (Xil_ExceptionHandler) timer_handler, &timerPort);

  	/* Setting prescaling and interval based on freq */
  	XInterval interval;
  	u8 prescaler;
  	XTtcPs_CalcIntervalFromFreq(&timerPort, freq, &interval, &prescaler);
  	XTtcPs_SetPrescaler(&timerPort, prescaler);
  	XTtcPs_SetInterval(&timerPort, interval);

  	/* Set the timer options */
  	XTtcPs_SetOptions(&timerPort, XTTCPS_OPTION_INTERVAL_MODE);
}

/*
 * ttc_start -- start the ttc
 */
void ttc_start(void) {
  	XTtcPs_EnableInterrupts(&timerPort, XTTCPS_IXR_INTERVAL_MASK);
  	XTtcPs_Start(&timerPort);
}

/*
 * ttc_stop -- stop the ttc
 */
void ttc_stop(void) {
	XTtcPs_DisableInterrupts(&timerPort, XTTCPS_IXR_INTERVAL_MASK);
	XTtcPs_Stop(&timerPort);
}

/*
 * ttc_close -- close down the ttc
 */
void ttc_close(void) {
	gic_disconnect(XPAR_XTTCPS_0_INTR);
}
