#include "MK64F12.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "uart.h"
#include "Constants.h"

#define MAX_INPUT (65535)

int maxValue = 0;

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
	
	int dutyCycle = 7;
	int left = MAX_PWM;
	int right = MAX_PWM;
	//LUT for how hard to turn
	if(steeringFactor < .75){
		//soft right turn
		dutyCycle = 8;
	}
	if(steeringFactor < .65){
		right = MAX_PWM - (MAX_PWM * .1);
	}
	if(steeringFactor < .55){
		//right turn
		dutyCycle = 9;
	}
	if(steeringFactor < .45){
		right = MAX_PWM - (MAX_PWM * .4);
		left = MAX_PWM;
	}
	if(steeringFactor < .35){
		//hard right turn
		right = 0;
		left = MAX_PWM - (MAX_PWM * .1);
	}
	if(steeringFactor > 1.35){
		//soft left turn
		dutyCycle = 6;
	}
	if(steeringFactor > 1.45){
		left = MAX_PWM - (MAX_PWM * .1);
	}
	if(steeringFactor > 1.55){
		//left turn
		dutyCycle = 5;
		//left = 0;
	}
	if(steeringFactor > 1.65){
		//left turn
		right = MAX_PWM;
		left = MAX_PWM - (MAX_PWM * .4);
	}
	if(steeringFactor > 1.75){
		//hard left turn
		left = 0;
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
		
		total += scaledLine[i];
		
		if(i < 65){
			left += scaledLine[i];
		}
		else{
			right += scaledLine[i];
		}
		
	}
	
	float steeringFactor;
	
	steeringFactor = ((float) left) /((float) right);
	
	Steer(steeringFactor);
	
}













