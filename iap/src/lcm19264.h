#ifndef LCM19264_H_
#define LCM2964_H_

// ========================================================================================
void lcm_Init (void);
void lcm_Clr( void );
void lcm_BackLightCtrl(u8 opened);
void lcm_DisplayOpen(u8 opened);
void lcm_Putcdot(u8 AX,u8 AY,u8 Order,u8 Order1,u8 instead);
void lcm_Putedot(u8 AX, u8 AY, u8 Order, u8 instead);
void lcm_Putstr(u8 AX, u8 AY, u8 *puts, u8 len, u8 instead);
void lcm_DisplayNoFont(void);

#endif

