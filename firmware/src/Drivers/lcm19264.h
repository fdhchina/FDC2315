#ifndef LCM19264_H_
#define LCM2964_H_

#define BACKLIGHT_DELAY				30	//����ر���ʱ����

// ========================================================================================
// �����ַ��ֿⳣ������
// С��ģʽ��������ǰ��λ���ں�
#define SYMBOL_WIR_NO_12DOT         0xE3A2        // �������ź�
#define SYMBOL_WIR_LOW_12DOT        0xE3A2        // ����С�ź�
#define SYMBOL_WIR_HIGH_12DOT       0xEFA2        // ����С�ź�

#define SYMBOL_PLAY_12DOT           0xABA2        // ����
#define SYMBOL_PAUSE_12DOT          0xABA3        // ��ͣ
#define SYMBOL_STOP_12DOT           0xABA4        // ֹͣ
#define SYMBOL_PREV_12DOT           0xABA5        // ��һ��
#define SYMBOL_NEXT_12DOT           0xABA6        // ��һ��
#define SYMBOL_UPDOWN_12DOT         0xABA7        // �ϡ���
#define SYMBOL_SELECTED_12DOT       0xABA8        // ѡ�з���

#define SYMBOL_MICPHONE_12DOT       0xF4A4        // ��Ͳ����
// ========================================================================================
extern void lcm_Init (void);
extern void lcm_DisplayOpen(u8 opened);
extern void lcm_Clr( void );
extern void lcm_ClrLine(u8 col,u8 row,u8 dx,u8 dy,u8 keep);
extern void lcm_Point(u8 AX,u8 AY);
extern void lcm_Linexy(u8 x0,u8 y0,u8 xt,u8 yt);
extern void lcm_DrawBmp(u8 col,u8 row,u8 X,u8 Y,const u8 *zk,u8 instead);
extern void lcm_MyPutedot(u8 AX,u8 AY,u8 Order,u8 instead);
//extern void MyPutedot(u8 AX,u8 AY,u8 Order,u8 instead);
extern void lcm_Putcdot(u8 AX,u8 AY,u8 Order,u8 Order1,u8 instead);
extern u8 lcm_Putstr(u8 AX,u8 AY,const char *puts1,u8 i,u8 instead);
extern void lcm_Putstr5x6(u8 col,u8 layer,const char *ptr,u8 instead);
extern void lcm_PutPlayerdot(u8 AX,u8 AY,u8 Order,u8 instead);
unsigned char lcm_Putstr32(unsigned char AX,unsigned char AY,const char *puts1,unsigned char len,unsigned char instead);
extern void lcm_UpdateLCMData(void);
extern 	u8 	BackLight_state;		// ����״̬
extern 	u8	BackLight_Delay;
extern void lcm_BackLightCtrl(u8 opened);
#if 1
// *****************************************************************************
// ������ʾ�ַ���
// *****************************************************************************
// ���ݽṹ
typedef struct _rollstr
{
	char *str;			// ��ʾ�ַ���ָ��
	u8 col,row;		// ��ʾλ��
	u8 showlen;		// ������ʾ���ַ�����
	u8 pos;			// ��ʼ�ַ�λ��
	u8 delay;			// ��ʱ��ʼ��ʾ������
	BOOL instead;	// �Ƿ���
}rollstr_t,*rollstr_p;

u8 lcm_RollShowStr(rollstr_p str);
#endif

#endif

