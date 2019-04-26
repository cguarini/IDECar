#include "MK64F12.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "uart.h"
#include "Constants.h"
#include "steering.h"

#define MAX_INPUT (65535)
#define STRAIGHT (1)




int maxValue = 0;
float maxSpeed = 0;
float turnFactor = 0;


uint16_t scaledLine[128];

int left;

//Enable printing to console
int steeringDebug = 0;

uint16_t freq = 10000; /* Frequency = 10 kHz */
uint16_t dir = 1;


//Determines how hard the car has to turn
void Steer(float steeringFactor){
	
	//Prints steering factor to console
	if(steeringDebug){
		char str[100];
		sprintf(str, "%f", steeringFactor);
		put(str);
		put("\n\r");
	}
	

	//default straight
	float dutyCycle = 7;
	int left = MAX_PWM;
	int right = MAX_PWM;
	
	
		//Initial value
	if (turnFactor == 0){
		turnFactor = (float) (.5);
	}
	//Ramp up speed
	if(turnFactor < 1.0){
		turnFactor += (float) (.01);
	}
	
	// differential steering
	if(steeringFactor > 1){
		//turnFactor = (((steeringFactor - 1) * -1) + 1);
		maxSpeed = (float) (TURN_PWM + ((MAX_PWM - TURN_PWM) * ((((steeringFactor - 1) * -1) + 1) * .5)));
	}
	else{
		//turnFactor = steeringFactor;
		maxSpeed = (float) (TURN_PWM + ((MAX_PWM - TURN_PWM) * (steeringFactor * .5)));
	}
	
	maxSpeed = ((float) turnFactor * maxSpeed);
	
	//Turn off PID differential steering
	right = maxSpeed;
	left = maxSpeed;
	
	//PID servo steering
	dutyCycle = 7.25  + ( -1 *(2 * 1.5 *  (((steeringFactor - 1) ))));

	
	//Handle drifting
	if(steeringFactor < .6){//.7
		//hard right turn
		right = 0;//cut right motor
		left = TURN_PWM;//left motor full turning speed
		//dutyCycle = 9; // hard right turn with servo
		//Drop speed during hard right turns, will have to ramp up back to full speed
		maxSpeed = TURN_PWM;
		turnFactor = .5;

	}
	
	//Keep straight
	if(steeringFactor < 1.05 && steeringFactor > .95){
		dutyCycle = 7.25;
	}
	
	if(steeringFactor > 1.6){//1.5
		//hard left turn
		left = 0;//Cut left motor
		right = TURN_PWM;//right motor full turning speed
		//dutyCycle = 5; //hard left servo turn
	  //Drop speed during hard left turns. Has to ramp up back to full speed
		maxSpeed = TURN_PWM;
		turnFactor = .5;
	}
	
	SetServoDutyCycle(dutyCycle, 50, 0);
	SetDutyCycle(left, right, freq, dir);



	
}

void steeringFunction(uint16_t line[128]){
	
	//used for steering
	int left = 0;
	int right = 0;
	int total = 0;
	
	//loop through line to find the max using a [1/3 1/3 1/3] digital filter
	for(int i = 6; i < 123; i++){
		if( ((i - 2) < 0) || ((i + 2) >= 128)){
			if(maxValue < line[i]){
				maxValue = line[i];
			}
		}
		else{
			uint16_t scaledValue = ((line[i-1] + line[i] + line[i+1])/3);//Will this break because it is only 16 bits?
			if(scaledValue > maxValue){
				maxValue = scaledValue;
			}
		}
	}
	
	//loop through again, scaling the input so that the max is 65535
	for(int i = 6; i < 123; i++){
		

		
		int newValue = (line[i] * (MAX_INPUT / maxValue));
		
		//Can only be max
		if(newValue > maxValue){
			newValue = maxValue;
		}
		
		scaledLine[i] = (uint16_t) newValue;
		
		total += line[i];
		
		if(i < 65){
			left += line[i];
		}
		else{
			right += line[i];
		}
		
	}
	
	float steeringFactor;
	
		steeringFactor = ((float) left) /((float) right);

	
	Steer(steeringFactor);
	
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








