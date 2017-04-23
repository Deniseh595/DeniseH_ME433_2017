#include "i2c_slave_setup.h"
 
//sets up the MCP23008 chip 
#define SLAVE_ADDR 0b01001100

void init_expander(void){
 //setting the iocon
 /*i2c_master_start();         //send start bit
 i2c_master_send(SLAVE_ADDR <<1 | 0);
 i2c_master_send(0x05);      //device opcode
 i2c_master_send(0b00001100); //seq, address pins enabled, int disabled
 i2c_master_stop();  
    */
    //set the in and out pins
 i2c_master_start();         //send start bit
 i2c_master_send(SLAVE_ADDR <<1 | 0); //device opcode 
 i2c_master_send(0x00);      //io dir address
 i2c_master_send(0xF0);      //0-3 out, 4-7 in
 i2c_master_stop(); 
 
i2c_master_start();         //enable internal pullup resistors
i2c_master_send(SLAVE_ADDR << 1 | 0);
i2c_master_send(0x06);
i2c_master_send(0b11110000);
i2c_master_stop();

//clear all of the pins 
i2c_master_start(); 
i2c_master_send(SLAVE_ADDR <<1 | 0);
i2c_master_send(0x09);
i2c_master_send(0x00);
i2c_master_stop();
 
}

void setExpander (unsigned char pin, unsigned char level) {
i2c_master_start();         //send start bit
i2c_master_send(SLAVE_ADDR <<1 | 0 ); //device opcode
i2c_master_send(0x09);      //io dir address
i2c_master_send(level << pin);
i2c_master_stop();
}


unsigned char getExpander(void){
    unsigned char pinread;
    i2c_master_start();                  //send start bit
    i2c_master_send(SLAVE_ADDR <<1 | 0); //device opcode
    i2c_master_send(0x09);               //io dir address (pins to read)
    i2c_master_restart();                //restart so we can read
    i2c_master_send(SLAVE_ADDR <<1 | 0x01);  //want to read
    pinread = i2c_master_recv();
    i2c_master_ack(1);                      //send a nack
    i2c_master_stop();
    return pinread;
}
