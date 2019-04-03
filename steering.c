#include "MK64F12.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "uart.h"

#define MAX_INPUT (65535)

int maxValue = 0;

uint16_t scaledLine[128];

float steeringFactor;
int left;


//Determines how hard the car has to turn
void Steer(){
	/*
	char str[100];
	sprintf(str, "%f", steeringFactor);
	put(str);
	put("\n");
	*/
	//default straight
	int dutyCycle = 7;
	/*
	//LUT for how hard to turn
	if(steeringFactor < -5){
		dutyCycle = 6;
	}
	else if(steeringFactor < -1){
		dutyCycle = 7;
	}
	else if(steeringFactor < 1){
		dutyCycle = 8;
	}
	else if(steeringFactor < 5){
		dutyCycle = 9;
	}
	else{
		dutyCycle = 10;
	}
	*/
	if(steeringFactor < .85){
		dutyCycle = 9;
	}
	else if(steeringFactor > 1.3){
		dutyCycle = 5;
	}
	else{
		dutyCycle = 7;
	}
	SetServoDutyCycle(dutyCycle, 50, 0);

	
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
	
	steeringFactor = ((float) left) /((float) right);
	
	Steer();
	
}













