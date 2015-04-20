#include "msp430f5510.h"

#define ADC_Channel ADC10INCH_2// ADC PIN P6.2

float ADC_Result;       // ADC Value
_Bool ADC_Ready;        // ADC Done flag


_Bool Timer_Ready = 0;  // Variable to toggle the LEDs w/timer

void measure(unsigned int);
void initTimer(float);

void main(void)
{

	WDTCTL = WDTPW+WDTHOLD;   // Stop WDT
	P1DIR |= BIT0;        // Set P1.0/Green LED to output direction
	P1DIR |= BIT1;        // Set P1.1/Orange LED to output direction
	PJDIR |= 0x0F;        // Set PJ.0 to PJ.3 to output direction
	P4DIR |= 0xFF;        // Set P4 to output direction

	P1OUT |= BIT0;        // Set P1.0 to 1
	P1OUT &= ~BIT1;       // Set P1.1 to 0
	ADC_Ready = 0;        // Variable to indicate ADC is ready
	initTimer(0.2);       // Start the timer clock - receives seconds

	for (;;)
	{
		__delay_cycles(100);    // default is 5000
		measure(ADC_Channel);
		__bis_SR_register(CPUOFF + GIE);// LPM0, ADC10_ISR will force exit

		if (Timer_Ready)        // Timer runs out
		{
			Timer_Ready = 0;
			P1OUT ^= BIT0;      // Toggle LED - XOR
			P1OUT ^= BIT1;      // Toggle LED - XOR
		}
		if (ADC_Ready)          // ADC conversion is ready
		{
			ADC_Ready=0;
			ADC_Result=ADC_Result*3.3/1023; // Convert into Volts

			if (ADC_Result < 0.665)         // All off
			{
				PJOUT = 0x00;
				P4OUT = 0x00;
			}
			else if (ADC_Result < 0.67)     // P4_4 on
			{
				PJOUT = 0x00;
				P4OUT = 0x10;
			}
			else if (ADC_Result < 0.69)     // and P4_5
			{
				PJOUT = 0x00;
				P4OUT = 0x30;
			}
			else if (ADC_Result < 0.73)     // and PJ_2
			{
				PJOUT = 0x04;
				P4OUT = 0x30;
			}
			else if (ADC_Result < 0.79)     // and P4_0
			{
				PJOUT = 0x04;
				P4OUT = 0x31;
			}
			else if (ADC_Result < 0.85)     // and PJ_1
			{
				PJOUT = 0x06;
				P4OUT = 0x31;
			}
			else if (ADC_Result < 0.98)     // and PJ_3
			{
				PJOUT = 0x0e;
				P4OUT = 0x31;
			}
			else                            // and P4_3
			{
				PJOUT = 0x0e;
				P4OUT = 0x39;
			}
		}
	}
}

// Initialize Timer - see from page 474 slau208n
void initTimer(float time)
{
	// Configure Timer
	TA0CCR0 = 32768*time;               // Value for timer to compare
	TA0CTL = TASSEL_1+MC_1+TACLR;       // ACLK, count to CCR0, clear TAR
	TA0CCTL0 = CCIE;                    // Gen interrupt when rollover
}

// ADC configuration - see from page 711 slau208n
void measure(unsigned int channel)
{
	// Configure ADC
	ADC10CTL0 |= ADC10SHT_2 + ADC10ON;  // ADC10ON, S&H=16 ADC clks
	ADC10CTL1 |= ADC10SHP;              // ADCCLK = MODOSC; sampling timer
	ADC10CTL2 |= ADC10RES;              // 10-bit conversion results
	ADC10MCTL0 |= channel;              // A3 ADC input select; Vref=AVCC
	ADC10IE |= ADC10IE0;                // Enable ADC conv complete interrupt
	ADC10CTL0 |= ADC10ENC + ADC10SC;    // Sampling and conversion start
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	ADC_Result = ADC10MEM0;             // Saves measured value.
	ADC_Ready = 1;                      // Sets flag for main loop.
	__bic_SR_register_on_exit(CPUOFF);  // Enable CPU so the main while loop continues
}

// Timer0 A0 interrupt service routine.
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
	Timer_Ready = 1;                        // Time to update
	__bic_SR_register_on_exit(LPM3_bits);   // Exit LPM
}
