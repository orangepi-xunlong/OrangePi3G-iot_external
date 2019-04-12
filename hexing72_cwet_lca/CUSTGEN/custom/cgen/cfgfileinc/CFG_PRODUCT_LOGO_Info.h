#ifndef _CFG_PRODUCT_LOGO_INFO_H
#define _CFG_PRODUCT_LOGO_INFO_H

typedef struct
{
	unsigned char BOOTLOGONUMBER[10];
	unsigned char reserved[4096-10];
	
}File_Product_Logo_Struct;

#define CFG_FILE_PRODUCT_LOGO_REC_SIZE    sizeof(File_Product_Logo_Struct)
#define CFG_FILE_PRODUCT_LOGO_REC_TOTAL   1

#endif