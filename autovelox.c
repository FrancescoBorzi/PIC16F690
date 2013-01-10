/*
 * File:   Main.c
 * Author: Gabriele
 *
 * Created on 22 dicembre 2012, 18.26
 */
#define _XTAL_FREQ   8000000

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
int adc, us, ms, s, test, T_DELAY, adc1, adc2 = 0;

void interrupt isr(void) {
   // if (INTCONbits.T0IF == 1) {
    us = us + 250;
            if (us == 1000) {
                us = 0;
                ms++;
                if(ms == 1000) {
                    s++;
                    ms = 0;
                    PORTCbits.RC0 = !PORTCbits.RC0;
                }
                
            }
        TMR0 = T_DELAY;
        INTCONbits.T0IF = 0;
    //}
        
    

//    if (PIR1bits.ADIF == 1) {
//
//        adc2 = adc1;
//        adc1 = adc;
//        adc = (ADRESH << 8) + ADRESL;
//        PIR1bits.ADIF = 0;
//        ADCON0bits.GO = 1;
//    } 
   
    



}
void update() {
        adc2 = adc1;
        adc1 = adc;
        ADCON0bits.GO = 1; // start conversion
        while (ADCON0bits.GO == 1); // wait for end of conversion
            adc = (ADRESH << 8) + ADRESL;
   
}

int main(void) {
    us = ms = s = test = 0;
    
    // frequenza settata a 4Mhz
    OSCCONbits.IRCF = 0b111;
    
    // registri per settare le porte da analogico a digitale, inizialmente le setto tutte a digitale
    // e successivamente mi setto le porte analogiche che mi servono
    ANSEL = 0; 
    ANSELH = 0;
    
    T_DELAY = 256 - 250;
    
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
    ADCON0bits.CHS = 0b0010; // Select channel 
    ADCON0bits.ADON = 1; //enable adc module
    ADCON1bits.ADCS = 0b000; // ADC clock =FOSC/2
   // INTCONbits.PEIE = 1; //enable peripheral interrupt ?
    //abilito l'interrupt del converitore
//    PIR1bits.ADIF = 0;
//    PIE1bits.ADIE = 1;

    
//setto il timer
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PS = 0b000;
    OPTION_REGbits.PSA = 0;
    
//faccio partire il timer
    INTCONbits.T0IF = 0;
    
   // INTCONbits.T0IE = 1; // enable interrupt on timer 0
//abilito l'interrupt generale
    INTCONbits.GIE = 1;



    for (;;) {
        do {
            update();    
        }while((adc + adc1 + adc2) / 3 < 200);
        ADCON0bits.CHS = 0b1010;
        INTCONbits.T0IE = 1;
        //printf("start\n\r");
        adc = adc1 = adc2 = 0;
        
        do {
            update();
        }while ((adc + adc1 + adc2) / 3 < 200);
        INTCONbits.T0IE = 0;
        ADCON0bits.CHS = 0b0010;
        adc = adc1 = adc2 = 0;

        printf("%ds %dms %dus\n\r", s, ms, us);

        ms = s = us = 0;
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        __delay_ms(500);


    }

    return 0;
}
