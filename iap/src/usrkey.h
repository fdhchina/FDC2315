#ifndef USRKEY_H
#define USRKEY_H

//最大允许的键值
#define MAX_KEY_NO		16		//20个按键

//KEY-各个标志位，为了节省内存，使用位方式
#define KEY_M_PUSH 			1				//按键有效
#define KEY_M_BCK_TIME 		(1<<1)			//按键开始计算消抖此标志还没代表按键有效
#define KEY_M_RELEASE		(1<<2)			//按键释放
#define KEY_M_BPK_TIME		(1<<3)			//连按标志

//KEY 消抖计数器
#define KEY_BCK_PUSH_TIME		50				//消抖计数器是1000HZ的，则0.05秒完成消抖
//KEY连按计数器
#define KEY_PBK_BEGIN_TIME		1500				// 1.5秒后开始连按
#define KEY_PBK_POLLING_TIME	250				//连按后按键速度为1/4秒

/**************************************************************
	外接引入的按键改变GUI信息.注意下列的宏定义仅仅是引入EDIT的信息,不需要与硬件的键值一一对应
***************************************************************/
enum {
	VK_INVALID=		0,
	VK_NUM0,
	VK_NUM1,
	VK_NUM2,
	VK_NUM3,
	VK_NUM4,
	VK_NUM5,
	VK_NUM6,
	VK_NUM7,
	VK_NUM8,
	VK_NUM9,
	VK_UP,
	VK_DOWN,
	VK_LEFT,
	VK_RIGHT,
	VK_ENTER,
	VK_ESC
 };	

void kb_Init(void);
u8 kb_GetKey(void);

//extern void CountingKey(void);					//消抖计数器,请放到1KHZ定时器上
//extern u8 ManageKey(u16 key);					//键盘管理函数
#endif

