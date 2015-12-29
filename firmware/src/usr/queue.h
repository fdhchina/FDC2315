#ifndef	_QUEUE_H_
#define _QUEUE_H_

typedef struct	_queue_t
{
	void	*next;
	void (*func)();
}queue_t,*queue_p;

queue_p add_queue(queue_p  head);
void remove_queue(queue_p  head,queue_p queue);	
// queue_p alloc_queue(void);
// void	free_queue(queue_p queue);
void queue_do(queue_p head);
void remove_allqueue(queue_p head);



#endif
