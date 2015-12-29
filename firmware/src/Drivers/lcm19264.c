#include "hal.h"
#include "lcm19264.h"
#include "25f.h"
#include "queue.h"

#if (_HAVE_FONTLIB_ ==FONT_CODE)	// 字库放在程序中
#include "ascii12.h"
#include "sthzk12.h"
const u8* const ASCII12= (const u8*)ZK_ASCII12;		// 英文字库地址
const u8* const HZK12	= (const u8*)ZK_HZK12;		// 中文字库地址
#elif (_HAVE_FONTLIB_==FONT_FLASH)	// 字库放在Flash中
const u8* const ASCII12= (const u8*)sacFont_Ascii12;		// 英文字库地址
const u8* const HZK12	= (const u8*)sacFont_Hzk12;		// 中文字库地址
#else	// 字库放在SPI存储器中
#include <string.h>

static u8 ZK_BUF[32];		// 定义字库读取缓冲，最大32个字节
#endif

const u8 MyDot12[][16];
const u8 playerdot[][24];
#define LCM_DELAY  5	//1000

// ========================================================================================
#if(GC_ZR&GC_ZR_GPIO)
// 硬件端口
// 数据方向控制
#define LCM_E_Port		GPIOB
#define LCM_E_Pin		GPIO_Pin_5

#define LCM_RW_Port		GPIOB
#define LCM_RW_Pin		GPIO_Pin_6

#define LCM_RS_Port		GPIOB
#define LCM_RS_Pin		GPIO_Pin_7

#define LCM_CSB_Port	GPIOB
#define LCM_CSB_Pin		GPIO_Pin_8

#define LCM_CSA_Port	GPIOB
#define LCM_CSA_Pin		GPIO_Pin_9

#define LCM_BG_Port		GPIOE
#define LCM_BG_Pin		GPIO_Pin_0

u8 LCM_GET_DATA() {
	u16 datt;
	datt = GPIO_ReadInputData(GPIOD);
	return datt&0xff;
}

void LCM_SET_DATA(u8 dat) {
	u16 datt;
	datt = GPIO_ReadInputData(GPIOD);
	datt = ((datt&~(0xFF)) | ((dat&0xFF)));
	GPIO_Write(GPIOD, datt);
}void lcm_dat_in()
{
	GPIOD->CRL= 0x33333333;
}
void lcm_dat_out()
{
	GPIOD->CRL= 0x44444444;
}
#define LCM_DATA_OUT()	lcm_dat_in()	// 4245为3V3->5V  // 推挽输出50MHZ速度	
#define LCM_DATA_IN()	lcm_dat_out()	// 4245为5V->3V3  // 浮空输入
// ========================================================================================

#define SET_LCM_RS() \
	LCM_RS_Port->BSRR = LCM_RS_Pin
#define CLR_LCM_RS() \
	LCM_RS_Port->BRR = LCM_RS_Pin

#define SET_LCM_RW() \
	LCM_RW_Port->BSRR = LCM_RW_Pin
#define CLR_LCM_RW() \
	LCM_RW_Port->BRR = LCM_RW_Pin

#define SET_LCM_E() \
	LCM_E_Port->BSRR = LCM_E_Pin
#define CLR_LCM_E() \
	LCM_E_Port->BRR = LCM_E_Pin

#define SET_LCM_CSA() \
	LCM_CSA_Port->BSRR = LCM_CSA_Pin
#define CLR_LCM_CSA() \
	LCM_CSA_Port->BRR = LCM_CSA_Pin

#define SET_LCM_CSB() \
	LCM_CSB_Port->BSRR = LCM_CSB_Pin
#define CLR_LCM_CSB() \
	LCM_CSB_Port->BRR = LCM_CSB_Pin

// 背光低电平驱动
#define SETGroudLight() \
	LCM_BG_Port->BSRR = LCM_BG_Pin
#define CLRGroudLight() \
	LCM_BG_Port->BRR = LCM_BG_Pin

#define LCM_WRITE(data) \
	LCM_SET_DATA(data)

#define LCM_READ LCM_GET_DATA
#else
// 硬件端口
// 数据方向控制
#define LCM_DIR_Port		GPIOD		
#define LCM_DIR_Pin		GPIO_Pin_7

#define LCM_E_Port		GPIOD
#define LCM_E_Pin		GPIO_Pin_6

#define LCM_RW_Port		GPIOD
#define LCM_RW_Pin		GPIO_Pin_5

#define LCM_RS_Port		GPIOD
#define LCM_RS_Pin		GPIO_Pin_4

#define LCM_CSB_Port	GPIOD
#define LCM_CSB_Pin		GPIO_Pin_3

#define LCM_CSA_Port	GPIOD
#define LCM_CSA_Pin		GPIO_Pin_2

#define LCM_BG_Port		GPIOD
#define LCM_BG_Pin		GPIO_Pin_1

#define LCM_DATA_OUT()	{GPIOD->CRH= 0x33333333; LCM_DIR_Port->BRR= LCM_DIR_Pin;}	// 4245为3V3->5V  // 推挽输出50MHZ速度	
#define LCM_DATA_IN()	{GPIOD->CRH= 0x44444444; LCM_DIR_Port->BSRR= LCM_DIR_Pin;}	// 4245为5V->3V3  // 浮空输入
	
#define LCM_GET_DATA()		((GPIOD->IDR>>8)&0xff)
#define LCM_SET_DATA(dat)	GPIOD->BSRR=((dat<<8)|((~dat)<<24))	// 这个可用ODR寄存器
// ========================================================================================

#define SET_LCM_RS() \
        LCM_RS_Port->BSRR = LCM_RS_Pin
#define CLR_LCM_RS() \
        LCM_RS_Port->BRR = LCM_RS_Pin

#define SET_LCM_RW() \
        LCM_RW_Port->BSRR = LCM_RW_Pin
#define CLR_LCM_RW() \
        LCM_RW_Port->BRR = LCM_RW_Pin
                
#define SET_LCM_E() \
        LCM_E_Port->BSRR=LCM_E_Pin
#define CLR_LCM_E() \
        LCM_E_Port->BRR=LCM_E_Pin

#define SET_LCM_CSA() \
        LCM_CSA_Port->BSRR=LCM_CSA_Pin
#define CLR_LCM_CSA() \
        LCM_CSA_Port->BRR=LCM_CSA_Pin

#define SET_LCM_CSB() \
        LCM_CSB_Port->BSRR = LCM_CSB_Pin
#define CLR_LCM_CSB() \
        LCM_CSB_Port->BRR = LCM_CSB_Pin

// 背光低电平驱动
#define SETGroudLight() \
        LCM_BG_Port->BSRR = LCM_BG_Pin
#define CLRGroudLight() \
        LCM_BG_Port->BRR = LCM_BG_Pin
		
#define LCM_WRITE(data) \
        LCM_SET_DATA(data)

#define LCM_READ LCM_GET_DATA
#endif
void lcm_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
#if(GC_ZR&GC_ZR_GPIO)
	// D
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOB, ENABLE);

	//PD1-BG, PD2-CSA, PD3-CSB, PD4-RS, PD5-RW, PD6-E, PD7-DIR
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |  GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50M时钟速度
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	//D0-D7口因为使用了IO,所以不在这里配置
#else
	// D
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	//PD1-BG, PD2-CSA, PD3-CSB, PD4-RS, PD5-RW, PD6-E, PD7-DIR
	GPIO_InitStructure.GPIO_Pin = 0x00FE;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50M时钟速度
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	//D8-D15口因为使用了IO,所以不在这里配置
#endif
}


void lcmDelay (int nDelay);
void lcmWriteData (u8 byData);

typedef unsigned char *	PUINT8;
/***********常用操作命令和参数定义***************/
#define	 DispOn		0x3f	/*显示on		*/
#define	 DispOff		0x3e	/*显示off		*/
#define	 DispFirst	0xc0	/*显示起始行定义	*/
#define	 SetX		0x40	/*X定位设定指令（页）	*/
#define	 SetY		0xb8	/*Y定位设定指令（列）	*/
#define	 LcdBusy	0x80	/*LCM忙判断位		*/

/**************显示分区边界位置*****************/
#define	 MODL		0x00	/*左区			*/
#define	 MODM		0x40	/*左区和中区分界	*/
#define	 MODR		0x80	/*中区和右区分界	*/
#define	 LCMLIMIT	0xC0	/*显示区的右边界	*/

// #define BackLight_Ctrl		6		//背光控制

u8	BackLight_Delay=BACKLIGHT_DELAY;		// 背光关闭延时
u8	bDisplayModified=1;		// 显示是否需要更新

typedef struct 
{
	u8 ID;
	u8 Buffer[8][0xC0];
}	LCM_Buffer;

LCM_Buffer   curLCM,oldLCM;

void lcm_UpdateLCMData(void);

void lcm_BackLightCtrl(u8 opened)
{
	if(opened)
		SETGroudLight();
	else
		CLRGroudLight();
}


void wtcom()
{
	u8 i=255;
  	lcmDelay(LCM_DELAY);
	CLR_LCM_RS();		/*CLR	DI		*/
	SET_LCM_RW();		/*SETB	RW		*/
	SET_LCM_E();
	LCM_DATA_IN();	
#if(!(GC_ZR&GC_ZR_GPIO))
	LCM_DIR_Port->BSRR= LCM_DIR_Pin;	// 4245为5V->3V3
#endif
  	lcmDelay(LCM_DELAY);
   	while((LCM_READ() & LcdBusy)&&(i--));
   	CLR_LCM_E();
	LCM_DATA_OUT();	
#if(!(GC_ZR&GC_ZR_GPIO))
	LCM_DIR_Port->BRR= LCM_DIR_Pin;	// 4245为3V3->5V
#endif
}

/********************************************************/
/* 分区操作允许等待,返回时保留分区选择状态		*/
/********************************************************/
void LcdBusyL()
{
	CLR_LCM_CSA();
	CLR_LCM_CSB();
	wtcom();		/* waitting for enable	*/
}

void LcdBusyM()
{
	CLR_LCM_CSA();
	SET_LCM_CSB();
	wtcom();		/* waitting for enable	*/
}

void LcdBusyR()
{
	SET_LCM_CSA();
	CLR_LCM_CSB();
	wtcom();		/* waitting for enable	*/
}


void WrcmdL(u8 X)
{
	lcmDelay (LCM_DELAY);
	LcdBusyL();		/*确定分区，返回时保留分区状态不变*/
	CLR_LCM_RS();			/*命令操作	*/
	CLR_LCM_RW();			/*写输出  	*/
  	lcmDelay(LCM_DELAY);
	SET_LCM_E();
	LCM_WRITE(X);			/*数据输出到数据口 */
	lcmDelay (LCM_DELAY);
	CLR_LCM_E();	/*读入到LCM*/
}

/********************************/
/* 命令输出到中区控制口		*/
/********************************/

void WrcmdM(u8 X)
{
	lcmDelay (LCM_DELAY);
	LcdBusyM();		/*确定分区，返回时保留分区状态不变*/
	CLR_LCM_RS();			/*命令操作	*/
	CLR_LCM_RW();			/*写输出  	*/
  	lcmDelay(LCM_DELAY);
	SET_LCM_E();
	LCM_WRITE(X);			/*命令输出到数据口 */
	lcmDelay (LCM_DELAY);
	CLR_LCM_E();	/*读入到LCM*/
}

/********************************/
/* 命令输出到右区控制口		*/
/********************************/

void WrcmdR(u8 X)
{
	lcmDelay (LCM_DELAY);
	LcdBusyR();	/*确定分区，返回时保留分区状态不变	*/
	CLR_LCM_RS();			/*命令操作	*/
	CLR_LCM_RW();			/*写输出  	*/
  	lcmDelay(LCM_DELAY);
	SET_LCM_E();
	LCM_WRITE(X);			/*命令输出到数据口 */
	lcmDelay (LCM_DELAY);
	CLR_LCM_E();	/*读入到LCM*/     
}


/********************************************************/
/*根据设定的坐标数据，定位LCM上的下一个操作单元位置	*/
/********************************************************/
void Locatexy(u8 Ax, u8 Ay)
{
	u8  x,y;

	switch (Ax&0xc0)		/*  col.and.0xC0	*/
	{			/*条件分支执行		*/
		case 0:		{LcdBusyL();break;}	/*左区	*/
		case 0x40:	{LcdBusyM();break;}	/*中区	*/
		case 0x80:	{LcdBusyR();break;}	/*右区	*/
	}
	x = (Ax&0x3F)|SetX;		/*  col.and.0x3f.or.setx	*/
	y = (Ay&0x07)|SetY;		/*  row.and.0x07.or.sety	*/
	wtcom();		/*  waitting for enable		*/
	CLR_LCM_RS();			/*CLR	DI	*/
	CLR_LCM_RW();			/*CLR	RW	*/
  	lcmDelay(LCM_DELAY);
	SET_LCM_E();
	LCM_WRITE(y);			/*MOV	P0,Y	*/
	lcmDelay (LCM_DELAY);
	CLR_LCM_E();

	wtcom();		/*  waitting for enable		*/
	CLR_LCM_RS();			/*CLR	DI	*/
	CLR_LCM_RW();			/*CLR	RW	*/
  	lcmDelay(LCM_DELAY);
	SET_LCM_E();
	LCM_WRITE(x);			/*MOV	P0,X	*/
	lcmDelay (LCM_DELAY);
	CLR_LCM_E();
}

        
void lcmWriteData (u8 byData)
{

	wtcom();
	SET_LCM_RS();	/*数据输出*/
	CLR_LCM_RW();	/*写输出  */
  	lcmDelay(LCM_DELAY);
	SET_LCM_E();	/*读入到LCM*/
	LCM_WRITE(byData);	/*数据输出到数据口 */
  	lcmDelay (LCM_DELAY);
	CLR_LCM_E();
	lcmDelay (LCM_DELAY);
}

void lcmDelay (int nDelay)
{
  int i;
  for (i = 0; i < nDelay; ++ i);
}


void fill_lcm_buffer(LCM_Buffer *data,u8 value)
{
	u8 i,j;	
	for(j=0;j<8;j++)
		for(i=0;i<LCMLIMIT;i++)
			data->Buffer[j][i]=value;
}
/****************************************/
/*	清除显示Buffer，全屏幕清零		*/
/****************************************/
void lcm_Clr( void ) 
{
	fill_lcm_buffer(&curLCM,0x0);
	bDisplayModified=1;
}


u8 Rddata(u8 AX,u8 AY)
{
	return(curLCM.Buffer[AY>>3][AX]);
}


/****************************************/
/*	清除row，从col列开始,共清length列	*/
/****************************************/
void lcm_ClrLine(u8 col,u8 row,u8 dx,u8 dy,u8 keep)
{
	u8 x,y,kk;

	dx= col+dx;
	if(dx>LCMLIMIT)	dx=LCMLIMIT;
	dy= row+dy;
	if(dy>8) dy=8;
	for(y=row;y<dy;y++)
		for(x=col;x<dx;x++)
		{
			if(y ==dy-1)
				kk=Rddata(x, (y<<3)) & keep;
			else
				kk=0;
			curLCM.Buffer[y][x]=kk;
		}
	bDisplayModified=1;
}

/****************************************/
/*	画点				*/
/****************************************/
void lcm_Point(u8 AX,u8 AY)
{
	u8	x,y,cu8;
	cu8=Rddata(AX,AY);
	y=AY&0x07;		/*字节内位置计算	*/
	x=1<<y;			/*移入所画点		*/
	curLCM.Buffer[AY>>3][AX]= cu8|x;	/*画上屏幕		*/

	bDisplayModified=1;
}

/*void LineH(u8 xs,u8 xe,u8 ye)
{
	u8 ii;
	for(ii=xs;ii<xe;ii++)
	{
		point(ii,ye);
	}
}

void LineV(u8 xs,u8 ys,u8 ye)
{
	u8 ii;
	if(ys%8!=0)
		for(ii=ys;ii<ys+ys%8;ii++)
			point(xs,ii);
	
	if(ye%8!=0)
		for(ii=ye-ye%8;ii<ye;ii++)
			point(xs,ii);
	for(ii=ys / 8;ii<ye/8;ii++)
	{
		Locatexy(xs,ii);
		lcmWriteData(0xFF);
	}
	
}*/
/************************************************/
/*画线。任意方向的斜线，不支持垂直的或水平线	*/
/************************************************/

void lcm_Linexy(u8 x0,u8 y0,u8 xt,u8 yt)
{
	register u8 t;
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy;

	delta_x=xt-x0;				/*计算坐标增量	*/
	delta_y=yt-y0;
	if(delta_x>0) incx=1;			/*设置单步方向	*/
	else if( delta_x==0 ) incx=0;		/*垂直线	*/
		else {incx=-1;delta_x=-delta_x;}

	if(delta_y>0) incy=1;
	else if( delta_y==0 ) incy=0;		/*水平线	*/
		else {incy=-1;delta_y=-delta_y;}

	if( delta_x > delta_y )	distance=delta_x; /*选取基本增量坐标轴*/
	else distance=delta_y;

	for( t=0;t <= distance+1; t++ )	{	/*画线输出	*/
		lcm_Point(x0,y0);			/*画点		*/
		xerr +=	delta_x	;
		yerr +=	delta_y	;
		
		if( xerr > distance ) {
			xerr-=distance;
			x0+=incx;
		}
		if( yerr > distance ) {
			yerr-=distance;
			y0+=incy;
		}
	}

	bDisplayModified=1;
}

/***************************************/
/*				图片显示			   */	
/*	入口: 坐标	图形点阵 图片起始地址  */
/***************************************/
void lcm_DrawBmp(u8 col,u8 row,u8 X,u8 Y,const u8 *zk,u8 instead)
{
	u8 i,j,n,m,cc,kk;	

	n=(Y+7) >> 3; 

	m= Y % 8;
	if(m>0)
	{
		kk=0;
		for(i=0; i<m; i++)
			kk= (kk<<1)	+1;
	} else
		kk=0xFF;
	for(i=0;i<n;i++)
	{
		for(j=0;j<X;j++)
		{
			cc=*zk;	
			if (instead)
				cc=~cc;
			if(n-1==i)
			{
				cc=cc & kk;
				m=Rddata(col+j,row);
				cc=(m&(0xFF-kk))|cc;
			}
			curLCM.Buffer[row>>3][col+j]=cc;
			zk++;
		}
		row+=8;
	}
	
}

/****************************************/
/*    全角播放器符号点阵码数据输出		*/
/****************************************/
void lcm_PutPlayerdot(u8 AX,u8 AY,u8 Order,u8 instead)
{
	lcm_DrawBmp(AX,AY,12,12,playerdot[Order],instead);
	bDisplayModified=1;
}						/*整个字符输出结束	*/

/****************************************/
/*    半角选择符号点阵码数据输出		*/
/****************************************/
void lcm_MyPutedot(u8 AX,u8 AY,u8 Order,u8 instead)
{
	//U16 x;			//偏移量，字符量少的可以定义为u8	
	//x=Order * 0x10;		//半角字符，每个字符12个字节	

	lcm_DrawBmp(AX,AY,8,12,MyDot12[Order],instead);
	bDisplayModified=1;
}						/*整个字符输出结束	*/

/****************************************/
/*   半角字符点阵码数据输出		*/
/****************************************/
void lcm_Putedot(u8 AX,u8 AY,u8 Order,u8 instead)
{
	u16 x;			//偏移量，字符量少的可以定义为u8	
	x=Order * 12;		//半角字符，每个字符12个字节	
#if _HAVE_FONTLIB_
	lcm_DrawBmp(AX,AY,6,12,ASCII12+x,instead);
#else
	sst25_Read(sacFont_Ascii12+x, ZK_BUF, 12);
	lcm_DrawBmp(AX,AY,6,12, ZK_BUF, instead);
#endif
}						/*整个字符输出结束	*/

/****************************************/
/*  全角字符点阵码数据输出		*/
/****************************************/
char ZK_BUF[32];
void lcm_Putcdot(u8 AX,u8 AY,u8 Order,u8 Order1,u8 instead)
{
	u32 x;				//偏移量，字符量少的可以定义为u8	
	x= ((unsigned long)Order*94 + Order1)*24;		/*全角字符，每个字符24字节	*/
	
#if _HAVE_FONTLIB_
	lcm_DrawBmp(AX,AY,12,12,HZK12+x,instead);
	
	
#else
	sst25_Read(sacFont_Hzk12+x, ZK_BUF, 24);
	lcm_DrawBmp(AX, AY, 12, 12, ZK_BUF, instead);
#endif
}						/*整个字符输出结束	*/


/****************************************/
/*	一个字串的输出			*/
/****************************************/
u8 lcm_Putstr(u8 AX,u8 AY,const char *puts1,u8 len,u8 instead)
{
	u8 j;
	if(len==0) len=0xFF;
	j=0;
	while((puts1[j]!=0)&&(j<len))
	{
		if(((u8)puts1[j]>0xA0)&&(j+1<len)&&((u8)puts1[j+1]>0xA0))
		{
			lcm_Putcdot(AX,AY,puts1[j]-0xA1,puts1[j+1]-0xA1,instead); /*只保留低7位*/
//	Putcdot(AX,AY,X&0x7f,instead); /*只保留低7位*/
			AX+=12;
			if(AX>191){
				AX=0;
				AY+=16;
			}
			j+=2;
		}else
		{
			if(puts1[j]<0x7F)
				lcm_Putedot(AX,AY,puts1[j]-0x20,instead);		/*ascii码表从0x20开始*/
			else
				lcm_Putedot(AX,AY,' '-0x20,instead);		/*ascii码表从0x20开始*/
			AX+=6;
			if(AX>191){
				AX=0;
				AY+=16;
			}
			j++;
		}
	}
	bDisplayModified=1;
	return(j);
}

// *****************************************************************************
// 滚动显示字符串
// *****************************************************************************
u8 lcm_RollShowStr(rollstr_p str)
{
#if 1		// 按字符滚动显示
	u8 i, len, len1, m;
	char s[255], s1[40], *pstr;

  if(!(*str->str)) return TRUE;
	memset(s,0,255);
	memset(s1,0,40);
	pstr=str->str;
	len= strlen(pstr);
//	if  (str->pos==0)  
	{
		if ( (pstr[str->pos]>0xA0)&&(str->pos+1<len)&&(pstr[str->pos+1]>0xA0) )
			str->pos++;
		str->pos++;
	}

	if(str->pos<=str->showlen-1)
	{
		for( i=0;i< str->showlen-str->pos;i++)
			s[i]=' ';
    	strncat(s,pstr, str->pos);
	}else
	{
		pstr+=str->pos+1-str->showlen;
		strncpy(s, pstr, str->showlen); // 取得本次需要显示的字符
		strncpy(s1,str->str,str->pos+1-str->showlen);
		len1= strlen(s1);
		m= 0;
		for( i=0;i<len1;i++)	 // 取得字符串中非ascii字符的个数
			if (s1[i]>0xA0) m++;
		if ((m &1) != 0)
		{
			s[0]= ' ';
		}
	}

	len1=strlen(s);
	m= 0;
	for(i=0;i<len1;i++)	// 取得字符串中非ascii字符的个数
		if (s[i]>0xa0) m++;
	if ((m & 0x1) !=0)
	{
		s[len1-1]= ' ';
	}

	lcm_Putstr(str->col,str->row,s,str->showlen,str->instead);
	if(str->pos>=len-1)
	    str->pos=0;
#else // 平滑滚动显示
	
#endif
	return TRUE;
}

void lcm_DisplayOpen(u8 opened)
{
	WrcmdL(opened);
	WrcmdM(opened);
	WrcmdR(opened);
}

void lcm_Init ()
{
	u8	cu8;
	lcm_Configuration();
	
	lcm_BackLightCtrl(0);
	lcm_DisplayOpen(DispOff);

	cu8 = DispFirst;	//+62;	/*定义显示起始行为零	*/
	WrcmdL(cu8);
	WrcmdM(cu8);
	WrcmdR(cu8);

	fill_lcm_buffer(&oldLCM,0xFF);
	lcm_Clr();		/*清屏		*/
	//Locatexy(0,0);
	lcm_UpdateLCMData();
	lcm_DisplayOpen(DispOn);
	lcm_BackLightCtrl(1);
	
}

void lcm_UpdateLCMData()
{
#if 0
	u8 i,j;

	WrcmdL(DispFirst);
	WrcmdM(DispFirst);
	WrcmdR(DispFirst);
	
	if(	bDisplayModified)
	{
		for(j=0;j<8;j++)
		{
			for(i=0;i<0xC0;i++)
				//if(oldLCM.Buffer[j][i]!=curLCM.Buffer[j][i])
				{
					Locatexy(i, j);
					lcmWriteData(curLCM.Buffer[j][i]);
					//oldLCM.Buffer[j][i]=curLCM.Buffer[j][i];
				}
		}
		bDisplayModified=0;
	}
#else
	u16 i, j, n;
	u8	*buf;
	
	if(	bDisplayModified)
	{
		for(j=0; j<3; j++)
		{
			n= j<<6;
			for(i=0;i<512;i++)
			{
				if((i&0x3F)==0)
				{
					buf= &curLCM.Buffer[i>>6][n];
					Locatexy(n, i>>6);
				}
				lcmWriteData(*buf++);
			}
		}
		bDisplayModified=0;
	}
#endif
}

u8 const ASCII56[][6];

/****************************************/
void lcm_PuteDot5x6(u8 col,u8 row,u8 Order,u8 instead)
	{
	int x1;			/*偏移量，字符量少的可以定义为u8	*/
	x1=(Order-0x2F);		/*半角字符，每个字符16字节	*/

	lcm_DrawBmp(col, row, 6, 7, ASCII56[x1], instead);

}						/*整个字符输出结束	*/

void lcm_Putstr5x6(u8 col,u8 layer, const char *ptr,u8 instead)
{
	u8 i=0;
	u8 ucol;
	ucol = col;   
	while(ptr[i]!= 0)
	{
		lcm_PuteDot5x6(ucol,layer,ptr[i],instead);
		ucol+=6;
		i++;                      //ASCII码的处理
	}
}	


//*********************星期数据字库*****************
//每个字符字节数：16
//宽度×高度(象素):8×12
//**************************************************
u8 const MyDot12[][16] = {
{// 日 //
0x00,0xFC,0x24,0x24,0x24,0x24,0xFC,0x00,
0x00,0x03,0x01,0x01,0x01,0x01,0x03,0x00},

{// 一 //
0x00,0x40,0x40,0x40,0x40,0x40,0x60,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

{// 二 //
0x00,0x10,0x10,0x10,0x10,0x18,0x00,0x00,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00},

{// 三 //
0x00,0x10,0x50,0x50,0x50,0x50,0x58,0x00,
0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x00},

{// 四 //
0xFC,0x44,0x3C,0x04,0x7C,0x44,0xFC,0x00,
0x03,0x01,0x01,0x01,0x01,0x01,0x03,0x00},

{// 五 //
0x00,0x24,0x24,0xF4,0x24,0xE4,0x04,0x00,
0x02,0x02,0x02,0x03,0x02,0x03,0x02,0x00},

{// 六 //
0x00,0x10,0xD0,0x14,0x14,0xD0,0x10,0x00,
0x02,0x02,0x01,0x00,0x00,0x01,0x03,0x00},

{// [x] //
0x00,0xFC,0x14,0xA0,0x40,0xA0,0x14,0xFC,
0x00,0x07,0x05,0x00,0x00,0x00,0x05,0x07},

{// [ ] //
0x00,0xFC,0x04,0x00,0x00,0x00,0x04,0xFC,
0x00,0x07,0x04,0x00,0x00,0x00,0x04,0x07}
};


u8 const playerdot[][24] = {
/* 播放   CAFA1 */
{0x00,0x00,0xFE,0xFE,0xFC,0xFC,0xF8,0xF8,0x70,0x70,0x20,0x00,0x00,0x00,0x03,0x03,
0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00},

/* 暂停   CAFA2 */
{0x00,0x00,0xFE,0xFE,0xFE,0x00,0x00,0x00,0xFE,0xFE,0xFE,0x00,0x00,0x00,0x03,0x03,
0x03,0x00,0x00,0x00,0x03,0x03,0x03,0x00},

/* 停止   CAFA3 */
{0x00,0x00,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0x00,0x00,0x00,0x03,0x03,
0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x00},

/* 上曲   CAFA4 */
{0x00,0x00,0xFE,0xFE,0x00,0x20,0x70,0xF8,0xFC,0xFE,0xFE,0x00,0x00,0x00,0x03,0x03,
0x00,0x00,0x00,0x00,0x01,0x03,0x03,0x00},

/* 下曲   CAFA5 */
{0x00,0x00,0xFE,0xFE,0xFC,0xF8,0x70,0x20,0x00,0xFE,0xFE,0x00,0x00,0x00,0x03,0x03,
0x01,0x00,0x00,0x00,0x00,0x03,0x03,0x00},

/* 弹出   CAFA6 */
{0x00,0x00,0x20,0x30,0x38,0x3C,0x3E,0x3C,0x38,0x30,0x20,0x00,0x00,0x00,0x03,0x03,
0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x00},

/* 录音   CAFA7 */
{0x00,0x00,0x70,0xFC,0xFC,0xFE,0xFE,0xFE,0xFC,0xFC,0x70,0x00,0x00,0x00,0x00,0x01,
0x01,0x03,0x03,0x03,0x01,0x01,0x00,0x00},

/* 静音   CAFA8 */
{0x00,0x00,0xF4,0x98,0xF0,0xA8,0x44,0xA2,0xFE,0x08,0x04,0x00,0x00,0x00,0x04,0x02,
0x01,0x01,0x02,0x04,0x07,0x02,0x04,0x00},

/* 立体声   CAFA9 */
{0x00,0xFE,0x04,0xF8,0x08,0xF8,0xF8,0x08,0xF8,0x04,0xFE,0x00,0x00,0x07,0x02,0x01,
0x01,0x01,0x01,0x01,0x01,0x02,0x07,0x00},

/* 单声道   CAFAA */
{0x00,0xF8,0x08,0xF8,0xF0,0x08,0x04,0xFE,0x00,0x24,0x22,0x00,0x00,0x01,0x01,0x01,
0x00,0x01,0x02,0x07,0x00,0x02,0x04,0x00},

/* 信号无   CAFAB */
{0x0C,0x14,0xE4,0x14,0x0C,0x20,0x40,0x80,0x40,0x20,0x00,0x00,0x00,0x00,0x03,0x00,
0x00,0x02,0x01,0x00,0x01,0x02,0x00,0x00},

/* 信号有   CAFAC */
{0x0C,0x14,0xE4,0x14,0x0C,0x00,0xC0,0x00,0xF0,0x00,0xFC,0x00,0x00,0x00,0x03,0x00,
0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00},

/* 上下箭头  CAFAD */
{0x00,0xFF,0x91,0x99,0x9D,0x9F,0x9F,0x9D,0x99,0x91,0xFF,0x00,0x00,0x0F,0x08,0x09,
0x0B,0x0F,0x0F,0x0B,0x09,0x08,0x0F,0x00}

};


/*ASCII字体,大小6X8,上到下D0~D7,左到右*/
u8 const ASCII56[][6]={
{0x00,0x00,0x00,0x00,0x00,0x00},/*SPACE*/
{0x00,0x3e,0x51,0x49,0x45,0x3e},/*0*/
{0x00,0x00,0x42,0x7f,0x40,0x40},/*1*/
{0x00,0x62,0x51,0x51,0x49,0x46},/*2*/
{0x00,0x21,0x41,0x45,0x4b,0x31},/*3*/
{0x00,0x18,0x14,0x12,0x7f,0x10},/*4*/
{0x00,0x27,0x45,0x45,0x45,0x39},/*5*/
{0x00,0x3c,0x4a,0x49,0x49,0x30},/*6*/
{0x00,0x01,0x71,0x09,0x05,0x03},/*7*/
{0x00,0x36,0x49,0x49,0x49,0x36},/*8*/
{0x00,0x06,0x49,0x49,0x29,0x1e},/*9*/
                               };

u8 const dot32[] = {

0x00,0x00,0x00,0xC0,0xE0,0x60,0x30,0x30,0x30,0x30,0x60,0xE0,0xC0,0x00,0x00,0x00,
0x00,0xFC,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFC,0x00,
0x00,0x1F,0x7F,0xF0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xF0,0x7F,0x1F,0x00,
0x00,0x00,0x00,0x01,0x03,0x03,0x06,0x06,0x06,0x06,0x03,0x03,0x01,0x00,0x00,0x00,// "0"0

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x03,0x03,0x03,0x03,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x07,0x00,0x00,0x00,0x00,0x00,// "1"1

0x00,0x00,0x80,0xC0,0xE0,0x70,0x30,0x30,0x30,0x30,0x30,0x60,0xE0,0xC0,0x00,0x00,
0x00,0x00,0x0F,0x07,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0xC0,0x60,0x3F,0x1F,0x00,
0x00,0x80,0xE0,0x70,0x18,0x0C,0x06,0x02,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x07,0x07,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x00,// "2"2

0x00,0x00,0x00,0xC0,0xE0,0x60,0x30,0x30,0x30,0x30,0x70,0xE0,0xC0,0x80,0x00,0x00,
0x00,0x00,0x07,0x07,0x00,0x00,0xC0,0xC0,0xC0,0xC0,0xE0,0xF0,0xBF,0x0F,0x00,0x00,
0x00,0x78,0xF8,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xC3,0xFF,0x7C,0x00,
0x00,0x00,0x00,0x01,0x03,0x06,0x06,0x06,0x06,0x06,0x07,0x03,0x03,0x01,0x00,0x00,// "3"3

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xE0,0xF0,0xF0,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xC0,0xF0,0x3C,0x0E,0x07,0x01,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x38,0x3E,0x37,0x31,0x30,0x30,0x30,0x30,0x30,0xFF,0xFF,0x30,0x30,0x30,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x07,0x00,0x00,0x00,0x00,// "4"4

0x00,0x00,0x00,0xF0,0xF0,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x00,0x00,
0x00,0x00,0xFC,0xFF,0xE3,0x60,0x30,0x30,0x30,0x30,0x70,0x60,0xE0,0x80,0x00,0x00,
0x00,0xE0,0xE1,0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC1,0xFF,0x7F,0x00,
0x00,0x00,0x01,0x03,0x03,0x06,0x06,0x06,0x06,0x06,0x07,0x03,0x01,0x00,0x00,0x00,// "5"5

0x00,0x00,0x00,0x80,0xC0,0x60,0x70,0x30,0x30,0x30,0x30,0x60,0xE0,0xC0,0x00,0x00,
0x00,0xF0,0xFE,0xC7,0x61,0x60,0x30,0x30,0x30,0x30,0x70,0x60,0xE3,0xC3,0x00,0x00,
0x00,0x3F,0xFF,0xC1,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC1,0xFF,0x3F,0x00,
0x00,0x00,0x00,0x01,0x03,0x07,0x06,0x06,0x06,0x06,0x07,0x03,0x01,0x00,0x00,0x00,// "6"6

0x00,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0xB0,0xF0,0x70,0x30,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xF0,0x38,0x0E,0x03,0x01,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x80,0xF0,0x7E,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x07,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,// "7"7

0x00,0x00,0x80,0xE0,0xE0,0x70,0x30,0x30,0x30,0x30,0x70,0xE0,0xC0,0x80,0x00,0x00,
0x00,0x00,0x0F,0x9F,0xF8,0xF0,0x60,0x60,0x60,0x60,0xF0,0xB8,0xBF,0x0F,0x00,0x00,
0x00,0x7C,0xFF,0xC3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC3,0xFF,0x7C,0x00,
0x00,0x00,0x01,0x03,0x03,0x07,0x06,0x06,0x06,0x06,0x07,0x03,0x03,0x01,0x00,0x00,// "8"8

0x00,0x00,0x80,0xC0,0x60,0x70,0x30,0x30,0x30,0x30,0x70,0x60,0xC0,0x80,0x00,0x00,
0x00,0x7E,0xFF,0xC1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC1,0xFF,0xFE,0x00,
0x00,0x00,0xE1,0xE3,0x03,0x06,0x06,0x06,0x06,0x06,0x03,0x83,0xF1,0x7F,0x0F,0x00,
0x00,0x00,0x01,0x03,0x03,0x06,0x06,0x06,0x06,0x06,0x03,0x03,0x01,0x00,0x00,0x00,// "9"9

0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x07,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,// ":"10

0x00,0xF0,0xF0,0xF0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xF0,0xF0,0xF0,0x00,
0x00,0xFF,0xFF,0x01,0x7F,0xF8,0x00,0x00,0x00,0x00,0xF8,0x7F,0x03,0xFF,0xFF,0x00,
0x00,0xFF,0xFF,0x00,0x00,0x0F,0xFF,0xE0,0xC0,0xFF,0x0F,0x00,0x00,0xFF,0xFF,0x00,
0x00,0x07,0x07,0x00,0x00,0x00,0x01,0x07,0x07,0x01,0x00,0x00,0x00,0x07,0x07,0x00,// "M"11

0x00,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0x00,
0x00,0xFF,0xFF,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFF,0xFF,0x00,
0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,
0x00,0x07,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x07,0x00,// "H"12

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x8C,0xCC,0x7C,0x3C,0x1C,0x00,0x00,
0x00,0x00,0x00,0xC0,0xE0,0x70,0x38,0x0E,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00,
0x00,0x02,0x07,0x07,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x00,// "z"13

0x00,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0x60,0x30,0x00,0x00,
0x00,0xFF,0xFF,0x00,0xC0,0x60,0x30,0xF8,0xCC,0x07,0x01,0x00,0x00,0x00,0x00,0x00,
0x00,0xFF,0xFF,0x01,0x00,0x00,0x00,0x00,0x03,0x0F,0x3C,0x70,0xE0,0x80,0x00,0x00,
0x00,0x07,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x06,0x00,// "K"14

0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0xF0,0xF0,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x80,0xF0,0x7F,0x0F,0x01,0x01,0x0F,0x7F,0xF0,0x80,0x00,0x00,0x00,
0x00,0xC0,0xF8,0x3F,0x0F,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0F,0x7F,0xF8,0xC0,0x00,
0x04,0x07,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x07,0x04,// "A"15

0x00,0x00,0xE0,0xF0,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x00,
0x00,0x00,0xFF,0xFF,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0x00,0x00,
0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x07,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,// "F"16

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x0F,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,// "."17

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 // " "18

};

unsigned char getdot24index(unsigned char order)
{
	const unsigned char dot24_tab[]={':','M','H','z','K','A','F','.'};
	u8 i;

	if(	(order>='0')&&(order<='9')	) 
		return order-'0';
	else
	{
		for(i=0;i<8;i++)
			if(order==dot24_tab[i])
				return i+10;
	}
	return 18;
}

/****************************************/
/*   半角字符点阵码数据输出		*/
/****************************************/
void Putedot32(unsigned char AX,unsigned char AY,unsigned char Order,unsigned char instead)
{
	lcm_DrawBmp(AX,AY,16,30,&dot32[Order<<6],instead);
}						/*整个字符输出结束	*/

/****************************************/
/*	一个字串的输出			*/
/****************************************/
unsigned char lcm_Putstr32(unsigned char AX,unsigned char AY,const char *puts1,unsigned char len,unsigned char instead)
{
	unsigned char j;
	j=0;
	while((puts1[j]!=0)&&(j<len))
	{
		Putedot32(AX,AY, getdot24index(puts1[j]),instead);		
		AX+=16;
		if(AX>191){
			AX=0;
			AY+=32;
		}
		j++;
	}
	bDisplayModified=1;
	return(j);
}



