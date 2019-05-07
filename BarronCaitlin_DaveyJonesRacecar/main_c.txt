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
#include "Constants.h"


void initialize();
void en_interrupts();


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
	//Initialization Code
	initialize();
	initCamera();
	//Used for speed selection through SW2
	Button_Init();

	//Used by motors
	uint16_t freq = 10000;
	uint16_t dir = 1;
	
	uint16_t line[128];
	
	//State machine for determining speed through SW2
	int state = 0;
	MAX_PWM = TOP_MAX;
	TURN_PWM = TOP_TURN;

	for (;;){
		
	//Button pressed
	if(!(GPIOC_PDIR & ( 1 << 6))){//checks if SW2 is pressed
		//Increment state, reset if no further states
		state++;
		if(state > 2){
			state = 0;
		}
		
		//Set speeds based on the state
		if(state == 0){
			MAX_PWM = TOP_MAX;
			TURN_PWM = TOP_TURN;
		}
		else if(state == 1){
			MAX_PWM = MID_MAX;
			TURN_PWM = MID_TURN;
		}
		else{
			MAX_PWM = LOW_MAX;
			TURN_PWM = LOW_TURN;
		}
		
		delay(50);
		
	}
	
	//Get a line from the camera
	getLine(line);
	//Steer car based on this line
	steeringFunction(line, MAX_PWM, TURN_PWM);
	
	//Used for testing servo, turns off both motors
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
