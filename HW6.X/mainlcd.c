/* 
 * File:   mainlcd.c
 * Author: Denise
 *
 * Created on April 22, 2017, 10:05 PM
 */

#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include "LCD_help.h"


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
#define COLORII WHITE

//fn prototypes
void ascii_write(char c, char x, char y, unsigned short colori);
void write_string(char *msg, char x, char y, unsigned short colori);
void draw_bar(char x, char y, unsigned short color, int  length); 



int main() {    
    
    char msg[100], x, y;
    
    x = 33; y = 32; //start writing message
   
    
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
   
    
    __builtin_enable_interrupts();
    
    SPI1_init();
    LCD_init();
    LCD_clearScreen(COLORII);
    
    sprintf(msg, "sup dude");
    write_string(msg, x, y, BLACK);   //write in black at 42,32
   
    while (1) {
        int num; 
        x = 82;                     //number comes after the message
        
        for (num= 0; num < 101; num++){
            _CP0_SET_COUNT(0);
            char xbar, ybar;
            
            xbar = 15, ybar = 50; 
            
            sprintf(msg, "%d", num);
            write_string(msg, x, y, BLACK);
            
            draw_bar(xbar, ybar, CYAN, num); //xstart, ystart, color, progress, width, max
            
             
            while(_CP0_GET_COUNT() < 48000000/2/5) {;} //delay for 5 Hz
        }
        
        int get_time; 
        get_time = _CP0_GET_COUNT();
        while(_CP0_GET_COUNT()< get_time+ 48000000/2){
        sprintf(msg, "%d %d %d ",0,0,0);             
        write_string(msg, x, y, COLORII); //clear words
        int ii; 
            for (ii = 0; ii<101; ii++){
            draw_bar(15,50, COLORII, ii); //clear bar
            }
        }
        
    }           
}
        
        
        
        
  

void ascii_write(char c, char x, char y, unsigned short colori){
    char row;  
    row = c - 0x20;            //loc in ascii array 
    int i, j;
    for (i = 0; i<5; i++) { 
        if ((x+i) < (128-5)){
            for (j=0; j<8; j++){
                if ((y+j)<(128-8)){
                    if ((ASCII[row][i] >> j & 1) == 1){
                        LCD_drawPixel(x+i, y+j, colori);
                    } 
                    else { 
                        LCD_drawPixel(x+i, y+j, COLORII);
                    }                    
                }
            }
        }        
    }    
}

void write_string(char *msg, char x, char y, unsigned short colori){
    int ii = 0; 
    char c;
    while (msg[ii] !=0) { 
        c = msg[ii];
        ascii_write(c, x, y, colori);
        x = x+5;
        ii++;  
    }    
 }

void draw_bar(char x, char y, unsigned short color, int  length){
    //draw the bar from 0 to +50 length and - 50 length to 0  
    int i; 
    for (i = 0; i <5; i++){
        LCD_drawPixel(x+length, y+i, color );
    }
}
