#ifndef _ARRAYLIST_H
#define _ARRAYLIST_H

#define MYLIST_MAX_SIZE	160		// 最大元素个数

typedef void *Element;    //定义List里的元素类型
typedef int  (*CmpFun)(Element, Element);//比较函数 

typedef struct _ArrayList{ //定义ArrayList 
      Element data[MYLIST_MAX_SIZE]; //List的元素    
      u32   size; //		List元素总数 
      u32   index;//	当前元素索引 
      CmpFun   cmpFun;//比较函数指针 
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
