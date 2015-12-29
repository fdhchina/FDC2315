#include "hal.h"
#include "arrayList.h"

// 通用列表类
// 可以最大添加256个元素
/*-----------------------------
*
* For    算法数据结构
* IDE    DEV-CPP4.9.2
* Auhtor  Czp
* Date   2012/11/4
*------------------------------
*/
//防止重复导入 

// 如果C++里调用,采用C编译器 
#ifdef __cplusplus 
   extern "C" {
#endif

void alInit(ArrayList *arr, CmpFun fun)
{
	if(arr&&fun)
	{
		arr->size= 0;
		arr->index= 0;
		arr->cmpFun= fun;
	}
}
/****************************
*@desc: 在末尾添加元素
*@param: arr->对那个arrlist添加 
*@parm: elm->要添加的元素
*@return: 大于等于0->元素在列表中 的位置
         -1->失败 
****************************** 
*/
int alAddElement(ArrayList *arr, Element elm)
{
	int nInd;
	if(arr!=NULL)
	{
		//检查剩余空间          
		if(arr->size<MYLIST_MAX_SIZE-1)
		{
			nInd= arr->size;
			arr->data[arr->size++] = elm;
			return nInd;                       
		}         
	}
	return -1;
}
/****************************
*@desc: 把指定的位置设置为elm
*@param: arr->待插入的列表
*@param: elm->待插入的元素 
*@param: i->待设置的位置 
*@return: 大于等于0->成功 
         -1->失败 
*****************************         
*/
int alSetElement(ArrayList *arr, Element elm, int i)
{
	if((arr!=NULL)&&(i>=0)&&(i<MYLIST_MAX_SIZE-1))
	{
		arr->data[i] = elm;
		return i;                          
	}
	return -1;
}

/************************
*@desc:列表是否为空
*param:arr->待检查的列表 
*@return:1->非空 0->空 
******************* ****
*/
bool alIsEmpty(ArrayList *arr)
{
	if(arr!=NULL)
		return arr->size?FALSE:TRUE;
	return FALSE;      
} 

/**************************************
*@desc:获取指定位置的元素
*@param:arr->待查找的列表
*@param:index->索引 
*@return:NULL->索引错误否则返回元素指针
*************************************** 
*/
Element *alGetElement(ArrayList *arr, int index)
{
	if((arr!=NULL)&&(index>=0)&&(index<arr->size))
		return arr->data[index];      
	return NULL;
}

/**************************
*@desc:获取某个元素的位置
*@param:arr>列表
*@param:elm->元素 
*@param:  fun->比较函数 
*@return:-1->找不到 
**************************
*/
int alIndexof(ArrayList *arr, Element elm)
{
	int i;
	if(arr!=NULL)
	{ 
		for(i=0;i<arr->size;i++)
			if(arr->cmpFun(arr->data[i], elm)==0)
				return i; 
	}
	return -1;
}

/************************
*@desc:移除指定的元素
*@param:arr->待操作的列表
*@param:index->要移除的元素索引号
*@return:0->失败1->成功 
**************************
*/
int  alRemoveIndex(ArrayList *arr, int index)
{
	if((arr!=NULL)&&(index>=0))
	{
		if(index<arr->size)
		{
			arr->size--;
			for(;index<arr->size;index++)
			{
				arr->data[index] = arr->data[index+1];                  
			}   
			return 1;
		}
	}
	return 0;
}

/************************
*@desc:移除指定的元素
*@param:arr->待操作的列表
*@param:elm->要移除的元素
*@param:fun->比较函数 
*@return:0->失败1->成功 
**************************
*/
int  alRemoveElement(ArrayList *arr, Element elm)
{
	if(arr!=NULL)
	{
		int i =  alIndexof(arr, elm);
		if(i>=0)
		{
			arr->index--;
			for(;i<arr->index;i++)
			{
				arr->data[i] = arr->data[i+1];                  
			}   
			return 1;
		}                      
	}
	return 0;
}

void alSort(ArrayList *arr)
{
	int i,j;
	Element t;
	if(arr==NULL) return;
	for(i=arr->size-1; i>=0; i--)
		for(j=0; j<i; j++)
		{
			if( arr->cmpFun(arr->data[j], arr->data[j+1])>0)  
			{
				t=arr->data[j+1];
				arr->data[j+1]=arr->data[j];
				arr->data[j]=t;
			}
		}
}

void alClear(ArrayList *arr)
{
	if(arr)
	{
		arr->size= 0;
		arr->index= 0;
	}
}

#ifdef __cplusplus
   }
#endif
   


