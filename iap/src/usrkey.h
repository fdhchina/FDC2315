#ifndef USRKEY_H
#define USRKEY_H

//�������ļ�ֵ
#define MAX_KEY_NO		16		//20������

//KEY-������־λ��Ϊ�˽�ʡ�ڴ棬ʹ��λ��ʽ
#define KEY_M_PUSH 			1				//������Ч
#define KEY_M_BCK_TIME 		(1<<1)			//������ʼ���������˱�־��û��������Ч
#define KEY_M_RELEASE		(1<<2)			//�����ͷ�
#define KEY_M_BPK_TIME		(1<<3)			//������־

//KEY ����������
#define KEY_BCK_PUSH_TIME		50				//������������1000HZ�ģ���0.05���������
//KEY����������
#define KEY_PBK_BEGIN_TIME		1500				// 1.5���ʼ����
#define KEY_PBK_POLLING_TIME	250				//�����󰴼��ٶ�Ϊ1/4��

/**************************************************************
	�������İ����ı�GUI��Ϣ.ע�����еĺ궨�����������EDIT����Ϣ,����Ҫ��Ӳ���ļ�ֵһһ��Ӧ
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

//extern void CountingKey(void);					//����������,��ŵ�1KHZ��ʱ����
//extern u8 ManageKey(u16 key);					//���̹�����
#endif

