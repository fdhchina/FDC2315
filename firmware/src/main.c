
#include "hal.h"
#include "lcm19264.h"
// #include "player.h"
#include "menu.h"
#include "log.h"

#define	FIRMWARE_ADR_Offset	0x0008000		// 程序起始地址
extern u8 Alarm_key(void);
u8 YK_Key=0xff; //KEY:1111  
u8 YK_SAVE=0;
char YK_Rev[12];
u32 DH_Key=0xffffffff;
u8 DH_SAVE=0;
u8 DH_Rev[12];

char const *key[]={
	"KEY:1111",
	"KEY:2222",
	"KEY:3333",
	"KEY:4444",
	"KEY:5555",
	"KEY:6666",
	"KEY:7777",
	"KEY:8888",
	"KEY:9999",
	"KEY:AAAA",
	"KEY:BBBB",
	"KEY:CCCC"
};

void KMG_Init(void) //看门狗初始化  
{
 	/* IWDG timeout equal to 280 ms (the timeout may varies due to LSI frequency
		dispersion) */
	/* Enable write access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
	IWDG_SetPrescaler(IWDG_Prescaler_256);

 	/* Set counter reload value to 349 */
	IWDG_SetReload(4000);//

 	/* Reload IWDG counter */
	IWDG_ReloadCounter();

	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	IWDG_Enable();
}
extern u8 jx_num;
int main(void)
{
	u8 i;
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, FIRMWARE_ADR_Offset);

	ChipHalInit();			//片内硬件初始化
	ChipOutHalInit();		//片外硬件初始化

	menu_Init();
 
	lcm_Clr();
	menu_Start();
	UART_PutStr(USART2," system is working......",20);
	Log_Init();//日志初始化
	KMG_Init();	
	while(1)
	{
		IWDG_ReloadCounter();
		menu_OnTimer();
		if(Ms_1)
		{
			menu_KeyDo(kb_getKey(), kb_IslongPress());
			menu_KeyDo(Alarm_key(), 0);
			Ms_1= 0;
		}
		if (S_1 || YK_SAVE || DH_SAVE)
		{
			extern void AutoAmpRun(); //每秒执行一次
			AutoAmpRun();
			S_1 = 0;
			//YK
			if(YK_SAVE==1){
				YK_SAVE=0;
				//UART_PutStr(USART2,YK_Rev,8);
				for(i=0;i<12;i++)
				{
					if(strcmp(key[i],YK_Rev) == 0) //收到遥控码
					{
						YK_Key=i;
						break;
					}
				}
			}
			//Phone
			if(DH_SAVE==1)
			{
				DH_SAVE=0;	
				DH_Key=0;
				for(i=0;i<jx_num;i++)
				{
					DH_Key *=10;
					if((DH_Rev[i+2]>=0x30)&&(DH_Rev[i+2]<=0x39))
					{
						DH_Key += DH_Rev[i+2]-0x30;
					}
					else if(DH_Rev[i+2] == 0x61)
					{
						DH_Key=1111110;
						break;
					}
					else
					{
						DH_Key=0xffffffff;
						break;
					}
				} 
				//UART_PutChar(USART2,DH_Key);
				//UART_PutStr(USART2,DH_Rev,10);
				//DH_Rev用完清空
				for(i=0;i<12;i++)
					DH_Rev[i]=0;
			}
		}
	}

}


