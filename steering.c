#include "MK64F12.h"
#include <string.h>
#include <stdlib.h>
#include "pwm.h"

#define MAX_INPUT (65535)

int maxValue = 0;

uint16_t scaledLine[128];

int steeringFactor;
int total;

void steeringFunction(uint16_t line[128]){
	
	//used for steering
	int left = 0;
	int right = 0;
	
	//loop through line to find the max using a [1/3 1/3 1/3] digital filter
	for(int i = 0; i < 128; i++){
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
	for(int i = 0; i < 128; i++){
		

		
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
	
	//set the steering factor
	if(right > left){//turn right
		steeringFactor = ((right / total) * 100);
	}
	else{//turn left
		steeringFactor = ((left / total) * -100);
	}
	
}


//Determines how hard the car has to turn
void Steer(){
	
	//default straight
	int dutyCycle = 7;
	
	//LUT for how hard to turn
	if(steeringFactor < -30){
		dutyCycle = 5;
	}
	else if(steeringFactor < -10){
		dutyCycle = 6;
	}
	else if(steeringFactor < 10){
		dutyCycle = 7;
	}
	else if(steeringFactor < 30){
		dutyCycle = 8;
	}
	else{
		dutyCycle = 9;
	}
	
	SetServoDutyCycle(dutyCycle, 50, 0);

	
}










