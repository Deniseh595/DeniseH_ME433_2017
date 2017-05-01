/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

#include <xc.h>
#include "imuhelp.h"
/* TODO:  Include other files here if needed. */


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */
/** Descriptive Data Item Name

  @Summary
    Brief one-line summary of the data item.
    
  @Description
    Full description, explaining the purpose and usage of data item.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
  @Remarks
    Any additional remarks
 */



/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */

/** 
  @Function
    int ExampleLocalFunctionName ( int param1, int param2 ) 

  @Summary
    Brief one-line description of the function.

  @Description
    Full description, explaining the purpose and usage of the function.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

  @Precondition
    List and describe any required preconditions. If there are no preconditions,
    enter "None."

  @Parameters
    @param param1 Describe the first parameter to the function.
    
    @param param2 Describe the second parameter to the function.

  @Returns
    List (if feasible) and describe the return values of the function.
    <ul>
      <li>1   Indicates an error occurred
      <li>0   Indicates an error did not occur
    </ul>

  @Remarks
    Describe any special behavior not described above.
    <p>
    Any additional remarks.

  @Example
    @code
    if(ExampleFunctionName(1, 2) == 0)
    {
        return 3;
    }
 */



/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

// *****************************************************************************

/** 
  @Function
    int ExampleInterfaceFunctionName ( int param1, int param2 ) 

  @Summary
    Brief one-line description of the function.

  @Remarks
    Refer to the example_file.h interface header for function usage details.
 */
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

void make_short(unsigned char * data, int len, signed short * imushorts ){  //each measure takes 2 bytes
    char i =0, j = 0; 
    while (i<len){
        imushorts[j] = data[i+1] <<8 | data [i];
        i = i+2; 
        j++ ;
    }
}


/* *****************************************************************************
 End of File
 */
