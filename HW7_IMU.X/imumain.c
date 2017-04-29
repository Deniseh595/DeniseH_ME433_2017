/* 
 * File:   imumain.c
 * Author: Denise
 *
 * Created on April 24, 2017, 11:36 PM
 */

#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include <stdlib.h>
#include "i2c_master_noint.h"
#include "LCD_help.h"

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


#define SLAVE_ADDR 0b01101010

void imu_init(void) {
    
    //ctrl_1xl
    i2c_master_start();                     //send start bit
    i2c_master_send(SLAVE_ADDR <<1 | 0);    //device opcode
    i2c_master_send(0x10);                  //io dir address (pins to read)
    i2c_master_send(0b10000010);            //1.66kHz, 100Hz mode   
    i2c_master_stop();
    
    //ctrl2_g
    i2c_master_start();                     //send start bit
    i2c_master_send(SLAVE_ADDR <<1 | 0);    //device opcode
    i2c_master_send(0x11);                  //io dir address (pins to read)
    i2c_master_send(0b10001000);            //enable automatic address update    
    i2c_master_stop();
    
    //ctrl3_c
    i2c_master_start();                     //send start bit
    i2c_master_send(SLAVE_ADDR <<1 | 0);    //device opcode
    i2c_master_send(0x12);                  //io dir address (pins to read)
    i2c_master_send(0b00000100);            //enable automatic address update    
    i2c_master_stop();
}

void i2c_seqread(unsigned char slave, unsigned char reg, unsigned char * data, int len) {    
    int i = 0;
    
    i2c_master_start(); 
    i2c_master_send(slave <<1 | 0); 
    i2c_master_send(reg);
    i2c_master_restart();
    i2c_master_send((slave <<1 )| 1);
    
    for (i = 0; i<(len-1); i++) {
        data [i] = i2c_master_recv();
        i2c_master_ack(0);
    }
    
    data[len-1] = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();   
}

void make_short(unsigned char * data, int len, signed short * imushorts ){
    char i =0, j = 0; 
    while (i<len){
        imushorts[j] = data[i+1] <<8 | data [i];
        i = i+2; 
        j++ ;
    }
}

/*void drawbar_x{
    
}

void drawbar_y{
    
}*/

int main (void){
    
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
    
    imu_init();
    SPI1_init();
    LCD_init();
    LCD_clearScreen(WHITE);
    
    unsigned char imudata[14], msg[100];
    signed short imushorts[7], temp, gyrox, gyroy, gyroz, accelx, accely; 
    
    
    while (1){
        i2c_seqread(SLAVE_ADDR, 0x20, imudata, 14);
        make_short(imudata, 14, imushorts );
        
        
        temp = imushorts[0];
        gyrox = imushorts[1];
        gyroy = imushorts[2];
        gyroz = imushorts[3];
        accelx = imushorts[4];
        accely = imushorts[5];
        
        
        //write numbers 
        sprintf(msg, "xbar: %d", accelx); // print x acceleration data
        write_string(msg, 2, 20, BLACK);
        sprintf(msg, "ybar: %d", accely); // print y acceleration data
        write_string(msg, 2, 30, BLACK);
        sprintf(msg, "xdir: %d", gyrox); // print  acceleration data
        write_string(msg, 2, 40, BLACK);
        sprintf(msg, "ydir: %d", gyroy); // print y acceleration data
        write_string(msg, 2, 50, BLACK);
        sprintf(msg, "zdir: %d", gyroz); // print y acceleration data
        write_string(msg, 2, 60, BLACK);
        sprintf(msg, "deg: %d", temp); // print y acceleration data
        write_string(msg, 2, 70, BLACK);
        
        
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 24000000 / 10) {;
        }; // 5 Hz update 
    }
       
    
}