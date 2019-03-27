/*
 * File:        uart.c
 * Purpose:     Provide UART routines for serial IO
 *
 *
 */

#include "MK64F12.h"
#include <string.h>
#include <stdio.h>
#define BAUD_RATE 9600      //default baud rate 
#define SYS_CLOCK 20485760 //default system clock (see DEFAULT_SYSTEM_CLOCK  in system_MK64F12.c)

void uart_init()
{
	//define variables for baud rate and baud rate fine adjust
	uint16_t ubd, brfa;

	//Enable clock for UART
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;

	//Configure the port control register to alternative 3 (which is UART mode for K64)
	PORTB_PCR16 = PORT_PCR_MUX(3);
	PORTB_PCR17 = PORT_PCR_MUX(3);

	/*Configure the UART for establishing serial communication*/
	//Disable transmitter and receiver until proper settings are chosen for the UART module
	UART0_C2 &= (0 << UART_C2_TE_SHIFT);
	UART0_C2 &= (0 << UART_C2_RE_SHIFT);
	
	//Select default transmission/reception settings for serial communication of UART by clearing the control register 1
	UART0_C1 = 0x00;


	//UART Baud rate is calculated by: baud rate = UART module clock / (16 × (SBR[12:0] + BRFD))
	//13 bits of SBR are shared by the 8 bits of UART3_BDL and the lower 5 bits of UART3_BDH 
	//BRFD is dependent on BRFA, refer Table 52-234 in K64 reference manual
	//BRFA is defined by the lower 4 bits of control register, UART0_C4 

	//calculate baud rate settings: ubd = UART module clock/16* baud rate
	ubd = (uint16_t)((SYS_CLOCK)/(BAUD_RATE * 16));  //ubd = 133.371

	//clear SBR bits of BDH
	UART0_BDH &= ~UART_BDH_SBR_MASK;

	//distribute this ubd in BDH and BDL
	UART0_BDH = (ubd >> 8) & UART_BDH_SBR_MASK;
	UART0_BDL = ubd & UART_BDL_SBR_MASK;

	//BRFD = (1/32)*BRFA 
	//make the baud rate closer to the desired value by using BRFA
	brfa = (((SYS_CLOCK*32)/(BAUD_RATE * 16)) - (ubd * 32)); //brfa = 0.001066666

	//write the value of brfa in UART0_C4
	UART0_C4 = brfa & UART_C4_BRFA_MASK;
		
	//Enable transmitter and receiver of UART
	UART0_C2 |= (1 << UART_C2_TE_SHIFT);
	UART0_C2 |= (1 << UART_C2_RE_SHIFT);

}

uint8_t uart_getchar()
{
	/* Wait until there is space for more data in the receiver buffer*/
	while(!(UART0_S1 & UART_S1_RDRF_MASK));

	/* Return the 8-bit data from the receiver */
	return (UART0_D);
}

void uart_putchar(char ch)
{
	/* Wait until transmission of previous bit is complete */
	while(!(UART0_S1 & UART_S1_TDRE_MASK));
		
	/* Send the character */
	UART0_D = ch;
}

void put(char *ptr_str){
	/*use putchar to print string*/
	while(*ptr_str)
	{
		uart_putchar(*ptr_str++);
	}
}

void putnumU(int i){
	char str[500];
	sprintf(str, "%d", i);
	put(str);
}

void LED_init()
{
	// Enable clocks on Ports B and E for LED timing
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;		//Enables Port B mask
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;		//Enables Port E mask
	
	// Configure the Signal Multiplexer for GPIO
	PORTB_PCR22 = PORT_PCR_MUX(1);				//Enables GPIO mode for PTB22
	PORTE_PCR26 = PORT_PCR_MUX(1);				//Enables GPIO mode for PTE26
	PORTB_PCR21 = PORT_PCR_MUX(1);				//Enables GPIO mode for PTB21
	
	// Switch the GPIO pins to output mode
	GPIOB_PDDR = (1 << 22) | (1 << 21);		//Set PTB22 and PTB21 to output
	GPIOE_PDDR = (1 << 26);								//Set PTE26 to output

	// Turn off the LEDs
  GPIOB_PDOR = (1 << 22) | (1 << 21);		//Init PTB22 and PTB21 LEDs to off
	GPIOE_PDOR = (1 << 26);								//Init PTE26 LED to off
}

void Button_Init(void)
{
	// Enable clock for Port C PTC6 button
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;		//Enables Port C mask
	
	// Configure the Mux for the button
	PORTC_PCR6 = PORT_PCR_MUX(1);					//Enables GPIO mode for PTC6

	// Set the push button as an input
	GPIOC_PDDR = (0 << 6);;								//Set PTC6 to input
	
}

