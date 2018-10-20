/* Authors: Nick Scamardi & Nick Setaro
 * Written: October 10, 2018
 * Last Update: October 18, 2018
 */

#include <msp430.h>

int byte = 0;   // Define and initialize 'byte' variable to 0 (Keeps track of the current number of bytes)
volatile unsigned int total;    // Define variable for the total bytes received

void Timer_Setup(void); // Function for Timer/PWM configuration
void LED_Setup(void);   // Function for LED configuration
void UART_Setup(void);  // Function for UART configuration

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// Disable the watchdog timer
	
	// Function Declarations
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

    TA0CCTL1 |= OUTMOD_3;   // Capture/Compare 1 Control: OUTMOD3 - Set/Reset Mode (Set when the timer reaches value in CCR1 and reset when it reaches CCR0 value)
    TA0CCTL2 |= OUTMOD_3;   // Capture/Compare 2 Control: OUTMOD3 - Set/Reset Mode (Set when the timer reaches value in CCR2 and reset when it reaches CCR0 value)
    TA0CCTL3 |= OUTMOD_3;   // Capture/Compare 3 Control: OUTMOD3 - Set/Reset Mode (Set when the timer reaches value in CCR3 and reset when it reaches CCR0 value)

    TA0CCR0 = 256;  // Maximum duty cycle (brightness) for any color on the RGB LED
    TA0CCR1 = 0;    // Initialize CCR1 (Red) to duty cycle = 0 (OFF)
    TA0CCR2 = 0;    // Initialize CCR2 (Green) to duty cycle = 0 (OFF)
    TA0CCR3 = 0;    // Initialize CCR3 (Blue) to duty cycle = 0 (OFF)
}

void LED_Setup(void)
{
    P1DIR |= BIT2 + BIT3 + BIT4;    // Set P1.2 (Red), P1.3 (Green), and P1.4 (Blue) to output direction
    P1SEL |= BIT2 + BIT3 + BIT4;    // Enables primary peripheral function on P1.2, P1.3, and P1.4 (Timer/PWM)
}

void UART_Setup(void)
{
    P4SEL |= BIT4 + BIT5;   // Configures UART on P4.4 (TX) and P4.5 (RX)

    UCA1CTL1 |= UCSWRST;    // Reset state machine
    UCA1CTL1 |= UCSSEL_2;   // Selects SMCLK for UART clock
    UCA1BR0 = 104;          // Sets BAUD Rate to 9600
    UCA1BR1 = 0;            // Sets BAUD Rate to 9600
    UCA1MCTL |= UCBRS_1 + UCBRF_0;  // Sets first and second stage modulation patterns
    UCA1CTL1 &= ~UCSWRST;   // Initialize USCI state machine
    UCA1IE |= UCRXIE;       // Enable USCI_A0 RX interrupt
    UCA1IFG &= ~UCRXIFG;    // Reset/Clear interrupt flags

}

// Interrupt service routine UART
#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch(byte)
    {
    case 0:
            total = UCA1RXBUF;  // Sets the total byte count to the data stored in the receive-data buffer (package received)
            break;
    case 1:
            TA0CCR1 = UCA1RXBUF;    // Sets the duty cycle for the Red LED using the first byte in package
            break;
    case 2:
            TA0CCR2 = UCA1RXBUF;    // Sets the duty cycle for the Green LED using the second byte in package
            break;
    case 3:
            TA0CCR3 = UCA1RXBUF;    // Sets the duty cycle for the Blue LED using the third byte in package

            while(!(UCA1IFG & UCTXIFG));    // While there are no interrupt flags
            UCA1TXBUF = total - 3;  // Remove the 3 bytes used from the total package/instruction in order to send the correct package
            break;

    default:
            if(byte > total)
            {
                byte = -1;  // If byte count is greater than the total bytes, set byte to -1.
            }
            else
            {
                while(!(UCA1IFG & UCTXIFG));
                UCA1TXBUF = UCA1RXBUF;  // Send/Transmit the updated package to the RX pin in order to send to the next board
            }
            break;
    }

    byte += 1;  // Increment the byte variable by 1
}
