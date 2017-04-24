/* 
 * File:   i2c_slave_setup.h
 * Author: Denise
 *
 * Created on April 19, 2017, 1:56 PM
 */

#ifndef I2C_SLAVE_SETUP_H
#define	I2C_SLAVE_SETUP_H

void init_expander(void);
void setExpander (unsigned char pin, unsigned char level);
unsigned char getExpander(void);


#endif	/* I2C_SLAVE_SETUP_H */

