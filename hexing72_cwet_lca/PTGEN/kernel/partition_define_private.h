#include "partition_define.h"
static const struct excel_info PartInfo_Private[PART_NUM]={
			{"preloader",1048576,0x0, NAND},
			{"pro_info",1048576,0x100000, NAND},
			{"nvram",3145728,0x200000, NAND},
			{"protect_f",3145728,0x500000, NAND},
			{"protect_s",3145728,0x800000, NAND},
			{"seccfg",262144,0xb00000, NAND},
			{"uboot",524288,0xb40000, NAND},
			{"bootimg",6291456,0xbc0000, NAND},
			{"recovery",6291456,0x11c0000, NAND},
			{"sec_ro",262144,0x17c0000, NAND},
			{"misc",786432,0x1800000, NAND},
			{"logo",1048576,0x18c0000, NAND},
			{"expdb",2097152,0x19c0000, NAND},
			{"fat",0,0x1bc0000, NAND},
			{"android",482082816,0x1bc0000, NAND},
			{"cache",7340032,0x1e780000, NAND},
			{"usrdata",0,0x1ee80000, NAND},
			{"bmtpool",10485760,0xFFFF0028, NAND},
 };

#ifdef  MTK_EMMC_SUPPORT
struct MBR_EBR_struct MBR_EBR_px[MBR_COUNT]={
	{"mbr", {}},
};

EXPORT_SYMBOL(MBR_EBR_px);
#endif

