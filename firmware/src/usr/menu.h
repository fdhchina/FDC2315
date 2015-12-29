#ifndef _MENU_
#define _MENU_

#define SONGNAME_CHARCOUNT	12
#define AUTOCLOSEBACKLIGHTCOUNTER		30		// 15秒无操作自动关闭背光
#define CANCELMICPHONEPRIORITYCOUNTER	10	// 话筒无信号10秒后恢复音源		

void dspWelcome(void);
void menu_LCMOn(void);
void menu_Init(void);
void menu_Start(void);
void menu_KeyDo(unsigned char KeyCode, unsigned char keyMask);
void menu_UDiskInsert(void);
void menu_OnTimer(void);
void menu_Repaint(void);
void menu_LCMOn(void);
void menu_TimerQueueInit(void);
#endif
