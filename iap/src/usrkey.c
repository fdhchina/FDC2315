/**************************************************************
usrkey.c 用户按键处理,此处还包括了用户按键与GUI的接口
***************************************************************/
#include "hal.h"
#include "usrkey.h"

extern u8 PollingKey(void);
extern void Key_Configuration(void);
void kb_Init()
{
	Key_Configuration();
}

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
u8 kb_GetKey()
{
	static volatile u16 New_Key=0;
	static volatile u16 Old_Key=0;		//键值

	static u32 dwKeyPressCount;
	static u8	byKeyPressed=0;
	New_Key= PollingKey();
	if(New_Key!=Old_Key)
	{
		dwKeyPressCount= 0;
		// EXTI_ClearITPendingBit(EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15);
		if(New_Key)		// 新按键
		{
			Old_Key= New_Key;
			// return Key_tran[GetKeyPressed(New_Key, 0)];
		}else
		{
			Old_Key= 0;
		}
		byKeyPressed=0;
	}else
	{
		New_Key= PollingKey();
		if(New_Key!=Old_Key)
			dwKeyPressCount= 0;
	 	if(New_Key)
		{
			if(byKeyPressed<11)
				byKeyPressed++;
			dwKeyPressCount++;
			if(byKeyPressed==10)
				return Key_tran[GetKeyPressed(New_Key, 0)];
			else
				if(byKeyPressed>10)
				if((dwKeyPressCount>1000)&&((dwKeyPressCount&0xFF)==0))
					return Key_tran[GetKeyPressed(New_Key, 0)];
		} else
		{
			dwKeyPressCount= 0;
			byKeyPressed=0;
			Old_Key= 0;
		}
	}
	return VK_INVALID;
}

//按键转换函数,硬件值对应键盘宏
const u8 Key_tran[17]={
	VK_INVALID,					// 0,   无键	键值
	//第一行
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
	//第四行
};

