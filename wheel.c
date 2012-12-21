/* PIC16F690 
	This software allows to increase and decrease the intensity of the lights by using the wheel.
	Each period is divided into two phases: T_on (light) and t_off (light off), inversely proportional to each other.
	More t_on is high, and the more we have the impression that the light is more intense (and vice versa).
	It uses interrupts on TMR0.
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
#pragma config BOREN = ON       // Brown-out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)

#define _XTAL_FREQ   4000000
#define OFF   0
#define ON    1

unsigned char period, t_on, t_off, status = 0;

// is called each time TMR0 reach its max value
void interrupt isr(void)
{
    if (INTCONbits.T0IF == 1) {
        if (status == ON) {
            /* We should turn LED on */
            if (t_on == 0) {
                status = OFF;
                TMR0 = -t_off;
            }
            else {
                TMR0 = -t_on;
            }
        }
        else {
            /* We should turn LED off */
            if (t_off == 0) {
                status = ON;
                TMR0 = -t_on;
            }
            else {
                TMR0 = -t_off;
            }
        }

        PORTCbits.RC0 = status;
        PORTCbits.RC1 = status;
        PORTCbits.RC2 = status;
        PORTCbits.RC3 = status;

        status = !status;
        INTCONbits.T0IF = 0;
    }
}

int main(void) {

    int percentage, tmp;

    OSCCONbits.IRCF = 0b110;
	
    ANSEL = 0; // all pin are assigned as digital
    ANSELbits.ANS0 = 1; // pin is assigned as analog input
    ANSELH = 0;

    ADCON0bits.CHS = 0; // channel selection

    VCFG = 0;
    ADFM = 1;
    
    ADCON1bits.ADCS = 101; // set speed
    
    ADON = 1;

    TRISCbits.TRISC0 = 0;
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;

    PORTCbits.RC0 = 0;
    PORTCbits.RC1 = 0;
    PORTCbits.RC2 = 0;
    PORTCbits.RC3 = 0;

    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PS = 0b100;
    OPTION_REGbits.PSA = 0;

    period = 200;

    TMR0 = period;
    INTCONbits.T0IF = 0;
    
    // enable global interrupt
    INTCONbits.GIE = 1;
    
    // enable interrupt on timer 0
    INTCONbits.T0IE = 1;

    percentage = 1;

    for (;;)
    {
    	// fetching wheel input
        ADCON0bits.GO = 1;
        while (ADCON0bits.GO == 1);
        tmp = ADRESH * 256 + ADRESL;

	// converting wheel input
        percentage = tmp * 10 / 102;
	
	// calculating intensity
        t_on = (period * percentage)/100;
        t_off = (period * (100 - percentage))/100;
        __delay_ms(50);
    }

    return 0;
}
