#ifndef _MENU_
#define _MENU_

#define SONGNAME_CHARCOUNT	12
#define AUTOCLOSEBACKLIGHTCOUNTER		30		// 15���޲����Զ��رձ���
#define CANCELMICPHONEPRIORITYCOUNTER	10	// ��Ͳ���ź�10���ָ���Դ		

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
