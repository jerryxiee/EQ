#include "stdio.h"
#include "string.h"
#include "stm32f2xx.h"
#include "b_queue.h" 

/*******************************************************************
** 描述: 初始化队列
** 参数说明:       queue： 要初始化的队列
                   buf  :  队列的缓冲区
                   max_size
** 返回: 实际取到的字节数
********************************************************************/
int b_queue_init(struct _b_queue *queue, unsigned char *buf, unsigned long max_size)
{
	if(queue != NULL)
	{
		memset(queue, 0, sizeof(struct _b_queue));
		queue->buf = buf;
		queue->max_size = max_size;
		return 1;
	}
	else
	{
		return 0;
	}
}

int lock_b_queue(struct _b_queue *queue)
{
	int retv;
	
	if(queue == NULL)
	{
		return 0;
	}

	__ENTER_CRITICAL();
	
	if( queue->lock )
	{
		
		retv = 0;
	}
	else
	{
		queue->lock = 1;

		retv = 1;
	}
	__EXIT_CRITICAL();

	return retv;
}


int unlock_b_queue(struct _b_queue *queue)
{
	queue->lock = 0;
	return 1;
}

/*******************************************************************
** 描述: 字节出队；从字节队列queue中取出cnt个字节，放到buf中
** 参数说明: cnt: 要获取的字节数,当cnt==-1时，表示要取出队列
             中的全部字节         
** 返回: 实际取到的字节数
********************************************************************/
signed long out_b_queue(struct _b_queue *queue, unsigned char *buf, signed long cnt)
{
	signed long retv = 0;
	
	if ( (queue == NULL) || (cnt == 0) )
		return 0;
	
	if( (queue->cnt > 0) )
	{	
		if ( (cnt == -1) || (queue->cnt >= cnt) )
		{
			if(cnt == -1)
				cnt = queue->cnt;
		
			retv = 0;
		
			while(queue->cnt > 0)
			{
				if(retv >= cnt)
					break;

				buf[retv++] = queue->buf[queue->rd_pt++];

				queue->cnt--;
			
				if( queue->rd_pt >= queue->max_size)
					queue->rd_pt = 0;
			}
		}
	}

	return retv;
}


/**********************************************************************************
** 函 数 名：in_b_queue
** 功能描述：入队
** 返    回：入队的字节数
***********************************************************************************/
signed long in_b_queue(struct _b_queue *queue, unsigned char *buf, signed long cnt)
{
	signed long retv = 0;
	
	if ( (queue == NULL) || (cnt <= 0) )
		return 0;
	
	if ( cnt <= (queue->max_size - queue->cnt) )
	{
		retv = 0;
		
		while(retv < cnt)
		{
			if( queue->wr_pt >= queue->max_size)
				queue->wr_pt = 0;
				
			queue->buf[queue->wr_pt++] = buf[retv++];
			
			if(queue->cnt > queue->max_size)
			{
			  	queue->cnt = queue->max_size;
				break;
			}
			queue->cnt++;
			
			

		}	
	}
	
	return retv;
}

//返回队列要出队的一个字节，即rd_pt指向的字节
s8 read_b_queue_fst_byte(struct _b_queue *queue, u8 *_byte)
{
  	if (queue == NULL)
		return 0;
	
	if(queue->cnt == 0)
	  	return 0;

	*_byte = queue->buf[queue->rd_pt];
	
	return 1;
}


s8 is_queue_enmty(struct _b_queue *queue)
{
	if(queue->cnt == 0)
	  	return 1;  
	
	return 0;
}