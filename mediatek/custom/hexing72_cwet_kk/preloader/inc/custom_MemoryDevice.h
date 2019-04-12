#ifndef __CUSTOM_MEMORYDEVICE__
#define __CUSTOM_MEMORYDEVICE__

/*
 ****************************************************************************
     Specify memory part number and clock rate
     Example :
     #define CS_PART_NUMBER[0]       KMN5U000ZM_B203
     #define EMI_CLK[0]              266M
     #define CS_PART_NUMBER[1]       H9TA4GH2GDMCPR_4GM   
     #define EMI_CLK[1]              266M
// //add 05_19  
//#define CS_PART_NUMBER[0]	KMN5X000ZM_B209  
 ****************************************************************************
*/

#define BOARD_ID                MT6572_EVB1

#define CS_PART_NUMBER[0]       KMN5X000ZM_B209

#define CS_PART_NUMBER[1]	KE4CN2L2HA5A2A

#define EMI_CLK[0]              266M
#define EMI_CLK[1]              266M
      
#endif /* __CUSTOM_MEMORYDEVICE__ */
