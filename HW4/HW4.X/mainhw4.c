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
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz (/2)
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV (*24 = 96))
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz (96/2)
#pragma config UPLLIDIV = DIV_12// divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN =  0 // USB clock on

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
  SPI1BRG = 0x1000;            // baud rate to 10 MHz [SPI1BRG = (80000000/(2*desired))-1]
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
    b1 = b1 | (0b1110000);
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
    initSPI1();
            
    __builtin_enable_interrupts();
    
    
    _CP0_SET_COUNT(0);
    int i = 0; 
    unsigned int sinwave[100];
    unsigned int rampwave[100];
    double temp;
   
    
    while (1) {
      
        for (i=0; i<100;i++){
        temp= 255.0/2.0+ 255.0/2.0*sin(3.1415*2.0*i/100.0);
        sinwave[i]= temp;
        rampwave [i]= i*255.0/100.0;
        }
        
        int gettime;
        gettime = _CP0_GET_COUNT();
        setVoltage(0,sinwave[i]);
        setVoltage(1,rampwave[i]);
        while ( _CP0_GET_COUNT() < 24000+gettime){
         ;
        }  //delay for right frequency 
    }
  return 0;  
}

