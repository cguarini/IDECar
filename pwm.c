/*
 * Pulse-Width-Modulation Code for K64
 * PWM signal can be connected to output pins PC3 and PC4
 * 
 * Author: Brent Dimmig <bnd8678@rit.edu>
 * Modified by: 
 * Created: 2/20/2014
 * Modified: 3/07/2015
 */
#include "MK64F12.h"
#include "pwm.h"
#include "Constants.h"

/*From clock setup 0 in system_MK64f12.c*/
#define DEFAULT_SYSTEM_CLOCK 20485760u /* Default System clock value */
#define CLOCK					20485760u
#define PWM_FREQUENCY			10000
#define FTM0_MOD_VALUE			(CLOCK/PWM_FREQUENCY)
#define FTM3_MOD_VALUE (CLOCK/128/50)

static volatile unsigned int PWMTick = 0;

/*
 * Change the Motor Duty Cycle and Frequency
 * @param DutyCycle (0 to 100)
 * @param Frequency (~1000 Hz to 20000 Hz)
 * @param dir: 1 for C4 active, else C3 active 
 */
void SetDutyCycle(unsigned int leftDuty, unsigned int rightDuty, unsigned int Frequency, int dir)
{
	
	if(leftDuty > MAX_PWM){
		leftDuty = MAX_PWM;
	}
	if(rightDuty > MAX_PWM){
		rightDuty = MAX_PWM;
	}
	// Calculate the new cutoff value
	uint16_t modLeft = (uint16_t) (((CLOCK/Frequency) * leftDuty) / 100);
	uint16_t modRight = (uint16_t) (((CLOCK/Frequency) * rightDuty) / 100);
  
	// Set outputs 
	if(dir==1){
		FTM0_C3V = modLeft; FTM0_C2V=0;//left
		FTM0_C0V = modRight; FTM0_C1V=0;//right
	}
  else
    {FTM0_C2V = modLeft; FTM0_C3V=0;
		 FTM0_C1V = modRight; FTM0_C0V=0;}

	// Update the clock to the new frequency
	FTM0_MOD = (CLOCK/Frequency);
}

void SetServoDutyCycle(unsigned int DutyCycle, unsigned int Frequency, int dir){
	// Calculate the new cutoff value
	uint16_t mod = (uint16_t) ((((CLOCK/128)/Frequency) * DutyCycle) / 100);
  
    FTM3_C4V = mod;//PTD1

	// Update the clock to the new frequency
	FTM3_MOD = ((CLOCK/128)/Frequency);

}

/*
 * Initialize the FlexTimer for PWM
 */
void InitPWM()
{
	// 12.2.13 Enable the Clock to the FTM0 Module
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;
	
	// Enable clock on PORT A so it can output
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK;
	
	// 11.4.1 Route the output of FTM channel 0 to the pins
	// Use drive strength enable flag to high drive strength
	//These port/pins may need to be updated for the K64 <Yes, they do. Here are two that work.>
	
    PORTC_PCR3  = PORT_PCR_MUX(4)  | PORT_PCR_DSE_MASK; //Ch2
    PORTC_PCR4  = PORT_PCR_MUX(4)  | PORT_PCR_DSE_MASK;//Ch3
		
		PORTC_PCR1 = PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK;//ch0
		PORTC_PCR2 = PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK;//ch1
	
	// 39.3.10 Disable Write Protection
	FTM0_MODE |= FTM_MODE_WPDIS_MASK;
	
	// 39.3.4 FTM Counter Value
	// Initialize the CNT to 0 before writing to MOD
	FTM0_CNT = 0;
	
	// 39.3.8 Set the Counter Initial Value to 0
	FTM0_CNTIN = 0;
	
	// 39.3.5 Set the Modulo resister
	FTM0_MOD = FTM0_MOD_VALUE;
	//FTM0->MOD = (DEFAULT_SYSTEM_CLOCK/(1<<7))/1000;

	// 39.3.6 Set the Status and Control of both channels
	// Used to configure mode, edge and level selection
	// See Table 39-67,  Edge-aligned PWM, High-true pulses (clear out on match)
	FTM0_C3SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0_C3SC &= ~FTM_CnSC_ELSA_MASK;
	
	// See Table 39-67,  Edge-aligned PWM, Low-true pulses (clear out on match)
	FTM0_C2SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0_C2SC &= ~FTM_CnSC_ELSA_MASK;
	
	// 39.3.6 Set the Status and Control of both channels
	// Used to configure mode, edge and level selection
	// See Table 39-67,  Edge-aligned PWM, High-true pulses (clear out on match)
	FTM0_C0SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0_C0SC &= ~FTM_CnSC_ELSA_MASK;
	
	// See Table 39-67,  Edge-aligned PWM, Low-true pulses (clear out on match)
	FTM0_C1SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0_C1SC &= ~FTM_CnSC_ELSA_MASK;
	
	// 39.3.3 FTM Setup
	// Set prescale value to 1 
	// Chose system clock source
	// Timer Overflow Interrupt Enable
	FTM0_SC = FTM_SC_PS(0) | FTM_SC_CLKS(1); 
	//| FTM_SC_TOIE_MASK;

	// Enable Interrupt Vector for FTM
    //NVIC_EnableIRQ(FTM0_IRQn);

}

/*
 * Initialize the FlexTimer for PWM
 */
void InitServoPWM()
{
	// 12.2.13 Enable the Clock to the FTM0 Module
	SIM_SCGC3 |= SIM_SCGC3_FTM3_MASK;
	
	// Enable clock on PORT A so it can output
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK;
	
	// 11.4.1 Route the output of FTM channel 0 to the pins
	// Use drive strength enable flag to high drive strength
	//These port/pins may need to be updated for the K64 <Yes, they do. Here are two that work.>
	
  PORTC_PCR8  = PORT_PCR_MUX(3)  | PORT_PCR_DSE_MASK; //Ch4
	
	// 39.3.10 Disable Write Protection
	FTM3_MODE |= FTM_MODE_WPDIS_MASK;
	
	// 39.3.4 FTM Counter Value
	// Initialize the CNT to 0 before writing to MOD
	FTM3_CNT = 0;
	
	// 39.3.8 Set the Counter Initial Value to 0
	FTM3_CNTIN = 0;
	
	// 39.3.5 Set the Modulo resister
	FTM3_MOD = FTM3_MOD_VALUE;
	//FTM0->MOD = (DEFAULT_SYSTEM_CLOCK/(1<<7))/1000;

	// 39.3.6 Set the Status and Control of both channels
	// Used to configure mode, edge and level selection
	// See Table 39-67,  Edge-aligned PWM, High-true pulses (clear out on match)
	FTM3_C4SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM3_C4SC &= ~FTM_CnSC_ELSA_MASK;
	
	// 39.3.3 FTM Setup
	// Set prescale value to 1 
	// Chose system clock source
	// Timer Overflow Interrupt Enable
	FTM3_SC = FTM_SC_PS(7) | FTM_SC_CLKS(1); 
	
	//| FTM_SC_TOIE_MASK;

	// Enable Interrupt Vector for FTM
    //NVIC_EnableIRQ(FTM0_IRQn);

}

/*OK to remove this ISR?*/
void FTM0_IRQHandler(void){ //For FTM timer

  FTM0_SC &= ~FTM_SC_TOF_MASK;
  
	//if motor tick less than 255 count up... 
	if (PWMTick < 0xff)
		PWMTick++;
  
	
}

/*OK to remove this ISR?*/
void FTM3_IRQHandler(void){ //For FTM timer

  FTM3_SC &= ~FTM_SC_TOF_MASK;
  
	//if motor tick less than 255 count up... 
	if (PWMTick < 0xff)
		PWMTick++;
}
	
