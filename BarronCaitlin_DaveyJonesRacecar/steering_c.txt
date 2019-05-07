#include "MK64F12.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "uart.h"
#include "steering.h"

#define MAX_INPUT (65535)
#define STRAIGHT (1)




int maxValue = 0;
float maxSpeed = 0;
float rampUp = 0;


uint16_t scaledLine[128];

int left;

//Enable printing to console
int steeringDebug = 0;

uint16_t freq = 10000; /* Frequency = 10 kHz */
uint16_t dir = 1;


/*
*Helper function to determine how hard the car should turn
*@param:	steeringFactor - Value calculated from linescan camera data, 1 is straight, <1 is right >1 is left
*					MAX_PWM - Max speed of the car during straight sections, set in Constants.h
*					TURN_PWM - Speed the car takes turns, set in Constants.h
*/
void Steer(float steeringFactor, int MAX_PWM, int TURN_PWM){
	
	//Prints steering factor to console
	if(steeringDebug){
		char str[100];
		sprintf(str, "%f", steeringFactor);
		put(str);
		put("\n\r");
	}
	
	
	//default values
	float dutyCycle = 7.25;//Servo is straight
	//both motors at max speed
	int left = MAX_PWM;
	int right = MAX_PWM;
	
	//Flips the steering factor: e.g. .7 -> 1.3; 1.2 -> .8
	float normalizedSteeringFactor = (((steeringFactor - 1) * -1) + 1);

	
	//Initialize value
	if (rampUp == 0){
		rampUp = (float) (.5);
	}
	
	
	//Ramp up speed as car goes straight
	if(rampUp < 1.0){
		rampUp += (float) ((normalizedSteeringFactor )* .01);
	}
	//Cannot be larger than MAX_PWM
	if(rampUp > 1.0){
		rampUp = 1.0;
	}
	//Cannot be smaller than 1/4 TURN_PWM
	if(rampUp < .25){
		rampUp = .25;
	}
	
	
	// Variable speed depending on how far from center of track, Straighter goes faster
	if(steeringFactor > 1){
		maxSpeed = (float) (TURN_PWM + ((MAX_PWM - TURN_PWM) * ((((steeringFactor - 1) * -1) + 1) * .5)));
	}
	else{
		maxSpeed = (float) (TURN_PWM + ((MAX_PWM - TURN_PWM) * (steeringFactor * .5)));
	}
	
	//Apply the rampUp variable
	maxSpeed = ((float) rampUp * maxSpeed);
	
	//apply maxSpeed to both motors
	right = maxSpeed;
	left = maxSpeed;
	
	//PID servo steering
	//7.25 is straight, applied when steeringFactor ~1
	//Variable from 3.75 to 10.75
	dutyCycle = 7.25  + ( -1 *(2 * 1.75 *  (((steeringFactor - 1) ))));


	/*
	*If the car is taking a sharp turn or too close to the left or right side of the track, it will cut the
	*corresponding motor in order to help with steering. This causes the car to drift at high speeds.
	*/
	
	//Handle drifting to the right
	if(steeringFactor < .6){
		//hard right turn
		right = 0;//cut right motor
		left = TURN_PWM;//left motor full turning speed
		//Drop speed during hard right turns, will have to ramp up back to full speed
		maxSpeed = TURN_PWM;
		//Ramp back up to speed after a hard turn
		rampUp = .5;

	}
	
	//Keep straight, controls for noise in the camera
	if(steeringFactor < 1.05 && steeringFactor > .95){
		dutyCycle = 7.25;
	}
	
	//Handle drifiting to the left
	if(steeringFactor > 1.6){
		//hard left turn
		left = 0;//Cut left motor
		right = TURN_PWM;//right motor full turning speed
	  //Drop speed during hard left turns. Has to ramp up back to full speed
		maxSpeed = TURN_PWM;
		//Ramp back up to speed after a hard turn
		rampUp = .5;
	}
	
	//Set servo and motor values
	SetServoDutyCycle(dutyCycle, 50, 0);
	SetDutyCycle(left, right, freq, dir);

}

void steeringFunction(uint16_t line[128], int MAX_PWM, int TURN_PWM){
	
	//Values that hold the integration of the left and right hand of the camera
	int left = 0;
	int right = 0;
		
	//Loop through the line output by the camera, integrating the left and right hand sides
	for(int i = 6; i < 123; i++){
		
		//Lefthand integration
		if(i < 65){
			left += line[i];
		}
		//righthand integration
		else{
			right += line[i];
		}
	}
	
	//Divide the left by the right, resulting in a factor that will be 1 when they are equal,
	// <1 when left is larger and >1 when right is larger.
	float steeringFactor = ((float) left) /((float) right);

	//Call the helper function Steer() to determine steering based on the steeringFactor.
	Steer(steeringFactor, MAX_PWM, TURN_PWM);
	
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








