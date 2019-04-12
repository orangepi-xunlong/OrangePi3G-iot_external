#include "partition_define.h"
static const struct excel_info PartInfo_Private[PART_NUM]={
			{"preloader",20971520,0x0, EMMC, 0,BOOT_1},
			{"mbr",524288,0x1400000, EMMC, 0,USER},
			{"ebr1",524288,0x1480000, EMMC, 1,USER},
			{"pro_info",3145728,0x1500000, EMMC, 0,USER},
			{"nvram",5242880,0x1800000, EMMC, 0,USER},
			{"protect_f",10485760,0x1d00000, EMMC, 2,USER},
			{"protect_s",10485760,0x2700000, EMMC, 3,USER},
			{"seccfg",131072,0x3100000, EMMC, 0,USER},
			{"uboot",393216,0x3120000, EMMC, 0,USER},
			{"bootimg",6291456,0x3180000, EMMC, 0,USER},
			{"recovery",6291456,0x3780000, EMMC, 0,USER},
			{"sec_ro",262144,0x3d80000, EMMC, 0,USER},
			{"misc",524288,0x3dc0000, EMMC, 0,USER},
			{"logo",3145728,0x3e40000, EMMC, 0,USER},
			{"expdb",10485760,0x4140000, EMMC, 0,USER},
			{"android",3730833408,0x4b40000, EMMC, 4,USER},
			{"cache",0,0xe3140000, EMMC, 5,USER},
			{"usrdata",0,0xe3140000, EMMC, 6,USER},
			{"fat",0,0xe3140000, EMMC, 7,USER},
			{"bmtpool",22020096,0xFFFF00a8, EMMC, 0,USER},
 };

#ifdef  MTK_EMMC_SUPPORT
struct MBR_EBR_struct MBR_EBR_px[MBR_COUNT]={
	{"mbr", {1, 2, 3, 4, }},
	{"ebr1", {5, 6, 7, }},
};

EXPORT_SYMBOL(MBR_EBR_px);
#endif

