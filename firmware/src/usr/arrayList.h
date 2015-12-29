#ifndef _ARRAYLIST_H
#define _ARRAYLIST_H

#define MYLIST_MAX_SIZE	160		// ���Ԫ�ظ���

typedef void *Element;    //����List���Ԫ������
typedef int  (*CmpFun)(Element, Element);//�ȽϺ��� 

typedef struct _ArrayList{ //����ArrayList 
      Element data[MYLIST_MAX_SIZE]; //List��Ԫ��    
      u32   size; //		ListԪ������ 
      u32   index;//	��ǰԪ������ 
      CmpFun   cmpFun;//�ȽϺ���ָ�� 
}ArrayList;

void alInit(ArrayList *arr, CmpFun fun);
int alAddElement(ArrayList *arr, Element elm);
int alSetElement(ArrayList *arr, Element elm, int i);
bool alIsEmpty(ArrayList *arr);
Element *alGetElement(ArrayList *arr, int index);
int alIndexof(ArrayList *arr, Element elm);
int  alRemoveIndex(ArrayList *arr, int index);
int  alRemoveElement(ArrayList *arr, Element elm);
void alSort(ArrayList *arr);
void alClear(ArrayList *arr);

#endif
