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

int adc, distance, ms, mode, T_DELAY, adc1, adc2, distance2 = 0;

// ridefinisco la printf

void putch(unsigned char byte)
{
    /* output one byte */
    TXREG = byte;
    while (!TXSTAbits.TRMT); /* set when register is empty */
}

void interrupt isr(void)
{
    if (INTCONbits.T0IF == 1)
    {
        ms++;
        if (ms == 1000)
        {
            ms = 0;
            PORTCbits.RC0 = !PORTCbits.RC0;
        }
        TMR0 = T_DELAY;
        INTCONbits.T0IF = 0;
    }
}

void settaggi()
{
    int restart = 0;
    
    do
    {
        char c;
        int x;
        
        printf("-----------Autovelox v1.0--------------\n\r");
        printf("               Setup                    \n\r");
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        printf("SELEZIONARE LA DISTANZA DI RIVELAMENTO\n\r");
        printf("\n\r");
        printf("Press 1 to 20cm\n\r");
        printf("Press 2 to 40cm\n\r");
        printf("Press 3 to 60cm\n\r");
        printf("Press 4 to 80cm\n\r");

        while (PIR1bits.RCIF == 0);
        
        c = RCREG;
        
        switch (c)
        {
            case '1': distance = 20;
                break;
            case '2': distance = 40;
                break;
            case '3': distance = 60;
                break;
            case '4': distance = 80;
                break;
        }
        
        printf("selected %d\n\r", distance);
        printf("\n\r");
        printf("SELEZIONARE UNITA' DI MISURA\n\r");
        printf("\n\r");
        printf("Press 1 to Km/h\n\r");
        printf("Press 2 to m/s\n\r");
        
        while (PIR1bits.RCIF == 0);
        
        c = RCREG;
        
        switch (c)
        {
            case '1': mode = 1;
                break;
            case '2': mode = 0;
                break;
        }
        
        if(mode)
            printf("selected Km/h\n\r");
        else
            printf("selected m/s\n\r");

        printf("\n\r");
        printf("SELEZIONARE LA DISTANZA FRA I SENSORI(press enter to default)\n\r");
        printf("\n\r");
        
        distance2 = 0;
        
        while(x != 13)
        {
            while (PIR1bits.RCIF == 0);
            
            x = RCREG;
            
            if (x == 13 )
                break;
            else
            {
                distance2 = x - 48;
                printf("%d", distance2);
            }
        }
        printf("\n\r");
        printf("Do you are ready for start Autovelox?\n\r");
        printf("press 1 to start or 0 to restart setup\n\r");
        
        while (PIR1bits.RCIF == 0);
        
        c = RCREG;
        
        switch (c)
        {
            case '0': restart = 1;
                break;
            case '1': restart = 0;
                break;
        }
    }
    while (restart);


    printf("\n\r");
    printf("\n\r");
    printf("Start\n\r");
}

void update()
{
    adc2 = adc1;
    adc1 = adc;
    
    ADCON0bits.GO = 1; // start conversion
    
    while (ADCON0bits.GO == 1); // wait for end of conversion
    
    adc = (ADRESH << 8) + ADRESL;
}

int main(void)
{

    us = ms = s = test = 0;
    
    // frequenza settata a 4Mhz
    OSCCONbits.IRCF = 0b111;
    
    // registri per settare le porte da analogico a digitale, inizialmente le setto tutte a digitale
    // e successivamente mi setto le porte analogiche che mi servono
    ANSEL = 0; 
    ANSELH = 0;
    
    T_DELAY = 256 - 250;
    
    ms = 0;
    //frequenza settata a 4Mhz
    OSCCONbits.IRCF = 0b111;
    OSCTUNEbits.TUN = 0b11010;
    
    //registri per settare le porte da analogico a digitale, inizialmente le setto tutte a digitale
    ANSEL = 0; //e successivamente mi setto le porte analogiche che mi servono
    ANSELH = 0;
    T_DELAY = 6 + 1;

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
    settaggi();
    
    /* configure ADC */
    ANSELbits.ANS2 = 1; //setto le porte
    ANSELHbits.ANS10 = 1;

    ADCON0bits.ADFM = 1; // right justified
    ADCON0bits.VCFG = 0; // Reference = VDD
    ADCON0bits.CHS = 0b0010; // Select channel 
    ADCON0bits.ADON = 1; //enable adc module
    ADCON1bits.ADCS = 0b010; // ADC clock =FOSC/32
    // INTCONbits.PEIE = 1; //enable peripheral interrupt ?
    //abilito l'interrupt del converitore
    //    PIR1bits.ADIF = 0;
    //    PIE1bits.ADIE = 1;


    //setto il timer
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PS = 0b010;
    OPTION_REGbits.PSA = 0;
    
//faccio partire il timer
    INTCONbits.T0IF = 0;
    
   // INTCONbits.T0IE = 1; // enable interrupt on timer 0
//abilito l'interrupt generale
    //faccio partire il timer
    INTCONbits.T0IF = 0;
    // INTCONbits.T0IE = 1; // enable interrupt on timer 0
    //abilito l'interrupt generale
    INTCONbits.GIE = 1;

    //OSCTUNE
    for (;;)
    {
        do	update();
        while (adc < 300 || adc1 < 300 || adc2 < 300);
        
        ADCON0bits.CHS = 0b1010;
        INTCONbits.T0IE = 1;
        //printf("start\n\r");
        adc = adc1 = adc2 = 0;

        do	update();
        while (adc < 300 || adc1 < 300 || adc2 < 300);
        
        INTCONbits.T0IE = 0;
        ADCON0bits.CHS = 0b0010;
        adc = adc1 = adc2 = 0;

        printf("%ds %dms\n\r", (int) (ms / 1000), ms);

        ms = 0;
        
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        
        __delay_ms(500);
    }
    return 0;
}
