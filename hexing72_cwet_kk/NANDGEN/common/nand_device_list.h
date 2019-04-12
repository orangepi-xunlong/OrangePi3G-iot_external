
#ifndef __NAND_DEVICE_LIST_H__
#define __NAND_DEVICE_LIST_H__

#define NAND_MAX_ID		7
#define CHIP_CNT		7
#define RAMDOM_READ		(1<<0)
#define CACHE_READ		(1<<1)

typedef struct
{
   u8 id[NAND_MAX_ID];
   u8 id_length;
   u8 addr_cycle;
   u8 iowidth;
   u16 totalsize;
   u16 blocksize;
   u16 pagesize;
   u16 sparesize;
   u32 timmingsetting;
   u8 devciename[30];
   u32 advancedmode;
}flashdev_info,*pflashdev_info;

static const flashdev_info gen_FlashTable[]={
	{{0x98,0xBC,0x90,0x66,0x76,0x16,0x08}, 7,5, 16,512,256,4096,256,0x10814114,"KSLCGBL2GA2H2A",0}, 
	{{0x2C,0xBC,0x90,0x66,0x54,0x00,0x00}, 5,5, 16,512,256,4096,224,0x10805113,"MT29C4G48MAAHBAAKS_5WT",0}, 
	{{0xEC,0xBC,0x00,0x6A,0x56,0x00,0x00}, 5,5, 16,512,256,4096,128,0x10C08123,"K524G2GACH_B050 ",1}, 
	{{0xAD,0xBC,0x90,0x11,0x00,0x00,0x00}, 5,5, 16,512,256,4096,128,0x21015133,"H9DA4VH2GJMMCR_4EM",0}, 
	{{0xAD,0xBC,0x80,0x16,0x20,0x00,0x00}, 5,5, 16,512,256,4096,128,0x21014133,"H9DA4VH2GJAMCR_4EM",0}, 
	{{0x98,0xBC,0x90,0x66,0x00,0x00,0x00}, 4,5, 16,512,256,4096,128,0x10814114,"TYBC0A111557KC",0}, 
	{{0xEC,0xBC,0x00,0x66,0x00,0x00,0x00}, 4,5, 16,512,256,4096,128,0x10C04133,"K524G2GACJ_B050",3}, 
};

#endif
