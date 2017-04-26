
#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

// DEVCFG0
#pragma config DEBUG =  10 // no debugging
#pragma config JTAGEN = 0 // no jtag
#pragma config ICESEL = 11 // use PGED1 and PGEC1
#pragma config PWP = 111111111  // no write protect
#pragma config BWP =  1 // no boot write protect
#pragma config CP = 1 // no code protect

// DEVCFG1
#pragma config FNOSC = 011 // use primary oscillator with pll
#pragma config FSOSCEN = 0 // turn off secondary oscillator
#pragma config IESO = 0 // no switching clocks
#pragma config POSCMOD = 10 // high speed crystal mode
#pragma config OSCIOFNC = 1 // disable secondary osc
#pragma config FPBDIV = 00 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = 10 // do not enable clock switch
#pragma config WDTPS =  10100 // use slowest wdt
#pragma config WINDIS = 1 // wdt no window mode
#pragma config FWDTEN = 0 // wdt disabled
#pragma config FWDTWINSZ = 11 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = 001 // divide input clock to be in range 4-5MHz (/2)
#pragma config FPLLMUL = 111 // multiply clock after FPLLIDIV (*24 = 96))
#pragma config FPLLODIV = 001 // divide clock after FPLLMUL to get 48MHz (96/2)
#pragma config UPLLIDIV = 111 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN =  0 // USB clock on

// DEVCFG3
#pragma config USERID = 0000000000000000 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = 0 // allow multiple reconfigurations
#pragma config IOL1WAY = 0 // allow multiple reconfigurations
#pragma config FUSBIDIO = 1 // USB pins controlled by USB module
#pragma config FVBUSONIO = 1 // USB BUSON controlled by USB module


int main() {

    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISBbits.TRISB4 = 1;       //set B4 as in
    TRISAbits.TRISA4 = 0;       //set A4 as out
    TRISBbits.TRISB2 = 0; 
    LATBbits.LATB2 = 1; 
    //LATAbits.LATA4 = 0;         //set high
    int timenow;
    
    __builtin_enable_interrupts();
    
    
    
    while(1) {
        while(!PORTBbits.RB4){;
        }
		LATAbits.LATA4 = 0; //turn LED on
        _CP0_SET_COUNT(0);// use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        while (_CP0_GET_COUNT()< 12000){;}
        LATAbits.LATA4 = 1; //turn LED off 
        timenow = _CP0_GET_COUNT();
        while (_CP0_GET_COUNT()< 12000+timenow){;}
        LATAbits.LATA4 = 0; //turn it back on
        // remember the core timer runs at half the sysclk
    }
}