/**************************************************************
usrkey.c 用户按键处理,此处还包括了用户按键与GUI的接口
***************************************************************/
#include "hal.h"
#include "usrkey.h"

extern u8 PollingKey(void);
void Key_Config(void);

void kb_Init()
{
	Key_Config();
}

static volatile u32 New_Key=0;
static volatile u32 Old_Key=0;		//键值
static u8 kb_longPress= 0;
u8 GetKeyPressed(u16 keymask, u8 nIndex)	// 取第nIndex有效按键
{
	u8 i, n=0;
	for(i=1;i<17;i++)
	{
		if((keymask&0x01)&&(n==nIndex))
		{
			return i;
		}
		keymask>>=1;
	}
	return 0x0;
}

extern const u8 Key_tran[];
u8 kb_getKey()
{
	static u32 dwKeyPressCount;
	static u8	byKeyPressed=0;
	u8 k;
	New_Key= PollingKey();
	if(New_Key)
	{
		DelayMS(10);
		if(New_Key==PollingKey())
		{
			if(New_Key==Old_Key)
			{
				if(byKeyPressed<25)
					byKeyPressed++;
				dwKeyPressCount++;
				if(byKeyPressed>24)
				{
					kb_longPress= 1;
					if((dwKeyPressCount&0x7)==0)
						return Key_tran[GetKeyPressed(New_Key, 0)];
				}
			}else
			{
				Old_Key= New_Key;
				kb_longPress= 0;
				dwKeyPressCount= 0;
				byKeyPressed=0;
			}
		}
	}else
	{
		dwKeyPressCount= 0;
		if(Old_Key&&(kb_longPress==0))
		{
			byKeyPressed=0;
			k= Key_tran[GetKeyPressed(Old_Key, 0)];
			Old_Key= 0;
			return k;
		}
		kb_longPress= 0;
		byKeyPressed=0;
		Old_Key= 0;
	}
		
	return VK_INVALID;
}

u8 kb_IslongPress()
{
	return kb_longPress;
}

void _keyscan(void) __irq
{
	New_Key= PollingKey();
	//if(New_Key!=Old_Key)		// 新按键
	//{
	//	New_Key= ;
	//	Old_Key= 0;
	//}
}

//按键转换函数,硬件值对应键盘宏
const u8 Key_tran[17]={
	VK_INVALID,					// 0,   无键	键值
	//第一行
	/*VK_NUM0,     				//  1
	VK_NUM4,     				//  2
	VK_NUM5,     				//  3
	VK_NUM9,     				//  4
	VK_NUM1,		     		//  5
	VK_ENTER,		     		//  6
	VK_NUM6, 		    		//  7
	VK_ESC,  				//  8
	VK_NUM2, 	    			//  9
	VK_UP, 	    				//  10
	VK_NUM7,		 		//  11
	VK_LEFT,			     		//  12
	VK_NUM3,				//  13
	VK_RIGHT,     			//  14
	VK_NUM8,				//  15
	VK_DOWN,     			//  16
	//第四行*/
	
	VK_NUM3,//     				//  1
	VK_NUM2, //    				//  2
	VK_NUM1,     	//			//  3
	VK_NUM0,     		//		//  4
	VK_NUM4,	//	     		//  5
	VK_ENTER,		     		//  6
	VK_UP,// 		    		//  7
	VK_DOWN,//  				//  8
	VK_NUM8, //	    			//  9
	VK_NUM7, // 	    				//  10
	VK_NUM9,	//	 		//  11
	VK_ESC,	//		     		//  12
	VK_NUM6,	//			//  13
	VK_NUM5,    // 			//  14
	VK_RIGHT,				//  15
	VK_LEFT,//     			//  16
};

