#include "hal.h"
#include "i2c_ee.h"
#include "Si47xx.h"

#define GPIO_INPUT		0		/* IO in input */
#define GPIO_OUTPUT		1		/* IO in output */

#define RST_LOW			(GPIOD->BRR=GPIO_Pin_14)
#define RST_HIGH		(GPIOD->BSRR=GPIO_Pin_14)

#define	SCLK_LOW		(GPIOD->BRR=GPIO_Pin_12)
#define SCLK_HIGH		(GPIOD->BSRR=GPIO_Pin_12)

#define SDIO_LOW		(GPIOD->BRR=GPIO_Pin_13)
#define SDIO_HIGH		(GPIOD->BSRR=GPIO_Pin_13)

/***********************************************************
进入2线模式,必须保证在RESET期间:
		GPIO2		为低电平
		GPIO1		为高电平
		I2C_CLK 		为高电平
		I2C_DAT		为低电平
		并在前后保持100mS以上		
************************************************************/
#define SI47XX_CHIPADDR   0xc6

static unsigned char write_buf[8];


void si47xx_Configuration()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	
	/* PB5, 6, 7输出 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;		//开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50M时钟速度
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void ResetSi47XX_2w(void)
{
	// 设置RST, SCLK, SDIO端口为输出
	si47xx_Configuration();

	SDIO_LOW;
	RST_LOW;
	SCLK_HIGH;
	DelayMS(100);
	RST_HIGH;
	DelayMS(1);
	SDIO_HIGH;
	DelayMS(1);

	// 设置SCLK, SDIO端口为I2C
	I2C_InitGPIO();
}

static unsigned short loop_counter = 0;
static unsigned char error_ind;
static unsigned char res;

unsigned char OperationSi47XX_2w(unsigned char operation, unsigned char *data1, unsigned char numBytes)
{
	if(operation == READ)
		return I2CReadNBytes(SI47XX_CHIPADDR, 0, 0, data1, numBytes)==0;
	else 
		return I2CWriteNBytes(SI47XX_CHIPADDR, 0, 0, data1, numBytes)==0;

}

unsigned char Si47XX_Wait(void)
{
	loop_counter = 0;
	error_ind = 0;
	do{
		error_ind= OperationSi47XX_2w(READ, &res, 1);
		if(error_ind)
			return I2C_ERROR;
		loop_counter++;
	}while( ((res&0x80)!=0x80)&&(loop_counter<0xFF) );
	if(loop_counter>=0xFF)
		return LOOP_EXP_ERROR;
	return OK;
}

unsigned char Si47XX_Wait_STC(void)
{
	loop_counter = 0;
	do{
		res=0x14;
		error_ind = OperationSi47XX_2w(WRITE, &res, 1);
		if(error_ind)
			return I2C_ERROR;
		error_ind = OperationSi47XX_2w(READ, &res, 1);
		if(error_ind)
			return I2C_ERROR;
		loop_counter++;
	}while( ((res&0x81)!=0x81)&&(loop_counter<0xFFF0) );
	if(loop_counter>=0xFFF0)
		return LOOP_EXP_ERROR;
	return OK;
}

unsigned char Si47XX_Power_Up(unsigned char  mod)
{	
	write_buf[0] = 0x01;
	switch(mod){
		case FM_RECEIVER:		// 缺省值
			write_buf[1] = 0xd0;
			write_buf[2] = 0x05;			
			break;
	    case FM_TRNSMITTER:
	    	write_buf[1] = 0xd2;
	    	write_buf[2] = 0x50;
	    	break;	 
			
	    case AM_RECEIVER:
	    	write_buf[1] = 0xd1;
	    	write_buf[2] = 0x05;
	    	break;
   }
	// ResetSi47XX_2w();
	// DelayMS(150);
	error_ind = OperationSi47XX_2w(WRITE, &(write_buf[0]), 3);//need wait 300mS
	if(error_ind)
		return I2C_ERROR;
	DelayMS(150);
	DelayMS(150);
	return Si47XX_Wait();
}

unsigned char Si47XX_Get_Rev(unsigned char *p)
{
	write_buf[0]=0x10;
	write_buf[1]=0;
 	error_ind = OperationSi47XX_2w(WRITE, write_buf, 2);
	if(error_ind)
		return I2C_ERROR;

	res= Si47XX_Wait();
	if(res!=OK)
		return res;
	error_ind =OperationSi47XX_2w(READ, p, 9);	
	if(error_ind)
		return I2C_ERROR;
		
	return res;	
}

/**************************************
Si47XX_Power_Down()
***************************************/
unsigned char Si47XX_Power_Down(void)
{
	unsigned char  Si47XX_power_down = 0x11;	

	//send CMD
 	error_ind = OperationSi47XX_2w(WRITE, &Si47XX_power_down, 1);
	if(error_ind)
		return I2C_ERROR;

	//wait CTS = 1
	return Si47XX_Wait();
}

/*
void TX_TUNE_POWER(unsigned char value )
{
	write_buf[0]=0x31;
	write_buf[1]=0x00;
	write_buf[2]=0x00;
	write_buf[3]=value;						
	write_buf[4]= 0;
	write_buf[5]= 0;
 	OperationSi47XX_2w(WRITE, &(write_buf[0]), 6);
}


void Si47XX_Tune_Measure_RPS(unsigned short tune_freq, unsigned char *noise_level)
{
	Si47XX_Tune(TX_TUNE_MEASURE,tune_freq);
	Si47XX_RSQ_Status(FM_TRNSMITTER,1,&write_buf[8]);
	*noise_level = write_buf[7];
}

void Si47XX_Min_RNL_Channel_RPS(unsigned short start_channel, unsigned short stop_channel, unsigned short *Min_RNL_Channel)
{
	unsigned char code weight_coeff[] = {1,2,6,2,1};
	unsigned char Last_Five_RNL[] = {0,0,0,0,0};
	unsigned char j, noise_level;
	unsigned short i, current_channel_RNL, last_channel_RNL = 65535;

	for(i=(start_channel-10*2),j=0; i<=(stop_channel+10*2); i=i+10,j++){
		if((i>=8750)&&(i<=10800)){
			Si47XX_Tune_Measure_RPS(i, &noise_level);
			Last_Five_RNL[j%5] = noise_level;
		}else
			Last_Five_RNL[j%5] = 0;
		
		if(j>=4){																															//RNL_index	coeff_index
			current_channel_RNL = 	Last_Five_RNL[j%5]*weight_coeff[4] + 		// 4->0->1		4->4->4
									Last_Five_RNL[(j-1)%5]*weight_coeff[3] + // 3->4->0		3->3->3
									Last_Five_RNL[(j-2)%5]*weight_coeff[2] + // 2->3->4		2->2->2
									Last_Five_RNL[(j-3)%5]*weight_coeff[1] + // 1->2->3		1->1->1
									Last_Five_RNL[(j-4)%5]*weight_coeff[0];	// 0->1->2		0->0->0
		  	if(current_channel_RNL <= last_channel_RNL){
		  		*Min_RNL_Channel = i - 10*2;		  	
		 	 	last_channel_RNL = current_channel_RNL;
		  	}
		}
	}	

}
*/

void Si47XX_Set_Property(unsigned char  ProertyH,unsigned char  ProertyL,unsigned char valueH,unsigned char valueL )
{
	write_buf[0]=0x12;
	write_buf[1]=0x00;
	write_buf[2]= ProertyH;
	write_buf[3]=ProertyL;
	write_buf[4]=valueH;
	write_buf[5]=valueL;						
 	OperationSi47XX_2w(WRITE, &(write_buf[0]), 6);
	//Si47XX_Wait_STC();	
	Si47XX_Wait();
}
/*
void Si47XX_Get_Property(unsigned char  ProertyH,unsigned char  ProertyL,unsigned char *valueH,unsigned char *valueL )
{
	write_buf[0]=0x13;
	write_buf[1]=0x00;	
	write_buf[2]=ProertyH;
	write_buf[3]=ProertyL;
 	OperationSi47XX_2w(WRITE, &(write_buf[0]), 4);
	OperationSi47XX_2w(READ, &(write_buf[0]), 4);
	*valueH  = write_buf[2];
	*valueL  = write_buf[3];
}
*/
unsigned char Si47XX_Tune(char mod,unsigned short Channel_Freq)
{
	write_buf[0]= mod;	
	write_buf[1]= 0x00;
	write_buf[2] = (Channel_Freq&0xff00) >> 8;
	write_buf[3] = (Channel_Freq&0x00ff);
	write_buf[4]= 0x00;

 	error_ind = OperationSi47XX_2w(WRITE, &(write_buf[0]), 5);		
	if(error_ind)
	{
		return I2C_ERROR;
	}
	DelayMS(100);
	if(Si47XX_Wait() != OK) return ERROR;	
	return OK;
}



unsigned char Si47XX_Tune_Status(char mod,char cancel,char int_clear,unsigned char *p)
{
	write_buf[0] = 0x02|mod;
	write_buf[1] = 0x00;
	if(int_clear) 	write_buf[1] |= 0x01;
	if(cancel) 		write_buf[1] |= 0x02;
 	error_ind = OperationSi47XX_2w(WRITE, write_buf, 2);
	if(error_ind)
		return I2C_ERROR;
	res= Si47XX_Wait();
	if(res!=OK)
		return res;

	error_ind = OperationSi47XX_2w(READ, p, 8);
	if(error_ind)
		return I2C_ERROR;
	if((p[1]&0x01) == 0)
		return ERROR;
	return OK;
/*
#if 0
	if(((*(p+1)&0x80) == 0))
		SeekFail = 0;
	else
		SeekFail = 1;		
	if((*(p+1)&0x01) == 0)
		valid_channel = 0;
	else
		valid_channel = 1;	
#endif
*/
}

/*
void Si47XX_RSQ_Status(char mod,char int_clear,unsigned char *p)
{
	write_buf[0] = 0x03|mod;
	write_buf[1] = 0x00;
	if(int_clear) 	write_buf[1] |= 0x01;
 	error_ind = OperationSi47XX_2w(WRITE, &(write_buf[0]), 2);
	if(error_ind)
		return ;	//I2C_ERROR;

	OperationSi47XX_2w(READ, p, 8);
}

unsigned char Si47XX_Seek_Start(char mod,unsigned char seek_mod)
{
	write_buf[0] = 0x01|mod;	
	write_buf[1] = seek_mod;

 	error_ind = OperationSi47XX_2w(WRITE, &(write_buf[0]), 2);
	if(error_ind)
		return I2C_ERROR;

	return Si47XX_Wait();
}
*/

//seek 在50khz,10khz step时会有一些问题,需特殊判断
#if 0
unsigned char  Si47XX_Seek(char mod,unsigned char seek_mod,unsigned char *p)
{
	bit valid_chn;


	loop_counter = 0;
	RX_HARD_MUTE(3);
	do{
		if(Si47XX_Seek_Start(mod,seek_mod)!=OK) return ERROR;
		if(Si47XX_Wait_STC() != OK) return ERROR;
		if(Si47XX_Tune_Status(mod,0,1,p)!=OK) return ERROR;
		loop_counter++;
		valid_chn= p[1]&0x01;
	}while( (valid_chn == 0) && (loop_counter<0xFF) && ((p[1]&0x80) == 0) );

	if(valid_chn == 1)	//		valid =? 0
	{
		RX_HARD_MUTE(0);
		return OK;				
	}
	return ERROR;
}

void Si47XX_Seek(char mod,unsigned char seek_mod)
{
	Si47XX_Seek_Start(mod,seek_mod);
}



unsigned char Si47XX_FM_Seek(unsigned char mod)
{
	unsigned char loop_counter;
	unsigned char loop_counter_2;
	loop_counter_2 = 0;

	Si47XX_Set_Property_FM_HARD_MUTE(3);
	Si47XX_FM_Seek_Start(mod);	
	do{
			loop_counter = 0xff;			
			while(loop_counter--){			
				OperationSi47XX_2w(READ,&read_buf[0],1);			
				if (read_buf[0]&0x01) break;		//STC == 1  exit		
				Si47XX_FM_Tune_Status(0,0);
				DELAY(D_1mS*10);
				//LED_FREQ(Channel_Freq);//display Freq
			}
			Si47XX_FM_Tune_Status(0,1);
			//LED_FREQ(Channel_Freq);			
			loop_counter_2++;
	}while((valid_channel == 0) && (loop_counter_2 < 0xffff) && (SeekFail == 0));  

	if(loop_counter_2 >= 0xffff){
		IOD = 0xf0;
		return LOOP_EXP_ERROR;
		}
	if((mod == SEEKDOWN_WRAP) || (mod == SEEKUP_WRAP))
		if((valid_channel == 1) && (SeekFail == 1))
			SeekFail = 0;
		
	Si47XX_Set_Property_FM_HARD_MUTE(0);
	return OK;
}

unsigned char Si47XX_FM_Seek_50KHz(unsigned char mod)
{
	unsigned char rssi_tmp;
	unsigned int freq_tmp;

	RX_HARD_MUTE(3);

	Si47XX_FM_Seek_Start(mod);
	
	while(1){  //126dBu RSSI
		do{
			loop_counter = 0xff;
			
			while(loop_counter--){			
				OperationSi47XX_2w(READ,&read_buf[0],1);			
				if (read_buf[0] & 0x01)
	    				break;
				
				Si47XX_FM_Tune_Status(0,0);
				DELAY(D_150mS);
				LED_FREQ(Channel_Freq);
				}		
			Si47XX_FM_Tune_Status(0,1);
			LED_FREQ(Channel_Freq);
			
			loop_counter_2++;
			}while((valid_channel == 0) && (loop_counter_2 < 0xffff) && (SeekFail == 0));  
		

			rssi_tmp = read_buf[4];
			freq_tmp =Channel_Freq;
			
//+250KHz
			Si47XX_FM_Tune_Freq(freq_tmp+25);
			Si47XX_FM_Tune_Status(0,1);
			if(rssi_tmp<read_buf[4]){
				Si47XX_FM_Tune_Freq(freq_tmp);
				Si47XX_FM_Seek_Start(mod);
				continue;		
			}
// -250KHz
			Si47XX_FM_Tune_Freq(freq_tmp-25);
			Si47XX_FM_Tune_Status(0,1);
			if(rssi_tmp<read_buf[4]){
				Si47XX_FM_Tune_Freq(freq_tmp);
				Si47XX_FM_Seek_Start(mod);
				continue;		
			}
//+250KHz			
			Si47XX_FM_Tune_Freq(freq_tmp+15);
			Si47XX_FM_Tune_Status(0,1);
			if(rssi_tmp<read_buf[4]){
				Si47XX_FM_Tune_Freq(freq_tmp);
				Si47XX_FM_Seek_Start(mod);
				continue;		
			}
// -150KHz
			Si47XX_FM_Tune_Freq(freq_tmp-15);
			Si47XX_FM_Tune_Status(0,1);
			if(rssi_tmp<read_buf[4]){
				Si47XX_FM_Tune_Freq(freq_tmp);
				Si47XX_FM_Seek_Start(mod);
				continue;		
			}

			Si47XX_FM_Tune_Freq(freq_tmp);
			break;
		}
	if(loop_counter_2 >= 0xffff)
		return LOOP_EXP_ERROR;
		
	if((mod == SEEKDOWN_WRAP) || (mod == SEEKUP_WRAP))
		if((valid_channel == 1) && (SeekFail == 1))
			SeekFail = 0;
		
	RX_HARD_MUTE(0);
	return OK;
}

unsigned char Si47XX_FM_Seek_10KHz(unsigned char mod)
{
	unsigned char loop_counter;
	unsigned char loop_counter_2;
	unsigned char rssi_tmp;
	unsigned int freq_tmp;
	loop_counter_2 = 0;

	Si47XX_Set_Property_FM_HARD_MUTE(3);

	Si47XX_FM_Seek_Start(mod);
	
	while(1){  //126dBu RSSI
		do{
			loop_counter = 0xff;
			
			while(loop_counter--){			
				OperationSi47XX_2w(READ,&read_buf[0],1);			
				if (read_buf[0] & 0x01)
	    				break;
				
				Si47XX_FM_Tune_Status(0,0);
				DELAY(D_150mS);
				LED_FREQ(Channel_Freq);
				}		
			Si47XX_FM_Tune_Status(0,1);
			LED_FREQ(Channel_Freq);
			
			loop_counter_2++;
			}while((valid_channel == 0) && (loop_counter_2 < 0xffff) && (SeekFail == 0));  
		

			rssi_tmp = read_buf[4];
			freq_tmp =Channel_Freq;
			
//+250KHz
			Si47XX_FM_Tune_Freq(freq_tmp+25);
			Si47XX_FM_Tune_Status(0,1);
			if(rssi_tmp<read_buf[4]){
				Si47XX_FM_Tune_Freq(freq_tmp);
				Si47XX_FM_Seek_Start(mod);
				continue;		
			}
// -250KHz
			Si47XX_FM_Tune_Freq(freq_tmp-25);
			Si47XX_FM_Tune_Status(0,1);
			if(rssi_tmp<read_buf[4]){
				Si47XX_FM_Tune_Freq(freq_tmp);
				Si47XX_FM_Seek_Start(mod);
				continue;		
			}
//+250KHz			
			Si47XX_FM_Tune_Freq(freq_tmp+15);
			Si47XX_FM_Tune_Status(0,1);
			if(rssi_tmp<read_buf[4]){
				Si47XX_FM_Tune_Freq(freq_tmp);
				Si47XX_FM_Seek_Start(mod);
				continue;		
			}
// -150KHz
			Si47XX_FM_Tune_Freq(freq_tmp-15);
			Si47XX_FM_Tune_Status(0,1);
			if(rssi_tmp<read_buf[4]){
				Si47XX_FM_Tune_Freq(freq_tmp);
				Si47XX_FM_Seek_Start(mod);
				continue;		
			}

			Si47XX_FM_Tune_Freq(freq_tmp);
			break;
		}
	if(loop_counter_2 >= 0xffff)
		return LOOP_EXP_ERROR;
		
	if((mod == SEEKDOWN_WRAP) || (mod == SEEKUP_WRAP))
		if((valid_channel == 1) && (SeekFail == 1))
			SeekFail = 0;
		
	Si47XX_Set_Property_FM_HARD_MUTE(0);
	return OK;
}
#endif
