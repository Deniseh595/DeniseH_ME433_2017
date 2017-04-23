/* 
 * File:   maini2c.c
 * Author: Denise
 *
 * Created on April 19, 2017, 10:35 AM
 */

#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include <stdlib.h>
#include "i2c_master_noint.h"

#pragma config DEBUG =  OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
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
#pragma config WDTPS = PS1048576 // use slowest wdt
#pragma config WINDIS = OFF // wdt no window mode
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz (/2)
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV (*24 = 96))
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz (96/2)
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN =  ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module


// Demonstrate I2C by having the I2C2 on PIC32 talk to MCP23008
// Master will use SDA2 (B2) and SCL2 (B3).  
//  SDA2 (PIC)-> SDA
//  SCL2 (PIC)-> SCL
// Two bytes will be written to the slave and then read back to the slave.

#define SLAVE_ADDR 0x20

void initExpander(void);
void setExpander(unsigned char pin, unsigned char level);
unsigned char getExpander(void);

void initExpander(void){
    //setting the iocon
    
    /*i2c_master_start();         //send start bit
    i2c_master_send(SLAVE_ADDR <<1);
    i2c_master_send(0x05);      //device opcode
    i2c_master_send(0b00100000);
    i2c_master_stop(); */
    
    //set the in and out pins
    i2c_master_start();                  //send start bit
    i2c_master_send(SLAVE_ADDR <<1 | 0); //device opcode 
    i2c_master_send(0x00);               //io dir address
    i2c_master_send(0xF0);               //0-3 out, 4-7 in
    i2c_master_stop(); 
 
   // i2c_master_start();          //enable internal pullup resistors
   // i2c_master_send(SLAVE_ADDR << 1| 0);
   // i2c_master_send(0x06);
   // i2c_master_send(0xF0);
   // i2c_master_stop();

    //clear all of the pins 
   // i2c_master_start(); 
   // i2c_master_send(SLAVE_ADDR <<1 | 0);
   // i2c_master_send(0x09);
   // i2c_master_send(0x00);
   // i2c_master_stop();
 
}

void setExpander (unsigned char pin, unsigned char level) {
    unsigned char value; 
    value = level << pin;
    i2c_master_start();                   //send start bit
    i2c_master_send(SLAVE_ADDR <<1 | 0);  //device opcode
    i2c_master_send(0x09);                //output latch address
    i2c_master_send(value);
    i2c_master_stop();
}


unsigned char getExpander(void){
    //LATAbits.LATA4 = 0;
    unsigned char pinread = 0;
    i2c_master_start();                     //send start bit
    i2c_master_send(SLAVE_ADDR <<1 | 0);    //device opcode
    i2c_master_send(0x09);                  //io dir address (pins to read)
    i2c_master_restart();                   //restart so we can read
    i2c_master_send((SLAVE_ADDR <<1) | 1);  //want to read
    pinread = i2c_master_recv();
    i2c_master_ack(1);                      //send a nack
    i2c_master_stop();
    return pinread;
}

int main() {
   //general inits 
    __builtin_disable_interrupts();

   //set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
   __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    
    ANSELBbits.ANSB2 = 0; 
    ANSELBbits.ANSB3 = 0;       //turn off analog in for i2c pins
    i2c_master_setup();
    TRISBbits.TRISB4 = 1;       //set B4 as in
    TRISAbits.TRISA4 = 0;       //set A4 as out
    LATAbits.LATA4 = 1; 
 
    __builtin_enable_interrupts();
    
    initExpander();              // init MCP223008 chip

    
    while(1) {           
      if ((getExpander() & 0b10000000) == 0b00000000) { 
          setExpander(0,0);         // turn LED on if button isn't pressed
      }
      else { 
          setExpander(0,1);        //LED off if button pressed
      }
  }
}