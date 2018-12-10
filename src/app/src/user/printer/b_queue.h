#ifndef _B_QUEUE_H
#define _B_QUEUE_H 

#define __ENTER_CRITICAL()  
#define __EXIT_CRITICAL()   


//-------定义字节队列---------
typedef struct _b_queue
{
	volatile char lock;
	
	unsigned char *buf;
	
	unsigned long wr_pt;
	
	unsigned long rd_pt;
	
	unsigned long cnt;
	
	unsigned long max_size;
	
}s_B_QUEUE;


//----------------输出函数-------------------
extern int b_queue_init(struct _b_queue *queue, unsigned char *buf, unsigned long max_size);
extern int lock_b_queue(struct _b_queue *queue);
extern int unlock_b_queue(struct _b_queue *queue);
extern signed long out_b_queue(struct _b_queue *queue, unsigned char *buf, signed long cnt);
extern signed long in_b_queue(struct _b_queue *queue, unsigned char *buf, signed long cnt);
extern s8 read_b_queue_fst_byte(struct _b_queue *queue, u8 *_byte);
extern s8 is_queue_enmty(struct _b_queue *queue);
#endif
