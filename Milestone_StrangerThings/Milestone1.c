/* Authors: Nick Scamardi & Nick Setaro
 * Written: October 10, 2018
 * Last Update: October 17, 2018
 */

#include <msp430.h>

int byte = 0;
volatile unsigned int total;

void Timer_Setup(void);
void LED_Setup(void);
void UART_Setup(void);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// Disable the watchdog timer
	
	Timer_Setup();
	LED_Setup();
	UART_Setup();

	__bis_SR_register(LPM0_bits + GIE); // Enter Low Power Mode once interrupt is detected & Global Interrupt Enable
    __enable_interrupt();   // Enable interrupt algorithm

    for(;;){}; // Infinite for loop
}

void Timer_Setup(void)
{
    TA0CTL = TASSEL_2 + MC_1 + ID_0 + TACLR;   // Timer A0 Control: SMCLK (~ 1MHz), Up mode, No Division, Clear timer

    TA0CCTL1 |= OUTMOD_3;   // Out Mode 3:
    TA0CCTL2 |= OUTMOD_3;
    TA0CCTL3 |= OUTMOD_3;

    TA0CCR0 = 256;
    TA0CCR1 = 0;
    TA0CCR2 = 0;
    TA0CCR3 = 0;
}

void LED_Setup(void)
{
    P1DIR |= BIT2 + BIT3 + BIT4;
    P1SEL |= BIT2 + BIT3 + BIT4;
}

void UART_Setup(void)
{
    P4SEL |= BIT4 + BIT5;
    P3SEL |= BIT3 + BIT4;

    UCA1CTL1 |= UCSWRST;                    //reset state machine
    UCA1CTL1 |= UCSSEL_2;                   //SMCLK for UART
    UCA1BR0 = 104;                          //
    UCA1BR1 = 0;                            //
    UCA1MCTL |= UCBRS_1 + UCBRF_0;          //Modulation
    UCA1CTL1 &= ~UCSWRST;                   //Initialize USCI state machine
    UCA1IE |= UCRXIE;                       //Enable USCI_A0 RX interrupt
    UCA1IFG &= ~UCRXIFG;                                        //Reset/Clear interrupt flags*/

}

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch(byte)
    {
    case 0:
            total = UCA1RXBUF;
            break;
    case 1:
            TA0CCR1 = UCA1RXBUF;
            break;
    case 2:
            TA0CCR2 = UCA1RXBUF;
            break;
    case 3:
            TA0CCR3 = UCA1RXBUF;
            while(!(UCA1IFG & UCTXIFG));
            UCA1TXBUF = total - 3;
            break;

    default:
            if(byte >= total)
            {
                byte = -1;
            }
            else
            {
                while(!(UCA1IFG & UCTXIFG));
                UCA1TXBUF = UCA1RXBUF;
            }
            break;
    }

    byte += 1;
}
