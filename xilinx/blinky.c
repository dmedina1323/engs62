/*
 * blinky.c -- working with Serial I/O and GPIO
 *
 * Assumes the LED's are connected to AXI_GPIO_0, on channel 1
 *
 * Terminal Settings:
 *  -Baud: 115200
 *  -Data bits: 8
 *  -Parity: no
 *  -Stop bits: 1
 */
#include <stdio.h>				/* printf(), getchar() */
#include <stdlib.h>
#include "xil_types.h"			/* u32, u16 etc */
#include "platform.h"			/* ZYBOboard interface */
#include <xgpio.h>				/* Xilinx GPIO functions */
#include "xparameters.h"		/* constants used by the hardware */
#include <string.h>
#include "led.h"

#define OUTPUT 0x0	/* setting GPIO direction to output */
#define CHANNEL1 1	/* channel 1 of the GPIO port */
#define CHANNEL2 2
#define MAXBUFF 20

int main() {

	init_platform();		/* initialize the hardware platform */

	/* Set stdin unbuffered, forcing getchar to return immediately when
		a character is typed. */
	setvbuf(stdin,NULL,_IONBF,0);

	/* Print [Hello] on the screen with anew line */
	printf("\n\r[Hello]\n\r");

	/* Turn on LED 0 */
	led_init();
	led_set(0, LED_ON);
	led_set(4, LED_ON);
//	XGpio_DiscreteWrite(&port, CHANNEL1, 0x1);			/* turn on led0 */

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
			/* Turn off LED 0 */
//			XGpio_DiscreteWrite(&port, CHANNEL1, 0x0);			/* turn off led0 */
			led_set(ALL, LED_OFF);
			led_set(4, LED_OFF);
			led_set_rgb(0b000);
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

		if (i == 2 && (firstChar == 'r' || firstChar == 'g' || firstChar == 'b' || firstChar == 'y' || firstChar == 'w')){
			switch (firstChar){
				case 'r':
					led_set_rgb(0b100);
					break;
				case 'g':
					led_set_rgb(0b010);
					break;
				case 'b':
					led_set_rgb(0b001);
					break;
				case 'y':
					led_set_rgb(0b110);
					break;
				case 'w':
					led_set_rgb(0b111);
			}
		}

		// reset i for the next input
		i=0;
	}


	cleanup_platform();					/* cleanup the hardware platform */

   return 0;

}
