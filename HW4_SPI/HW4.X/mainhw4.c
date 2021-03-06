/* 
 * File:   mainhw4.c
 * Author: Denise
 *
 * Created on April 11, 2017
 */


#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <math.h>


// DEVCFG0
#pragma config DEBUG =  OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF  // no write protect
#pragma config BWP =  OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable secondary osc
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS =  PS1 // use slowest wdt
#pragma config WINDIS = OFF // wdt no window mode
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz (/2)
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV (*24 = 96))
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz (96/2)
#pragma config UPLLIDIV = DIV_12// divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN =  ON // USB clock on

// DEVCFG3
#pragma config USERID = 0000000000000000 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

#define CS LATBbits.LATB9    // chip select pin

void initSPI1() {
  // set up the chip select pin as an output
  // the chip select pin is used by the dac to indicate
  // when a command is beginning (clear CS to low) and when it
  // is ending (set CS high)
  RPB13Rbits.RPB13R = 0b0011; 	//SDO1 from PIC (B13)
  SDI1Rbits.SDI1R = 0b0100 ; 	//set RB8 to input pin
  TRISBbits.TRISB9 = 0;       //set A4 as out
  
  CS= 1;

  // Master - SPI1, pins are: SDI1(B8), SDO1(B13), SCK1(B14).  
  // we manually control SS as a digital output (A4)
  // since the pic is just starting, we know that spi is off. We rely on defaults here
 
  // setup spi1
  SPI1CON = 0;              // turn off the spi module and reset it
  SPI1BUF;                  // clear the rx buffer by reading from it
  SPI1BRG = 0x300;            // baud rate to 10 MHz [SPI1BRG = (80000000/(2*desired))-1]
  SPI1STATbits.SPIROV = 0;  // clear the overflow bit
  SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
  SPI1CONbits.MSTEN = 1;    // master operation
  SPI1CONbits.ON = 1;       // turn on spi 1

}

unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

void setVoltage(unsigned char channel, unsigned char voltage){
	//channel = channel <<7;
    //spi_io((voltage >> 4) & (channel | 0x7F));
    
    //spi_io((voltage << 4)); 	
    //CS = 1; 
    unsigned char b1 = 0, b2 = 0; 
    
    b1=(channel <<7);
    b1 = b1 | (0b01110000);
    b1 = b1 | (voltage >>4);
    
    b2 = voltage <<4; 
    
    CS = 0; 
    spi_io(b1);
    spi_io(b2);
    CS = 1; 
	
}


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
    TRISAbits.TRISA4 = 0; 
    initSPI1();
            
    __builtin_enable_interrupts();
    
  
    int i = 0; 
    unsigned int sinwave[100];
    unsigned int rampwave[100];
    double temp;
   
    for (i=0; i<100;i++){
        temp= 255.0/2.0+ 255.0/2.0*sin(3.1415*2.0*i/100.0);
        sinwave[i]= temp;
        rampwave [i]= i*255.0/100.0;
        }
        
    while (1) {  
         _CP0_SET_COUNT(0);
        for (i = 0; i<100; i++){
        setVoltage(0,sinwave[i]); 
        setVoltage(1,rampwave[i]);
        while ( _CP0_GET_COUNT() < 24000){
         ; }  //delay for right frequency 
        }
    }
  return 0;  
}

