/* PIC16F690 
	This example shows how to handle the timer 0 and its interrupts:
	- The first two lights (RC0 and RC1) are handled by main function
	- The last two lights (RC2 and RC3) are handled by interrupts
	
	Timer is costantly increased, until it reachs its max value (overflow).
	Each time the timer has overflowed, the interrupt function is called.
*/

#include <xc.h>
#include <pic16f690.h>
#include <delays.h>

// CONFIG
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)

// set internal clock speed (4 MHz)
#define _XTAL_FREQ 4000000

// it defines the interrupts frequency
#define T_DELAY (256 - 240)

void interrupt isr();

int main()
{
    // set internal clock speed (4 MHz)
    OSCCONbits.IRCF = 0b110;

    // disable all analog inputs
    ANSEL = 0;
    ANSELH = 0;

    // setup RC0 as output
    TRISC = 0x0;
    
    // enable global interrupt
    INTCONbits.GIE = 1;

    // enable interrupt on timer 0 (TMR0)
    INTCONbits.T0IE = 1;

    // TC0 config
    OPTION_REGbits.T0CS = 0; // Timer0 Clock Source set to internal instruction cycle clock
    OPTION_REGbits.PS = 0b111; // prescaler
    OPTION_REGbits.PSA = 0; // prescaler is assigned to the Timer0 module

    // it's automatically incremented, its speed depends on clock speed
    // once it exceeds its maximum value (256), its overflow interrupt flag bit (T0IF) is set to 1
    TMR0 = T_DELAY;

    // T0IF is the Timer0 Overflow Interrupt Flag bit
    // INTCONbits.T0IF = 1 -> TMR0 register has overflowed and interrupt func is called
    // INTCONbits.T0IF = 0 -> TMR0 register did not overflow
    INTCONbits.T0IF = 0;

    // turn off all lights
    PORTC = 0x0;

    // turn on just one light
    PORTCbits.RC1 = 1;

    // infinite loop
    for (;;)
    {
        // revert first and second light status
        PORTCbits.RC0 = !PORTCbits.RC0;
        PORTCbits.RC1 = !PORTCbits.RC1;
        __delay_ms(500);
    }

    return 0;
}

// is called each time TMR0 reach its max value
void interrupt isr()
{
    if (INTCONbits.T0IF == 1)
    {
        // changes the state of the third and fourth light
        PORTCbits.RC2 = !PORTCbits.RC2;
        PORTCbits.RC3 = !PORTCbits.RC3;

        // reset timer
        TMR0 = T_DELAY;

        // cleaning the Timer0 Overflow Interrupt Flag bit
        // it will be automatically set to 1 once TM0R will be overflowed again
        INTCONbits.T0IF = 0;
    }
}
