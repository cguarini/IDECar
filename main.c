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
#include "camera.h"
#include "steering.h"


void initialize();
void en_interrupts();

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
			SetDutyCycle(25, 25, freq, dir);
			SetServoDutyCycle(i, 50, 0);
			i+=j;
			if ( i >= 10){
				j = -1;
			}
			else if( i <=5){
				j = 1;
			}
		}
	return 0;
}
}

int main(void){
	// Initialize UART and PWM
	initialize();
	initCamera();


//Step 9

	uint16_t freq = 10000; /* Frequency = 10 kHz */
	uint16_t dir = 1;
	
	uint16_t line[128];

	
	// 0 to 100% duty cycle in forward direction
	for (;;){
		
		
		getLine(line);
		steeringFunction(line);
		//SetDutyCycle(0, 0, freq, dir);
	}
	return 0;

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
	//uart_init();	
	
	// Initialize the FlexTimer
	InitPWM();
	
	InitServoPWM();
	
	motor_shield_init();
}
