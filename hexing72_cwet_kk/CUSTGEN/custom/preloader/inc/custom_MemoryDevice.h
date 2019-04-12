
#ifndef __CUSTOM_MEMORYDEVICE__
#define __CUSTOM_MEMORYDEVICE__

/*
 ****************************************************************************
 [README , VERY IMPORTANT NOTICE]
 --------------------------------
 After user configured this C header file, not only C compiler compile it but
 also auto-gen tool parse user's configure setting.
 Here are recommend configure convention to make both work fine.

 1. All configurations in this file form as #define MACRO_NAME MACRO_VALUE format.
    Note the #define must be the first non-space character of a line

 2. To disable the optional configurable item. Please use // before #define,
    for example: //#define MEMORY_DEVICE_TYPE

 3. Please don't use #if , #elif , #else , #endif conditional macro key word here.
    Such usage might cause compile result conflict with auto-gen tool parsing result.
    Auto-Gen tool will show error and stop.
    3.1.  any conditional keyword such as #if , #ifdef , #ifndef , #elif , #else detected.
          execpt this #ifndef __CUSTOM_MEMORYDEVICE__
    3.2.  any duplicated MACRO_NAME parsed. For example auto-gen tool got 
          2nd MEMORY_DEVICE_TYPE macro value.
 ****************************************************************************
*/

/*
 ****************************************************************************
 Step 1: Specify memory device type and its complete part number
         Possible memory device type: LPSDRAM (SDR, DDR)
 ****************************************************************************
*/

 
#define BOARD_ID                MT6572_EVB1

#define CS_PART_NUMBER[0]       KMN5U000ZM_B203	//04EMCP04_NL2DM627
#define EMI_CLK[0]                 266M

#define CS_PART_NUMBER[1]       KMNJS000ZM_B205	//GD9D4M4EYK
#define EMI_CLK[1]                 266M

#define CS_PART_NUMBER[2]				04EMCP04_NL2AS100
#define EMI_CLK[2]                 266M

#define CS_PART_NUMBER[3]				KE4CN2L2HA5A2A
#define EMI_CLK[3]                 266M

#define CS_PART_NUMBER[4]				KE4CN2L2SA5H2A
#define EMI_CLK[4]                 266M

#define CS_PART_NUMBER[5]				KMN5X000ZM_B209
#define EMI_CLK[5]                 266M

//#define CS_PART_NUMBER[6]				KMN5U000ZM_B203
//#define EMI_CLK[6]                 266M

//#define CS_PART_NUMBER[7]				KMNJS000ZM_B205
//#define EMI_CLK[7]                 266M

#endif /* __CUSTOM_MEMORYDEVICE__ */
