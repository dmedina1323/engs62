#include "adc.h"

XAdcPs adcPort;

/*
 * initialize the adc module
 */
void adc_init(void){
	XAdcPs_Config* adcConfig = XAdcPs_LookupConfig(XPAR_XADCPS_0_DEVICE_ID);
	XAdcPs_CfgInitialize(&adcPort, adcConfig, adcConfig->BaseAddress);

	if (XAdcPs_SelfTest(&adcPort) != XST_SUCCESS) {
		printf("ADC test failed\n");
	}

	XAdcPs_SetSequencerMode(&adcPort, XADCPS_SEQ_MODE_SAFE);
	XAdcPs_SetAlarmEnables(&adcPort, 0);
//	XAdcPs_SetSeqInputMode(&adcPort, XADCPS_SEQ_CH_AUX14 | XADCPS_SEQ_CH_VCCINT | XADCPS_SEQ_CH_TEMP);
	XAdcPs_SetSeqChEnables(&adcPort, XADCPS_SEQ_CH_AUX14 | XADCPS_SEQ_CH_VCCINT | XADCPS_SEQ_CH_TEMP);
	XAdcPs_SetSequencerMode(&adcPort, XADCPS_SEQ_MODE_CONTINPASS);
}

/*
 * get the internal temperature in degree's centigrade
 */
float adc_get_temp(void) {
	u16 adc_value = XAdcPs_GetAdcData(&adcPort, XADCPS_CH_TEMP);
	return(XAdcPs_RawToTemperature(adc_value));
}

/*
 * get the internal vcc voltage (should be ~1.0v)
 */
float adc_get_vccint(void) {
	u16 adc_value = XAdcPs_GetAdcData(&adcPort, XADCPS_CH_VCCINT);
	return(XAdcPs_RawToVoltage(adc_value));
}

/*
 * get the **corrected** potentiometer voltage (should be between 0 and 1v)
 */
float adc_get_pot(void) {
	u16 adc_value = XAdcPs_GetAdcData(&adcPort, XADCPS_CH_AUX_MAX-1);
	return(XAdcPs_RawToVoltage(adc_value));
}
