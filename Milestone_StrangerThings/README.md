# Milestone 1: Stranger Things Light Wall
An addressable RGB LED was constructed which has the ability to produce various colors by alternating the duty cycle of the Red, Green, and Blue components. By changing the duty cycle (brightness) of each node, the 3 main colors could be mixed to produce a wide variety of others. Communication with the RGB LED was accomplished using the MSP430F5529 microcontroller along with its UART features. The LED built for this milestone possesses the ability to communicate with others using the RX and TX UART channels given that the Baud Rate (9600) was consistent across all of the componenets. The overall functionality of the LED inlcudes the ability to receive a package of bytes (any size), take the first three bytes to set the color of the LED, and then pass on the rest of the package so that the next LED can set its color.

# Authors
Nick Scamardi and Nick Setaro are the two contributors to this Milestone Project.

# Dependencies
The library dependency for the code behind the LEDs functionality is the standard `<msp430.h>` library.

# Components
The main components used to build this milestone include:
* TI MSP430F5529 Microcontroller
* RGB LED (Common Anode)
* Code Composer Studio 8.1.0


# Functionality of Milestone1.c Code
Two variables and three functions were defined to begin the code. The two variables used are an integer 'byte', which was initialized to zero, and a volatile unsigned integer 'total', which kept track of the total package or bytes. The three functions used are Timer_Setup, LED_Setup, and UART_Setup. The functions were used to configure PWM using the timer A peripheral, the output pins for the LED nodes, and UART RX and TX for receiving and sending bytes.

The Milestone1.c main function is as follows:
```c
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
```
This portion of the code is responsible for calling the functions, disabling the watchdog timer, and enabling the global interrupt as well as putting the microcontroller into low power mode on the trigger of an interrupt.

Moving on to the individual functions, Timer_Setup was the first function configured. This function setup timer A0 to use SMCLK and put the timer into Up mode using the line `TA0CTL = TASSEL_2 + MC_1 + ID_0 + TACLR`. The 3 capture compare registers, CCR1, CCR2, and CCR3 were set to use OUTMOD3, which is Set/Reset mode. This means that the value was set when the timer reached the value in the respective CCR register and reset when it reached the value in CCR0. The duty cycles were also taken care of in this function, with CCR0 being set to 256, which is the maximum duty cycle or brightness. CCR1, CCR2, and CCR3 were all set to 0 or OFF initially, with the registers being used for the duty cycles of the Red, Green, and Blue nodes respectively.

The LED_Setup configured the output pins used to drive each node on the LED. P1.2, P1.3, and P1.4 were all set to the output direction. P1.2 was connected to the Red node, P1.3 was connected to Green, and P1.4 was connected to Blue. The primary peripheral functionality was also set on each pin in order to enable the Timer A (PWM) on each pin.
```c
void LED_Setup(void)
{
    P1DIR |= BIT2 + BIT3 + BIT4;    // Set P1.2 (Red), P1.3 (Green), and P1.4 (Blue) to output direction
    P1SEL |= BIT2 + BIT3 + BIT4;    // Enables primary peripheral function on P1.2, P1.3, and P1.4 (Timer/PWM)
}
```
The last function definition was the UART_Setup, which configured the UART communication to the LED. The UART TX and RX pins were set using the line `P4SEL |= BIT4 + BIT5`. The other configurations included resetting and intializing the state machine, selecting SMCLK for UART, enabling and clearing interrupt flags, and setting the BAUD Rate to 9600. The BAUD Rate was set by setting the UCA1BR0 register to 104 and the UCA1BR1 register to 0.

The last portion of the Milestone1.c was the interrupt service routine for UART. This was configured using a switch statement with a series of cases. Case 0 sets the total byte integer to the value received, which is stored in the receive-data buffer. The following three cases (case 1, case 2, and case 3) set the duty cycles for the Red, Green, and Blue LEDs. This is done by taking the first three bytes in the initial package that was received. Case three also removes the three bytes that were used from the package. This ensures that the updated package gets passed along so that the next LED will display the correct color. This is accomplished in the following way:
```c
while(!(UCA1IFG & UCTXIFG));    // While there are no interrupt flags
UCA1TXBUF = total - 3;  // Remove the 3 bytes used from the total package/instruction in order to send the correct package
```

The default case sets byte equal to -1 in the case that the byte integer is greater than the total byte count. Otherwise, the package gets passed from the RX register to the TX register so that the package is sent. Byte is incremented by 1 in the interrupt routine as well.
