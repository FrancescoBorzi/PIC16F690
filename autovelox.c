#include <xc.h>
#include <pic16f690.h>
#include <delays.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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


#define _XTAL_FREQ   8000000
#define T_DELAY 1
#define FACTOR 0.48

unsigned int adc[3]; // contengono il valore della misura della distanza
unsigned int distance; // massima distanza tollerata
unsigned int distance2; // distanza fra i sensori
unsigned int distanceEff; // distanza effettiva misurata
unsigned int volt; //volt rivelati dal sensore
unsigned int ms; // tempo misurato in millisecondi
int mode; // unita' di misura della velocita'


// ridefinisco la printf

void putch(unsigned char byte) {
    /* output one byte */
    TXREG = byte;
    while (!TXSTAbits.TRMT); /* set when register is empty */
}

void interrupt isr(void) {
    // scatta ogni millisecondo
    if (INTCONbits.T0IF == 1) {
        ms++;

        TMR0 = T_DELAY;

        INTCONbits.T0IF = 0;
    }
}

void settaggi() {
    int restart = 0;

    do {
        // variabili di appoggio
        char c, num[3];
        int x, count;

        printf("-----------Autovelox v1.0--------------\n\r");
        printf("               Setup                    \n\r");
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        printf("SELEZIONARE LA DISTANZA DI RIVELAMENTO\n\r");
        printf("\n\r");
        printf("Press 1 to 10cm\n\r");
        printf("Press 2 to 20cm\n\r");
        printf("Press 3 to 30cm\n\r");
        printf("Press 4 to 40cm\n\r");


        // aspetta un input da tastiera
        while (PIR1bits.RCIF == 0);

        // memorizza il carattere che e' appena arrivato
        c = RCREG;

        // settaggio distanza massima tollerata
        switch (c) {
            case '1': distance = 10;
                break;
            case '2': distance = 20;
                break;
            case '3': distance = 30;
                break;
            case '4': distance = 40;
                break;
        }

        printf("selected %d\n\r", distance);
        printf("\n\r");

        printf("SELEZIONARE UNITA' DI MISURA\n\r");
        printf("\n\r");
        printf("Press 1 to Km/h\n\r");
        printf("Press 2 to m/s\n\r");

        // aspetta un input da tastiera
        while (PIR1bits.RCIF == 0);

        // memorizza il carattere che e' appena arrivato
        c = RCREG;

        // settaggio unita' di misura
        switch (c) {
            case '1': mode = 1;
                break;
            case '2': mode = 0;
                break;
        }

        if (mode)
            printf("selected m/s\n\r");
        else
            printf("selected cm/ms\n\r");

        printf("\n\r");

        printf("SELEZIONARE LA DISTANZA FRA I SENSORI COMPRESA TRA 0 E 999(enter = default)\n\r");
        printf("\n\r");

        // inizializzo le variabili di appoggio
        distance2 = x = count = 0;
        num[0] = 48;
        num[1] = 48;
        num[2] = 48;

        while (x != 13 && count < 3) {
            while (PIR1bits.RCIF == 0);

            x = RCREG;

            if (x == 13)
                break;
            else {
                printf("%d", x - 48);
                num[count] = x;
                count++;
            }
        }
        switch (count) {
            case 0:
                distance2 = 20;
                break;
            case 1:
                distance2 = num[0] - 48;
                break;
            case 2:
                distance2 = (num[0] - 48) * 10 + (num[1] - 48);
                break;
            case 3:
                distance2 = (num[0] - 48) * 100 + (num[1] - 48) * 10 + (num[2] - 48);
                break;
        }

        printf("(TEST): valore scelto %d\n", distance2);

        printf("\n\r");


        printf("Do you are ready for start Autovelox?\n\r");
        printf("press 1 to start or 0 to restart setup\n\r");

        while (PIR1bits.RCIF == 0);

        c = RCREG;

        switch (c) {
            case '0': restart = 1;
                break;
            case '1': restart = 0;
                break;
        }
    } while (restart);


    printf("\n\r");
    printf("\n\r");
    printf("Start\n\r");
}

void update() {
    int i = 0;
    //int var = 0;
    do {
        //int average = 0;

        for (i = 0; i < 3; i++) {

            ADCON0bits.GO = 1; // start conversion

            while (ADCON0bits.GO == 1); // wait for end of conversion

            adc[i] = (ADRESH << 8) + ADRESL;
            
        }
      //  average = ((adc[0] + adc[1] + adc[2]) / 3);
        //var = abs(adc[0] - average) + abs(adc[1] - average) + abs(adc[2] - average);

       
    } while (adc[0] < 120 || adc[1] < 120 || adc[2] < 120); //|| var > 3);
    //printf("%d\n\r",var);
    //printf("%u\n\r", adc[0]);
     //__delay_ms(500);
    volt = ((adc[0] + adc[1] + adc[2]) / 3); //* FACTOR;

    distanceEff = (6787 / (volt - 3)) - 4;
    //printf("%u\n\r", distanceEff);
    
}

main() {
    //frequenza settata a 4Mhz
    OSCCONbits.IRCF = 0b111;
    OSCTUNEbits.TUN = 0b11100;

    // registri per settare le porte da analogico a digitale, inizialmente le setto tutte a digitale
    // e successivamente mi setto le porte analogiche che mi servono
    ANSEL = 0;
    ANSELH = 0;

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

    // avvia la configurazione utente
    settaggi();

    /* configure ADC */
    ANSELbits.ANS2 = 1; // setto le porte
    ANSELHbits.ANS10 = 1;

    ADCON0bits.ADFM = 1; // right justified
    ADCON0bits.VCFG = 0; // Reference = VDD
    ADCON0bits.CHS = 0b0010; // Select channel 
    ADCON0bits.ADON = 1; // enable adc module
    ADCON1bits.ADCS = 0b010; // ADC clock =FOSC/2

    // setto il timer
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PS = 0b010;
    OPTION_REGbits.PSA = 0;

    // abilito l'interrupt generale
    INTCONbits.GIE = 1;

    ms = 0;

    for (;;) {
        float result;
        int res1;
        int res2;
        // ricezione dal primo sensore
        do update(); while (distanceEff > distance);

        PORTCbits.RC0 = 1;
        
        // cambia il sensore di ricezione
        ADCON0bits.CHS = 0b1010;

        // avvio il timer
        INTCONbits.T0IE = 1;

        // ricezione dal secondo sensore
        do update(); while (distanceEff > distance);

        // disabilito il timer
        INTCONbits.T0IE = 0;

        // riabilita il primo canale (a scapito del secondo)
        ADCON0bits.CHS = 0b0010;

        PORTCbits.RC0 = 0;

        // output del risultato
        printf("%u\n\r", distanceEff);
        printf("%us %ums\n\r", (int) (ms / 1000), ms - (int) (ms / 1000 * 1000));
        result = (float)distance2/(float)ms;
        res1 = (int)result;
        res2 = (result - res1)*100;
        printf("%d,%d cm/ms",res1, res2);


        // resetto il tempo
        ms = 0;

        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");

        __delay_ms(500);
    }
}
