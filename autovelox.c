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


#define _XTAL_FREQ   8000000
#define T_DELAY 256 - 249

int adc, adc1, adc2; // contengono il valore della misura della distanza
int distance; // massima distanza tollerata
int distance2; // distanza fra i sensori
int ms; // tempo misurato in millisecondi
int mode; // unita' di misura della velocita'

// ridefinisco la printf
void putch(unsigned char byte)
{
    /* output one byte */
    TXREG = byte;
    while (!TXSTAbits.TRMT); /* set when register is empty */
}

void interrupt isr(void)
{
    // scatta ogni millisecondo
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
        printf("Press 1 to 20cm\n\r");
        printf("Press 2 to 40cm\n\r");
        printf("Press 3 to 60cm\n\r");
        printf("Press 4 to 80cm\n\r");

	// aspetta un input da tastiera
        while (PIR1bits.RCIF == 0);
        
        // memorizza il carattere che e' appena arrivato
        c = RCREG;
        
        // settaggio distanza massima tollerata
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
        
        // aspetta un input da tastiera
        while (PIR1bits.RCIF == 0);
        
        // memorizza il carattere che e' appena arrivato
        c = RCREG;
        
        // settaggio unita' di misura
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
        
        // TODO: distance2 deve contenere il numero composto da tutti i caratteri cifra ricevute in input
        printf("SELEZIONARE LA DISTANZA FRA I SENSORI COMPRESA TRA 0 E 999(enter = default)\n\r");
        printf("\n\r");

        // inizializzo le variabili di appoggio
        distance2 = count = 0;
        num[0] = 0;
        num[1] = 0;
        num[2] = 0;
        
        while(x != 13 && count < 3)
        {
            while (PIR1bits.RCIF == 0);

            x = RCREG;
            
            if (x == 13 )
                break;
            else
            {
                num[count] = x;
                count++;
            }
        }

        distance2 = (num[0] - 48) * 100 + (num[0] - 48) * 10 + (num[0] - 48);

        printf("(TEST): valore scelto %d", distance);

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
    // cronologia delle misure
    adc2 = adc1;
    adc1 = adc;
    
    ADCON0bits.GO = 1; // start conversion
    
    while (ADCON0bits.GO == 1); // wait for end of conversion
    
    adc = (ADRESH );
    printf("%d\n\r",adc);
    printf("%d\n\r",ADRESL);
    __delay_ms(100);
}

main()
{
    //frequenza settata a 4Mhz
    OSCCONbits.IRCF = 0b111;
    OSCTUNEbits.TUN = 0b11010;
    
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
    ADCON1bits.ADCS = 0b010; // ADC clock =FOSC/32

    // setto il timer
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PS = 0b010;
    OPTION_REGbits.PSA = 0;
    
    // abilito l'interrupt generale
    INTCONbits.GIE = 1;

	ms = 0;

    for (;;)
    {
    	// ricezione dal primo sensore
        do	update();
        while (adc < 300 || adc1 < 300 || adc2 < 300);
        
        // cambia il sensore di ricezione
        ADCON0bits.CHS = 0b1010;
        
        // avvio il timer
        INTCONbits.T0IE = 1;
        
        // azzero la cronologia
        adc = adc1 = adc2 = 0;
		
	// ricezione dal secondo sensore
        do	update();
        while (adc < 300 || adc1 < 300 || adc2 < 300);
        
        // disabilito il timer
        INTCONbits.T0IE = 0;
        
        // riabilita il primo canale (a scapito del secondo)
        ADCON0bits.CHS = 0b0010;
        
        // azzero la cronologia
        adc = adc1 = adc2 = 0;

        // output del risultato
        printf("%ds %dms\n\r", (int) (ms / 1000), ms);

	// resetto il tempo
        ms = 0;
        
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        printf("\n\r");
        
        __delay_ms(500);
    }
}
