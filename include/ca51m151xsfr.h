#ifndef __CA51M151_XDATA_H__
#define __CA51M151_XDATA_H__

#define XRAM_AT_(x)       (*((volatile unsigned char xdata *)(x)))


#define     P00F        XRAM_AT_(0x8000)
#define     P01F        XRAM_AT_(0x8001)
#define     P02F        XRAM_AT_(0x8002)
#define     P03F        XRAM_AT_(0x8003)
#define     P04F        XRAM_AT_(0x8004)
#define     P05F        XRAM_AT_(0x8005)
#define     P06F        XRAM_AT_(0x8006)
#define     P07F        XRAM_AT_(0x8007)

#define     P10F        XRAM_AT_(0x8008)
#define     P11F        XRAM_AT_(0x8009)
#define     P12F        XRAM_AT_(0x800a)
#define     P13F        XRAM_AT_(0x800b)
#define     P14F        XRAM_AT_(0x800c)
#define     P15F        XRAM_AT_(0x800d)
#define     P16F        XRAM_AT_(0x800e)
#define     P17F        XRAM_AT_(0x800f)

#define     P20F        XRAM_AT_(0x8010)
#define 	P21F        XRAM_AT_(0x8011)
#define 	P22F        XRAM_AT_(0x8012)
#define 	P23F        XRAM_AT_(0x8013)
#define 	P24F        XRAM_AT_(0x8014)
#define 	P25F        XRAM_AT_(0x8015)
#define 	P26F        XRAM_AT_(0x8016)
#define 	P27F        XRAM_AT_(0x8017)

#define 	P30F        XRAM_AT_(0x8018)
#define 	P31F        XRAM_AT_(0x8019)

#define 	ADCON       XRAM_AT_(0x8060)
#define 	ADCFGL      XRAM_AT_(0x8061)
#define 	ADCDL       XRAM_AT_(0x8062)
#define 	ADCDH       XRAM_AT_(0x8063)
#define 	ADCALL      XRAM_AT_(0x8064)
#define 	ADCALH      XRAM_AT_(0x8065)

#define 	S1RELL      XRAM_AT_(0x8068)
#define 	S1RELH      XRAM_AT_(0x8069)

#define 	P00C        XRAM_AT_(0x8120)
#define 	P01C        XRAM_AT_(0x8121)
#define 	P02C        XRAM_AT_(0x8122)
#define 	P03C        XRAM_AT_(0x8123)
#define 	P04C        XRAM_AT_(0x8124)
#define 	P05C        XRAM_AT_(0x8125)
#define 	P06C        XRAM_AT_(0x8126)
#define 	P07C        XRAM_AT_(0x8127)

#define 	P10C        XRAM_AT_(0x8128)
#define 	P11C        XRAM_AT_(0x8129)
#define 	P12C        XRAM_AT_(0x812a)
#define 	P13C        XRAM_AT_(0x812b)
#define 	P14C        XRAM_AT_(0x812c)
#define 	P15C        XRAM_AT_(0x812d)
#define 	P16C        XRAM_AT_(0x812e)
#define 	P17C        XRAM_AT_(0x812f)

#define 	P20C        XRAM_AT_(0x8130)
#define 	P21C        XRAM_AT_(0x8131)
#define 	P22C        XRAM_AT_(0x8132)
#define 	P23C        XRAM_AT_(0x8133)
#define 	P24C        XRAM_AT_(0x8134)
#define 	P25C        XRAM_AT_(0x8135)
#define 	P26C        XRAM_AT_(0x8136)
#define 	P27C        XRAM_AT_(0x8137)

#define 	P30C        XRAM_AT_(0x8138)
#define 	P31C        XRAM_AT_(0x8139)

#define 	MECON       XRAM_AT_(0xfc00)
#define 	FSCMD       XRAM_AT_(0xfc01)
#define 	FSDAT       XRAM_AT_(0xfc02)
#define 	LOCK        XRAM_AT_(0xfc03)
#define 	PTSL        XRAM_AT_(0xfc05)
#define 	PTSH        XRAM_AT_(0xfc06)


#endif  
