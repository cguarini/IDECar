;/*
 * Rochester Institute of Technology
 * Department of Computer Engineering
 * CMPE 460  Interfacing Digital Electronics
 * Fall 2018
 * Authors: Michael Baumgarten & Matthew Toro
 *
 * PIT0 determines the integration period
 * FTM2 handles the camera clock/logic
 * ADC0 Converts the analog camera output
 * GPIO PTB9			- Camera CLK
 *			PTB23			- Camera SI
 *			ADC0_DP0	- Camera In
 *
 * Filename: camera.c
 */

#include "camera.h"
#include "MK64F12.h"
#include <string.h>
#include <stdio.h>
#include "uart.h"

void initPIT(void);
void initGPIO(void);
void initADC0(void);
void initFTM2(void);

// These variables are for streaming the camera
//	 data over UART
int debugcamdata = 1;
int capcnt = 0;
char str[100];

// ADC0VAL holds the current ADC value
uint16_t ADC0VAL;

// Pixel counter for camera logic
// Starts at -2 so that the SI pulse occurs
int pixcnt = -2;

// line stores the current array of camera data
uint16_t line[128];

int lineready = 0;

/*
 * Fetches the latest data from the camera
 * @param data The integer array to put the new data into
 */
void getLine(uint16_t* data)
{
	while(lineready == 0)
	{
		//Wait
	}
	memcpy(data, line, 128*sizeof(uint16_t));
}


/* 
 * Initializes all of the peripherals needed to drive the camera
 */
 void initCamera(void)
{
	initPIT();
	initGPIO();
	initADC0();
	initFTM2();
}


void initPIT(void)
{
	/* Initalize PIT0 */
	// Enable clock for timers
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
	
	// Enabled clock for standard PIT timers
	PIT_MCR &= ~PIT_MCR_MDIS_MASK;
	
	// Enable timers to continue in debug mode
	PIT_MCR &= ~PIT_MCR_FRZ_MASK; // In case you need to debug
	
	// PIT clock frequency is the system clock
	// Load the value that the timer will count down from
	PIT_LDVAL0 = SYSTEM_CLOCK * INTEGRATION_TIME;
	
	// Enable timer interrupts
	PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;
	
	// Enable the timer
	PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;

	// Clear interrupt flag
	PIT_TFLG0 |= PIT_TFLG_TIF_MASK;

	// Enable PIT interrupt in the interrupt controller
	NVIC_EnableIRQ(PIT0_IRQn);
}


void initGPIO(void)
{
	/* Initalize GPIO */
	// Turn on clock for PORTB
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	
	// Configure Port Mux for GPIO
	PORTB_PCR9 	|= PORT_PCR_MUX(1);
	PORTB_PCR23 |= PORT_PCR_MUX(1);
	PORTB_PCR22 |= PORT_PCR_MUX(1);
	
	// Set GPIO pins to output mode
	GPIOB_PDDR |= CAM_CLK | CAM_SI;
	
}

void initADC0(void)
{
	/* Initalize ADC */
	unsigned int calib;
	// Turn on ADC0
	SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK;
	
	// Single ended 16 bit conversion, no clock divider
	ADC0_CFG1 |= ADC_CFG1_MODE(3); //MODE = 11b

	// Do ADC Calibration for Singled Ended ADC. Do not touch.
	ADC0_SC3 = ADC_SC3_CAL_MASK;
	while ( (ADC0_SC3 & ADC_SC3_CAL_MASK) != 0 );
	calib = ADC0_CLP0; calib += ADC0_CLP1; calib += ADC0_CLP2;
	calib += ADC0_CLP3; calib += ADC0_CLP4; calib += ADC0_CLPS;
	calib = calib >> 1; calib |= 0x8000;
	ADC0_PG = calib;
	
	// Select hardware trigger.
	ADC0_SC2 |= ADC_SC2_ADTRG_MASK;
	
	// Set to single ended mode	
	ADC0_SC1A &= ~ADC_SC1_DIFF_MASK; //DIFF = 0 (Single ended)'

	// Set to DADP0
	ADC0_SC1A &= ~ADC_SC1_ADCH(0x1F);

	// Set up FTM2 trigger on ADC0
	SIM_SOPT7 = 0; //Clear Register
	SIM_SOPT7 |= SIM_SOPT7_ADC0TRGSEL(10); // FTM2 select (1010b)
	SIM_SOPT7 |= SIM_SOPT7_ADC0ALTTRGEN_MASK; // Alternative trigger en.
	SIM_SOPT7 &= ~SIM_SOPT7_ADC0PRETRGSEL_MASK; // Pretrigger A
	
	//Enable ADC interrupts
	ADC0_SC1A |= ADC_SC1_AIEN_MASK;
	
	// Enable NVIC interrupt
	NVIC_EnableIRQ(ADC0_IRQn);
}

void initFTM2(void)
{
	/* Initalize FTM2 */
	// Enable clock
	SIM_SCGC6 |= SIM_SCGC6_FTM2_MASK;

	// Disable Write Protection
	FTM2_MODE |= FTM_MODE_WPDIS_MASK;
	
	// Set output to '1' on init
	FTM2_OUTINIT |= FTM_OUTINIT_CH0OI_MASK;
	
	// Initialize the CNT to 0 before writing to MOD
	FTM2_CNT = 0;
	
	// Set the Counter Initial Value to 0
	FTM2_CNTIN = 0;
	
	// Set the period (~10us)
	FTM2_MOD = 10*(SYSTEM_CLOCK/1000000);
	
	// 50% duty
	FTM2_C0V = (10*(SYSTEM_CLOCK/1000000)) / 2;
	
	// Set edge-aligned mode
	FTM2_C0SC |= FTM_CnSC_MSB_MASK;
	
	// Enable High-true pulses
	// ELSB = 1, ELSA = 0
	FTM2_C0SC |= FTM_CnSC_ELSB_MASK;
	FTM2_C0SC &= ~FTM_CnSC_ELSA_MASK;
	
	// Enable hardware trigger from FTM2
	FTM2_EXTTRIG |= FTM_EXTTRIG_CH0TRIG_MASK;
	
	// Don't enable interrupts yet (disable)
	FTM2_SC &= ~FTM_SC_TOIE_MASK;
	
	// No prescalar, system clock
	FTM2_SC &= ~FTM_SC_PS(0);
	FTM2_SC |= FTM_SC_CLKS(1);
	
	// Set up interrupt
	NVIC_EnableIRQ(FTM2_IRQn);
}



/* ADC0 Conversion Complete ISR  */
void ADC0_IRQHandler(void) {
	// Reading ADC0_RA clears the conversion complete flag
	ADC0VAL = ADC0_RA;
}

/* PIT0 determines the integration period
 * When it overflows, it triggers the clock logic from
 * FTM2. Note the requirement to set the MOD register
 * to reset the FTM counter because the FTM counter is 
 * always counting, I am just enabling/disabling FTM2 
 * interrupts to control when the line capture occurs
 */
void PIT0_IRQHandler(void){
	
	// Clear interrupt
	PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
	
	// Setting mod resets the FTM counter
	FTM2_MOD = 10*(SYSTEM_CLOCK/1000000);
	
	// Enable FTM2 interrupts (camera)
	FTM2_SC |= FTM_SC_TOIE_MASK;
	
	return;
}

/* FTM2 handles the camera driving logic
 * This ISR gets called once every integration period
 * by the periodic interrupt timer 0 (PIT0)
 * When it is triggered it gives the SI pulse,
 * toggles clk for 128 cycles, and stores the line
 * data from the ADC into the line variable
 */
void FTM2_IRQHandler(void){ //For FTM timer
	
	//Line is not ready to read
	lineready = 0;
	
	// Clear interrupt
	FTM2_SC &= ~FTM_SC_TOF_MASK;
	
	// Toggle clk
	GPIOB_PTOR |= CAM_CLK;
	
	// Line capture logic
	if ((pixcnt >= 2) && (pixcnt < 256)) 
	{
		if (!(GPIOB_PDOR & CAM_CLK)) // check for falling edge
		{	
			// ADC read (note that integer division is 
			//  occurring here for indexing the array)
			line[pixcnt/2] = ADC0VAL;
		}
		pixcnt += 1;
	} 
	else if (pixcnt < 2) 
	{
		if (pixcnt == -1) 
		{
			GPIOB_PSOR |= CAM_SI; // SI = 1
		} 
		else if (pixcnt == 1) 
		{
			GPIOB_PCOR |= CAM_SI; // SI = 0
			// ADC read
			line[0] = ADC0VAL;
		} 
		pixcnt += 1;
	} 
	else // After Reading Line
	{
		GPIOB_PCOR |= CAM_CLK; // CLK = 0
		pixcnt = -2; // reset counter
		// Disable FTM2 interrupts (until PIT0 overflows
		//   again and triggers another line capture)
		FTM2_SC &= ~FTM_SC_TOIE_MASK;
		
		//Line is ready to read
		lineready = 1;
	}
	
	return;
}


int camera_debug_main(void)
{
	int i;
	
	uart_init();
	initGPIO(); // For CLK and SI output on GPIO
	initFTM2(); // To generate CLK, SI, and trigger ADC
	initADC0();
	initPIT();	// To trigger camera read based on integration time
	
	for(;;) {

		if (debugcamdata) {
			// Every 2 seconds
			//if (capcnt >= (2/INTEGRATION_TIME)) {
			if (lineready) {
				GPIOB_PCOR |= (1 << 22);
				// send the array over uart
				sprintf(str,"%i\n\r",-1); // start value
				put(str);
				for (i = 0; i < 127; i++) {
					sprintf(str,"%i\n", line[i]);
					put(str);
				}
				sprintf(str,"%i\n\r",-2); // end value
				put(str);
				capcnt = 0;
				GPIOB_PSOR |= (1 << 22);
			}
		}

	} //for
} //main

