#ifndef LCM19264_H_
#define LCM2964_H_

#define BACKLIGHT_DELAY				30	//背光关闭延时秒数

// ========================================================================================
// 特殊字符字库常量定义
// 小端模式，区码在前，位码在后
#define SYMBOL_WIR_NO_12DOT         0xE3A2        // 天线无信号
#define SYMBOL_WIR_LOW_12DOT        0xE3A2        // 天线小信号
#define SYMBOL_WIR_HIGH_12DOT       0xEFA2        // 天线小信号

#define SYMBOL_PLAY_12DOT           0xABA2        // 播放
#define SYMBOL_PAUSE_12DOT          0xABA3        // 暂停
#define SYMBOL_STOP_12DOT           0xABA4        // 停止
#define SYMBOL_PREV_12DOT           0xABA5        // 上一曲
#define SYMBOL_NEXT_12DOT           0xABA6        // 下一曲
#define SYMBOL_UPDOWN_12DOT         0xABA7        // 上、下
#define SYMBOL_SELECTED_12DOT       0xABA8        // 选中方框

#define SYMBOL_MICPHONE_12DOT       0xF4A4        // 话筒符号
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
extern 	u8 	BackLight_state;		// 背光状态
extern 	u8	BackLight_Delay;
extern void lcm_BackLightCtrl(u8 opened);
#if 1
// *****************************************************************************
// 滚动显示字符串
// *****************************************************************************
// 数据结构
typedef struct _rollstr
{
	char *str;			// 显示字符串指针
	u8 col,row;		// 显示位置
	u8 showlen;		// 可以显示的字符总数
	u8 pos;			// 开始字符位置
	u8 delay;			// 延时开始显示的秒数
	BOOL instead;	// 是否反显
}rollstr_t,*rollstr_p;

u8 lcm_RollShowStr(rollstr_p str);
#endif

#endif

