/* PIC16F690 
	This is just an example about how to use lights.
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

// set internal clock speed (8 MHz)
#define _XTAL_FREQ   8000000

int main()
{
    // set internal clock speed (8 MHz)
    OSCCONbits.IRCF = 0b111;

    // disable all analog inputs
    ANSEL = 0;
    ANSELH = 0;

    // setup RC0 as output
    TRISC = 0x0;

    // turn off all lights
    PORTC = 0x0;

    // infinite loop
    for (;;)
    {
        // turn on the first light and wait 0.5 sec
        PORTCbits.RC0 = 1;
        __delay_ms(500);

        // turn on the second light, turn off the first and wait 0.5 sec
        PORTCbits.RC0 = 0;
        PORTCbits.RC1 = 1;
        __delay_ms(500);

        // turn on the third light, turn off the others and wait 0.5 sec
        PORTCbits.RC0 = 0;
        PORTCbits.RC1 = 0;
        PORTCbits.RC2 = 1;
        __delay_ms(500);

        // turn on the fourth light, turn off the others and wait 0.5 sec
        PORTCbits.RC0 = 0;
        PORTCbits.RC1 = 0;
        PORTCbits.RC2 = 0;
        PORTCbits.RC3 = 1;
        __delay_ms(500);
        
        // turn off all lights and wait 0.5 sec
        PORTC = 0x0;
        __delay_ms(500);

        // turn on all lights and wait 1 sec
        PORTC = 0xf;
        __delay_ms(1000);

        // turn off all lights and wait 1 sec
        PORTC = 0x0;
        __delay_ms(1000);

        // turn on all lights and wait 1 sec
        PORTC = 0xf;
        __delay_ms(1000);

        // turn off all lights and wait 1 sec
        PORTC = 0x0;
        __delay_ms(1000);
    }
    
    return 0;
}
