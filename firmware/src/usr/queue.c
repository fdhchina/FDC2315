#include <string.h>
#include "queue.h"

typedef struct
{
	unsigned char status;
	queue_t	queue;
}queue_block_t;

#define	QUEUE_MAXNUM			0x5	//最多四项

static queue_block_t _queue_buf[QUEUE_MAXNUM];

queue_p alloc_queue()
{
	int i;
	for (i = 0; i < QUEUE_MAXNUM; i++) 
	{
		if (_queue_buf[i].status == 0) 
		{
			_queue_buf[i].status = 1;
			memset(&_queue_buf[i].queue, 0,sizeof(queue_t));
			return (&_queue_buf[i].queue);
		}
      }
	return 0;
}

void	free_queue(queue_p queue)
{
	int i;
	for (i = 0; i < QUEUE_MAXNUM; i++) 
	{
		if (&_queue_buf[i].queue== queue) 
			memset(&_queue_buf[i], 0,sizeof(queue_block_t));
      }
	queue=NULL;
}
queue_p add_queue(queue_p  head)
{
	queue_p	tmp=head;
	queue_p queue= alloc_queue();
	if(!queue) return 0;
	if(tmp)
	{
		while(tmp->next)
			tmp=tmp->next;
		tmp->next= queue;
	}
	return queue;
}

void remove_queue(queue_p  head,queue_p queue)
{

	queue_p tmp,tmp1=head;
	if((!queue)||(!head))return;
	tmp= queue->next;
	while( (tmp1->next!=queue) &&(tmp1->next) )	
		tmp1=tmp1->next;
	tmp1->next= tmp;
	free_queue(queue);
}

void queue_do(queue_p head)
{
	queue_p tmp=head;
	while(tmp )
	{
		if(tmp->func)
			tmp->func();
		tmp= tmp->next;
	}
}

void remove_allqueue(queue_p head)
{
	if(!head) return;
	while(head->next)
		remove_queue(head, head->next);
}

