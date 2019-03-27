/*
 * Main Method for testing the PWM Code for the K64F
 * PWM signal can be connected to output pins are PC3 and PC4
 * 
 * Author:  
 * Created:  
 * Modified:  
 */

#include "MK64F12.h"
#include "uart.h"
#include "PWM.h"

void initialize();
void en_interrupts();
void delay();

void turnOffAll(){
		GPIOD_PCOR = (1 << 0);
		GPIOD_PCOR = (1 << 1);
		GPIOD_PCOR = (1 << 2);
		GPIOD_PCOR = (1 << 3);
}

void turnOnAll(){
	GPIOD_PSOR = (1 << 0);
	GPIOD_PSOR = (1 << 1);
	GPIOD_PSOR = (1 << 2);
	GPIOD_PSOR = (1 << 3);
}

int test_main(void){
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	GPIOD_PDDR = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
	PORTD_PCR0 |= PORT_PCR_MUX(1);				//Enables GPIO mode for PTD0
	PORTD_PCR1 |= PORT_PCR_MUX(1);				//Enables GPIO mode for PTD1
	PORTD_PCR2 |= PORT_PCR_MUX(1);				//Enables GPIO mode for PTD2
	PORTD_PCR3 |= PORT_PCR_MUX(1);				//Enables GPIO mode for PTD3
	turnOnAll();
	for(;;);
}

int stepper_main(void){
		// Enable clocks on Port D
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	
	GPIOD_PDDR = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

	PORTD_PCR0 = PORT_PCR_MUX(1);				//Enables GPIO mode for PTD0
	PORTD_PCR1 = PORT_PCR_MUX(1);				//Enables GPIO mode for PTD1
	PORTD_PCR2 = PORT_PCR_MUX(1);				//Enables GPIO mode for PTD2
	PORTD_PCR3 = PORT_PCR_MUX(1);				//Enables GPIO mode for PTD3

	// Configure the Signal Multiplexer for the Port D GPIO Pins
	// Configure the GPIO Pins for Output
	int forward = 1;
	int phase = 0;
	while ( 1 ){
		// Turn off all coils , Set GPIO pins to 0
		
		turnOffAll();
		
		// Set one pin high at a time
		if( forward ){
			if( phase == 0){ turnOffAll(); GPIOD_PSOR = (1 << 0); phase ++;} //A, 1a
			else if( phase == 1){ turnOffAll(); GPIOD_PSOR = (1 << 1); phase ++;} //B ,2a
			else if ( phase == 2) { turnOffAll(); GPIOD_PSOR = (1 << 2); phase ++;} //C ,1b
			else { turnOffAll(); GPIOD_PSOR = (1 << 3); phase =0;} //D ,2b
		}
		else {// reverse
			if ( phase == 0) { turnOffAll(); GPIOD_PSOR |= (1 << 3); phase ++;} //D ,2b
			else if ( phase == 1) { turnOffAll(); GPIOD_PSOR |= (1 << 2); phase ++;} //C ,1b
			else if ( phase == 2) { turnOffAll(); GPIOD_PSOR |= (1 << 1); phase ++;} //B ,2a
			else { turnOffAll(); GPIOD_PSOR |= (1 << 0); phase =0;} //A ,1a
		}
		// Note - you need to write your own delay function
		delay (10); // smaller values = faster speed
	}

}

int main(void){
	// Initialize UART and PWM
	initialize();

	// Print welcome over serial
	put("Running... \n\r");
	
	//Step 3
	//SetDutyCycle(20, 10000, 1);

	//for(;;) ;  //then loop forever
	
	//Step 9
	for(;;)  //loop forever
	{
		uint16_t dc = 0;
		uint16_t freq = 10000; /* Frequency = 10 kHz */
		uint16_t dir = 1;
		char c = 48;
		int i=5;
		int j=1;
		
		// 0 to 100% duty cycle in forward direction
		for (;;){
			SetDutyCycle(25, freq, dir);
			SetServoDutyCycle(i, 50, 0);
			i+=j;
			if ( i >= 10){
				j = -1;
			}
			else if( i <=5){
				j = 1;
			}
			delay(100);
		}
	return 0;
}
}

int first_main(void)
{
	// Initialize UART and PWM
	initialize();

	// Print welcome over serial
	put("Running... \n\r");
	
	//Step 3
	//SetDutyCycle(20, 10000, 1);

	//for(;;) ;  //then loop forever
	
	//Step 9
	for(;;)  //loop forever
	{
		uint16_t dc = 0;
		uint16_t freq = 10000; /* Frequency = 10 kHz */
		uint16_t dir = 1;
		char c = 48;
		int i=0;
		
		// 0 to 100% duty cycle in forward direction
		for (i=0; i<100; i++){
			SetDutyCycle(i, freq, dir);
			delay(10);
		}
		
		// 100% down to 0% duty cycle in the forward direction
		for (i=100; i>=0; i--){
			SetDutyCycle(i, freq, dir);
			delay(10);
		}
		
		// 0 to 100% duty cycle in reverse direction
		for (i=0; i<100; i++){
			SetDutyCycle(i, freq, -1);
			delay(10);
		}
		
		// 100% down to 0% duty cycle in the reverse direction
		for (i=100; i>=0; i--){
			SetDutyCycle(i, freq, -1);
			delay(10);
		}

	}
	return 0;
}


/**
 * Waits for a delay (in milliseconds)
 * 
 * del - The delay in milliseconds
 */
void delay(int del){
	int i;
	for (i=0; i<del*50000; i++){
		// Do nothing
	}
}

void motor_shield_init(){
	//enable PORTB clock
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	
	//Set to output mode
	GPIOB_PDDR |= ((1 << 2) | (1 << 3));
	
	//enable H bridge
	PORTB_PCR2 = PORT_PCR_MUX(1);
	PORTB_PCR3 = PORT_PCR_MUX(1);
	
	//Set to high to enable H bridge
	GPIOB_PSOR |= ((1 << 2) | (1 << 3));
	
	
}

void initialize()
{
	// Initialize UART
	uart_init();	
	
	// Initialize the FlexTimer
	InitPWM();
	
	InitServoPWM();
	
	motor_shield_init();
}
