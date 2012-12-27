/*
 * File:   Main.c
 * Author: Gabriele
 *
 * Created on 22 dicembre 2012, 18.26
 */
#define _XTAL_FREQ   4000000

#include <xc.h>
#include <pic16f690.h>
#include <delays.h>
#include <stdio.h>
// PIC16F690 Configuration Bit Settings

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

//ridefinisco la prinf

void putch(unsigned char byte) {
    /* output one byte */
    TXREG = byte;
    while (!TXSTAbits.TRMT); /* set when register is empty */
}
int adc, us, ms, s, test, T_DELAY = 0;

void interrupt isr(void) {

    if (PIR1bits.ADIF == 1) {

        adc = (ADRESH << 8) + ADRESL;
       // PORTCbits.RC0 = !PORTCbits.RC0;
        PIR1bits.ADIF = 0;
        ADCON0bits.GO = 1;
        // printf("test1\n\r");

    }

        if (INTCONbits.T0IF == 1) {
            TMR0 = T_DELAY;
            //PORTCbits.RC1 = !PORTCbits.RC1;
            if(test) {
                 ms = ms + 65;
                 if(ms>1000) {
                     ms = 0;
                     s++;
          //           PORTCbits.RC0 = !PORTCbits.RC0;
                 }
            }
            INTCONbits.T0IF = 0;
       }

}

int main(void) {
    us = ms = s = test = 0;
    //frequenza settata a 4Mhz
    OSCCONbits.IRCF = 0b110;
    //registri per settare le porte da analogico a digitale, inizialmente le setto tutte a digitale
    ANSEL = 0; //e successivamente mi setto le porte analogiche che mi servono
    ANSELH = 0;
    T_DELAY = 0;
    /* configure digital I/O */
    TRISCbits.TRISC0 = 0;
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;
    TRISBbits.TRISB7 = 0; // TX PIN set to output
    TRISBbits.TRISB5 = 1; // RX PIN set to input
    // setup UART transmitter
    TXSTAbits.TX9 = 0; // 8 bit data
    TXSTAbits.TXEN = 1; // enable transmitter
    TXSTAbits.BRGH = 1; // high speed transmission

    // setup UART receiver
    RCSTAbits.SPEN = 1; // enable serial port
    RCSTAbits.RX9 = 0; // 8 bit data
    RCSTAbits.CREN = 1; // enable receiver

    // baud rate generator control
    BAUDCTLbits.BRG16 = 1; // 16 bit baud rate generator

    // baud rate generator value
    SPBRGH = 0;
    SPBRG = 207;

    PORTC = 0x00;

    /* configure ADC */
    ANSELbits.ANS2 = 1; //setto le porte
    ANSELHbits.ANS10 = 1;

    ADCON0bits.ADFM = 1; // right justified
    ADCON0bits.VCFG = 0; // Reference = VDD
    // Select channel 0
    ADCON0bits.CHS = 0b0010;
    // turn adc ok

    ADCON1bits.ADCS = 0b110; // ADC clock = FOSC/64
    INTCONbits.PEIE = 1;
    //abilito l'interrupt del converitore
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;

    ADCON0bits.ADON = 1;
    ADCON0bits.GO = 1;

     OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PS = 0b111;
    OPTION_REGbits.PSA = 0;

    INTCONbits.T0IF = 0;
    INTCONbits.T0IE = 1; // enable interrupt on timer 0

    INTCONbits.GIE = 1;


    for (;;) {
        adc = 0;
        while(adc < 400);
        ADCON0bits.CHS = 0b1010;
        printf("start\n\r");
       
        test = 1;
        adc = 0;
        while (adc < 400);
        ADCON0bits.CHS = 0b0010;
        test = 0;
        printf("%ds %dms\n\r",s,ms);
       
        ms = s = 0;
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        __delay_ms(1000);

     
    }

    return 0;
}
