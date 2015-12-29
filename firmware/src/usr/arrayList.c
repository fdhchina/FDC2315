#include "hal.h"
#include "arrayList.h"

// ͨ���б���
// ����������256��Ԫ��
/*-----------------------------
*
* For    �㷨���ݽṹ
* IDE    DEV-CPP4.9.2
* Auhtor  Czp
* Date   2012/11/4
*------------------------------
*/
//��ֹ�ظ����� 

// ���C++�����,����C������ 
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
*@desc: ��ĩβ���Ԫ��
*@param: arr->���Ǹ�arrlist��� 
*@parm: elm->Ҫ��ӵ�Ԫ��
*@return: ���ڵ���0->Ԫ�����б��� ��λ��
         -1->ʧ�� 
****************************** 
*/
int alAddElement(ArrayList *arr, Element elm)
{
	int nInd;
	if(arr!=NULL)
	{
		//���ʣ��ռ�          
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
*@desc: ��ָ����λ������Ϊelm
*@param: arr->��������б�
*@param: elm->�������Ԫ�� 
*@param: i->�����õ�λ�� 
*@return: ���ڵ���0->�ɹ� 
         -1->ʧ�� 
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
*@desc:�б��Ƿ�Ϊ��
*param:arr->�������б� 
*@return:1->�ǿ� 0->�� 
******************* ****
*/
bool alIsEmpty(ArrayList *arr)
{
	if(arr!=NULL)
		return arr->size?FALSE:TRUE;
	return FALSE;      
} 

/**************************************
*@desc:��ȡָ��λ�õ�Ԫ��
*@param:arr->�����ҵ��б�
*@param:index->���� 
*@return:NULL->����������򷵻�Ԫ��ָ��
*************************************** 
*/
Element *alGetElement(ArrayList *arr, int index)
{
	if((arr!=NULL)&&(index>=0)&&(index<arr->size))
		return arr->data[index];      
	return NULL;
}

/**************************
*@desc:��ȡĳ��Ԫ�ص�λ��
*@param:arr>�б�
*@param:elm->Ԫ�� 
*@param:  fun->�ȽϺ��� 
*@return:-1->�Ҳ��� 
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
*@desc:�Ƴ�ָ����Ԫ��
*@param:arr->���������б�
*@param:index->Ҫ�Ƴ���Ԫ��������
*@return:0->ʧ��1->�ɹ� 
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
*@desc:�Ƴ�ָ����Ԫ��
*@param:arr->���������б�
*@param:elm->Ҫ�Ƴ���Ԫ��
*@param:fun->�ȽϺ��� 
*@return:0->ʧ��1->�ɹ� 
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
   


