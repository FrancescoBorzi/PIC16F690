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
//#define FACTOR 0.48

unsigned int adc[3]; // contengono il valore della misura della distanza
unsigned int distance; // massima distanza tollerata
unsigned int distance2; // distanza fra i sensori
unsigned int distanceEff; // distanza effettiva misurata
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
    ms = 0;

    do {
        // variabili di appoggio
        char c, num[3];
        int x, count;

        printf("-----------Autovelox v1.0--------------\n\r");
        printf("               Setup                    \n\n\r");
        //printf("\n\r");
        printf("SELEZIONARE LA DISTANZA DI RIVELAMENTO\n\n\r");
        //printf("\n\r");
        printf("Premi 1 per 10cm\n\r");
        printf("Premi 2 per 20cm\n\r");
        printf("Premi 3 per 30cm\n\r");
        printf("Premi 4 per 40cm\n\r");
        printf("Premi 5 per per utilizzare la rotellina\n\n\r");


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
            case '5': mode = 1;
        }

        //printf("selected %d\n\r", distance);
        //printf("\n\r");
        printf("SELEZIONARE LA DISTANZA FRA I SENSORI COMPRESA TRA 0 E 999 cm(enter = default)\n\n\r");
        //printf("\n\r");

        // inizializzo le variabili di appoggio
        x = count = 0;
        num[0] = num[1] = num[2] = 48;

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
        }

        printf("\n\rAvviare l'autovelox?\n\rpremi 1 per avviare 0 per ripetere il setup\n\n\r");
        //printf("premi 1 per avviare 0 per ripetere il setup\n\n\r");

        while (PIR1bits.RCIF == 0);

        c = RCREG;

        if(c == '0')
            restart = 1;
        else
            restart = 0;


    } while (restart);

    printf("Start\n\n\r");
}

void update() {

    int i = 0; // indice per il ciclo for

    int var = 0; // varianza

    int average = 0; // media
    do {

        for (i = 0; i < 3; i++) {

            ADCON0bits.GO = 1; // start conversion

            while (ADCON0bits.GO == 1); // wait for end of conversion

            adc[i] = (ADRESH << 8) + ADRESL; // assegnamento del voltaggio

        }

        average = ((adc[0] + adc[1] + adc[2]) / 3); // media dei valori

        var = abs(adc[0] - average) + abs(adc[1] - average) + abs(adc[2] - average); //calcolo della varianza

    } while (var > 5);

    // volt = ((adc[0] + adc[1] + adc[2]) / 3); //* FACTOR;

    distanceEff = (6787 / (average - 3)) - 4;  //calcolo della distanza effettiva


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
    ANSELbits.ANS0 = 1;

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

    for (;;) {

        int adc1;
        
        float result; // velocità

        int res[2]; // velocità (parte intera)

       // int res2; // velocità (parte frazionaria)

        if(mode) {
            //setto il canale per la rotellina
             ADCON0bits.CHS = 0b0000;

             __delay_ms(500);

             ADCON0bits.GO = 1; // start conversion
             while (ADCON0bits.GO == 1); // wait for end of conversion

             adc1 = (ADRESH << 8) + ADRESL;

             ADCON0bits.CHS = 0b0010;

             if(adc1 > 720) adc1 = 720;

             adc1 = (((float)adc1/(float)720)*100);

             distance = ((float)adc1 / (float)100) * 30 + 10;

             //printf("%d distance\n\r", distance);
        }

        // ricezione dal primo sensore
        do update(); while (distanceEff > distance);

        // avvio il led
        //PORTCbits.RC0 = 1;

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

        // disattivo il led
        //PORTCbits.RC0 = 0;

        // output del risultato
        printf("%ucm distanza effettiva \n\r%ucm distanza considerata\n\r", distanceEff, distance);
        printf("%us %ums tempo impiegato\n\r", (int) (ms / 1000), ms - (int) (ms / 1000 * 1000));
        printf("%ucm distanza percorsa\n\r", distance2);
        result = (float) distance2 / (float) ms;
        result *= 10;
        res[0] = (int) result;
        res[1] = (result - res[0])*100;
        printf("%d,%d m/s\n\r", res[0], res[1]);
        //
        result *= 3.6;
        res[0] = (int) result;
        res[1] = (result - res[0])*100;
        printf("%d,%d km/h\n\n\r", res[0], res[1]);

        // resetto il tempo
        ms = 0;

        __delay_ms(500);
    }
}
