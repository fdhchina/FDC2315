#ifndef _SI47XX_H
#define _SI47XX_H

#define    READ  			1
#define    WRITE  			2

#define    OK  				1
#define    I2C_ERROR  		2
#define    LOOP_EXP_ERROR  	3
#define    ERROR  			4

#define    SEEKDOWN_HALT 	0x00
#define    SEEKDOWN_WRAP  	0x04
#define    SEEKUP_HALT  	0x08
#define    SEEKUP_WRAP  	0x0c

#define    FM_RECEIVER  	0x20
#define    FM_TRNSMITTER  	0x30
#define    AM_RECEIVER  	0x40
#define    TX_TUNE_MEASURE 	0x32


#define DELAY(DURATION)		{volatile unsigned short i = DURATION; while(i--);}  // 1uS/cyclce 

void ResetSi47XX_2w(void);
unsigned char OperationSi47XX_2w(unsigned char operation, unsigned char *data1, unsigned char numBytes);

unsigned char Si47XX_Power_Up(unsigned char  mod);
unsigned char Si47XX_Get_Rev(unsigned char *p);
unsigned char Si47XX_Power_Down(void);

// void TX_TUNE_POWER(unsigned char value);
void Si47XX_Tune_Measure_RPS(unsigned short tune_freq, unsigned char *noise_level);
void Si47XX_Min_RNL_Channel_RPS(unsigned short start_channel, unsigned short stop_channel, unsigned short *Min_RNL_Channel);

unsigned char Si47XX_Tune(char mod,unsigned short Channel_Freq);
unsigned char Si47XX_Tune_Status(char mod,char cancel,char int_clear,unsigned char *p);
// unsigned char Si47XX_Seek_Start(char mod,unsigned char seek_mod);
// void Si47XX_Seek(char mod,unsigned char seek_mod);
// unsigned char  Si47XX_Seek(char mod,unsigned char seek_mod,unsigned char *P);

// void Si47XX_RSQ_Status(char mod,char int_clear,unsigned char *p);


void Si47XX_Set_Property(unsigned char  ProertyH,unsigned char  ProertyL,unsigned char valueH,unsigned char valueL );

struct  ch_t{
	//char name[16];
	unsigned a:4;
	unsigned b:4;
	//unsigned c:1;
//	unsigned c:4;
//	unsigned d:4;
};

#define GPO_IEN(valueH,valueL)							Si47XX_Set_Property(0x00,0x01,valueH,valueL)//0x0000


//0x02xx Si4705/06/21/31/35/37/39 Only
#define DIGITAL_OUTPUT_FORMAT(valueH,valueL)			Si47XX_Set_Property(0x01,0x02,valueH,valueL)//0x0000
#define DIGITAL_OUTPUT_SAMPLE_RATE(valueH,valueL)		Si47XX_Set_Property(0x01,0x04,valueH,valueL)//0x0000


#define REFCLK_FREQ(valueH,valueL)						Si47XX_Set_Property(0x02,0x01,valueH,valueL)//0x8000
#define REFCLK_PRESCALE(valueH,valueL)					Si47XX_Set_Property(0x02,0x02,valueH,valueL)//0x0001


//0x1100 not applicable for Si4749
#define FM_DEEMPHASIS(valueH,valueL)  					Si47XX_Set_Property(0x11,0x00,valueH,valueL)//0x0002;
//0x1102 Si4706/49 Only
#define FM_CHANNEL_FILTER(valueH,valueL)				Si47XX_Set_Property(0x11,0x02,valueH,valueL)//0x0001
//0x1105 Not applicable for Si4706/40/41/49
#define FM_BLEND_STEREO_THRESHOLD(valueH,valueL)		Si47XX_Set_Property(0x11,0x05,valueH,valueL)//0x0031
//0x1106 Not applicable for Si4706/40/41/49
#define FM_BLEND_MONO_THRESHOLD(valueH,valueL)			Si47XX_Set_Property(0x11,0x06,valueH,valueL)//0x001e
//0x1107 Si4704/05/06/20/21 Only
#define FM_Antenna_Input(valueH,valueL)					Si47XX_Set_Property(0x11,0x07,valueH,valueL)//0x0000;
#define FM_MAX_TUNE_ERROR(valueH,valueL)				Si47XX_Set_Property(0x11,0x08,valueH,valueL)//0x001e


#define FM_RSQ_INT_SOURCE(valueH,valueL)				Si47XX_Set_Property(0x12,0x00,valueH,valueL)//0x0000
#define FM_RSQ_SNR_HI_THRESHOLD(valueH,valueL)			Si47XX_Set_Property(0x12,0x01,valueH,valueL)//0x007f
#define FM_RSQ_SNR_LO_THRESHOLD(valueH,valueL)			Si47XX_Set_Property(0x12,0x02,valueH,valueL)//0x0000
#define FM_RSQ_RSSI_HI_THRESHOLD(valueH,valueL)			Si47XX_Set_Property(0x12,0x03,valueH,valueL)//0x007f
#define FM_RSQ_RSSI_LO_THRESHOLD(valueH,valueL)			Si47XX_Set_Property(0x12,0x04,valueH,valueL)//0x0000
//0x1205 Si4706/40/41/49 Only
#define FM_RSQ_MULTIPATH_HI_THRESHOLD(valueH,valueL) 	Si47XX_Set_Property(0x12,0x05,valueH,valueL)//0x007f;
//0x1206 Si4706/40/41/49 Only
#define FM_RSQ_MULTIPATH_LO_THRESHOLD(valueH,valueL) 	Si47XX_Set_Property(0x12,0x06,valueH,valueL)//0x0000;
#define FM_RSQ_BLEND_THRESHOLD(valueH,valueL)			Si47XX_Set_Property(0x12,0x07,valueH,valueL)//0x0081


//0x13xx not applicable for Si4749
#define FM_SOFT_MUTE_RATE(valueH,valueL)				Si47XX_Set_Property(0x13,0x00,valueH,valueL)//0x0040
#define FM_SOFT_MUTE_MAX_ATTENUATION(valueH,valueL) 	Si47XX_Set_Property(0x13,0x02,valueH,valueL)//0x0010;
#define FM_SOFT_MUTE_SNR_THRESHOLD(valueH,valueL)		Si47XX_Set_Property(0x13,0x03,valueH,valueL)//0x0004


#define FM_SEEK_BAND_BOTTOM(valueH,valueL)				Si47XX_Set_Property(0x14,0x00,valueH,valueL)//0x222e
#define FM_SEEK_BAND_TOP(valueH,valueL)					Si47XX_Set_Property(0x14,0x01,valueH,valueL)//0x2a26;	
#define FM_SEEK_SPACE(valueH,valueL)					Si47XX_Set_Property(0x14,0x02,valueH,valueL)//0x000A;
#define FM_SNR_Threshold(valueL)						Si47XX_Set_Property(0x14,0x03,0,valueL)//0x0003;
#define FM_RSSI_Threshold(valueL) 						Si47XX_Set_Property(0x14,0x04,0,valueL)//0x0014;



//0x1500 Si4705/06/21/31/35/37/39/41/49 Only
#define RDS_INT_SOURCE(valueH,valueL)					Si47XX_Set_Property(0x15,0x00,valueH,valueL)//0x0000
//0x1501 Si4705/06/21/31/35/37/39/41/49 Only
#define RDS_INT_FIFO_COUNT(valueH,valueL)				Si47XX_Set_Property(0x15,0x01,valueH,valueL)//0x0000
//0x1502 Si4705/06/21/31/35/37/39/41/49 Only
#define RDS_CONFIG(valueH,valueL)						Si47XX_Set_Property(0x15,0x02,valueH,valueL)//0x0000
//0x1503 Si4706/49 Only
#define FM_RDS_CONFIDENCE(valueH,valueL)				Si47XX_Set_Property(0x15,0x03,valueH,valueL)//0x1111


//0x18xx only Si4740,Si4741
#define FM_BLEND_RSSI_STEREO_THRESHOLD(valueH,valueL)		Si47XX_Set_Property(0x18,0x00,valueH,valueL)//0x0031
#define FM_BLEND_RSSI_MONO_THRESHOLD(valueH,valueL)			Si47XX_Set_Property(0x18,0x01,valueH,valueL)//0x001e
#define FM_BLEND_RSSI_ATTACK_RATE(valueH,valueL)			Si47XX_Set_Property(0x18,0x02,valueH,valueL)//0x03e8
#define FM_BLEND_RSSI_RELEASE_RATE(valueH,valueL)			Si47XX_Set_Property(0x18,0x03,valueH,valueL)//0x0064
#define FM_BLEND_SNR_STEREO_THRESHOLD(valueH,valueL)		Si47XX_Set_Property(0x18,0x04,valueH,valueL)//0x001e
#define FM_BLEND_SNR_MONO_THRESHOLD(valueH,valueL)			Si47XX_Set_Property(0x18,0x05,valueH,valueL)//0x000e
#define FM_BLEND_SNR_ATTACK_RATE(valueH,valueL)				Si47XX_Set_Property(0x18,0x06,valueH,valueL)//0x03e8
#define FM_BLEND_SNR_RELEASE_RATE(valueH,valueL)			Si47XX_Set_Property(0x18,0x07,valueH,valueL)//0x0064
#define FM_BLEND_MULTIPATH_STEREO_THRESHOLD(valueH,valueL)	Si47XX_Set_Property(0x18,0x08,valueH,valueL)//0x0028
#define FM_BLEND_MULTIPATH_MONO_THRESHOLD(valueH,valueL)	Si47XX_Set_Property(0x18,0x09,valueH,valueL)//0x000a
#define FM_BLEND_MULTIPATH_ATTACK_RATE(valueH,valueL)		Si47XX_Set_Property(0x18,0x0a,valueH,valueL)//0x03e8
#define FM_BLEND_MULTIPATH_RELEASE_RATE(valueH,valueL)		Si47XX_Set_Property(0x18,0x0b,valueH,valueL)//0x000a


//0x40xx not applicable for Si4749
#define RX_VOLUME(valueL)									Si47XX_Set_Property(0x40,0x00,0,valueL)//0x003f
#define RX_HARD_MUTE(valueL)								Si47XX_Set_Property(0x40,0x01,0,valueL)//0x0000


//0x41xx only Si4740,Si4741
#define RF_AGC_ATTACK_RATE(valueH,valueL)					Si47XX_Set_Property(0x41,0x00,valueH,valueL)//0x0004
#define RF_AGC_RELEASE_RATE(valueH,valueL)					Si47XX_Set_Property(0x41,0x01,valueH,valueL)//0x008c




#define AM_DEEMPHASIS(valueH,valueL)    				Si47XX_Set_Property(0x31,0x00,valueH,valueL)//0x0000;
#define AM_CHANNEL_FILTER(valueH,valueL)     			Si47XX_Set_Property(0x31,0x02,valueH,valueL)//0x0003 

#define AM_RSQ_INT_SOURCE(valueH,valueL)     			Si47XX_Set_Property(0x32,0x00,valueH,valueL)//0x0000 
#define AM_RSQ_SNR_HI_THRESHOLD(valueH,valueL)     		Si47XX_Set_Property(0x32,0x01,valueH,valueL)//0x007f 
#define AM_RSQ_SNR_LO_THRESHOLD(valueH,valueL)     		Si47XX_Set_Property(0x32,0x02,valueH,valueL)//0x0000 
#define AM_RSQ_RSSI_HI_THRESHOLD(valueH,valueL)     	Si47XX_Set_Property(0x32,0x03,valueH,valueL)//0x007f 
#define AM_RSQ_RSSI_LO_THRESHOLD(valueH,valueL)     	Si47XX_Set_Property(0x32,0x04,valueH,valueL)//0x0000 

#define AM_SOFT_MUTE_RATE(valueH,valueL)     			Si47XX_Set_Property(0x33,0x00,valueH,valueL)//0x0040 
#define AM_SOFT_MUTE_SLOPE(valueH,valueL)     			Si47XX_Set_Property(0x33,0x01,valueH,valueL)//0x0002 
#define AM_SOFT_MUTE_MAX_ATTENUATION(valueH,valueL)     Si47XX_Set_Property(0x33,0x02,valueH,valueL)//0x0010 
#define AM_SOFT_MUTE_SNR_THRESHOLD(valueH,valueL)     	Si47XX_Set_Property(0x33,0x03,valueH,valueL)//0x000a 

#define AM_SEEK_BAND_BOTTOM(valueH,valueL)    			Si47XX_Set_Property(0x34,0x00,valueH,valueL)//0x020a;
#define AM_SEEK_BAND_TOP(valueH,valueL)    				Si47XX_Set_Property(0x34,0x01,valueH,valueL)//0x06ae;
#define AM_SEEK_FREQ_SPACING(valueH,valueL)    			Si47XX_Set_Property(0x34,0x02,valueH,valueL)//0x0009;	
#define AM_SEEK_TUNE_SNR_THRESHOLD(valueH,valueL)    	Si47XX_Set_Property(0x34,0x03,valueH,valueL)//0x0005;
#define AM_SEEK_TUNE_RSSI_THRESHOLD(valueH,valueL)    	Si47XX_Set_Property(0x34,0x04,valueH,valueL)//0x0019;


#define TX_LINE_INPUT_LEVEL(valueH,valueL)				Si47XX_Set_Property(0x21,0x04,valueH,valueL)//0x327c;



// void Si47XX_Get_Property(unsigned char  ProertyH,unsigned char  ProertyL,unsigned char *valueH,unsigned char *valueL );

// #define Get_FM_Value(valueH,valueL)							Si47XX_Get_Property(0x40,0x00,valueH,valueL)

#endif
