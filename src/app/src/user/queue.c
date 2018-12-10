
/*
*********************************************************************************************************
*
*                                               算法
*                                             循环队列
*
*
* Filename      : queue.c
* Version       : V1.00
* Programmer(s) : dxl,由标准算法略微改编而成
*********************************************************************************************************
*/	
/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "queue.h"
#include "string.h"
#include<stdlib.h>

/*
*********************************************************************************************************
*                                             define
*********************************************************************************************************
*/	

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      LOCAL GLOBAL VARIABLRS
*********************************************************************************************************
*/

extern u8	RadioProtocolRx1Buffer[];//连接1接收缓冲
extern u8	RadioProtocolRx2Buffer[];//连接1接收缓冲
extern u8	RadioProtocolRxSmsBuffer[];//短信接收缓冲
/*
*********************************************************************************************************
*                                          ALGQueueIsEmpty()
*
* Description : 判断一个队列是否为空
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
int QueueIsEmpty( Queue Q )
{
    return Q->Size == 0;
}
/*
*********************************************************************************************************
*                                          ALGQueueIsFull()
*
* Description : 判断一个队列是否为满
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
int QueueIsFull( Queue Q )
{
    return Q->Size == Q->Capacity;
}
/*
*********************************************************************************************************
*                                          QueueCreate1
*
* Description : 创建连接1接收队列
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : 仅用于创建连接1接收队列，不能用于创建其它的队列
*********************************************************************************************************
*/
Queue QueueCreate1(int MaxElements)
{
    Queue Q;

    if(MaxElements < MIN_QUEUE_SIZE)
        return NULL;

    Q = (Queue)malloc( sizeof(struct QueueRecord));
    if( Q == NULL )
        return NULL;

    Q->Array = RadioProtocolRx1Buffer;
    
    if( Q->Array == NULL )
        return NULL;
    
    Q->Capacity = MaxElements;
    
    QueueMakeEmpty( Q );

    return Q;
}
/*
*********************************************************************************************************
*                                          QueueCreate2
*
* Description : 创建连接2接收队列
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : 仅用于创建连接2接收队列，不能用于创建其它的队列
*********************************************************************************************************
*/
Queue QueueCreate2(int MaxElements)
{
    Queue Q;

    if(MaxElements < MIN_QUEUE_SIZE)
        return NULL;

    Q = (Queue)malloc( sizeof(struct QueueRecord));
    if( Q == NULL )
        return NULL;

    Q->Array = RadioProtocolRx2Buffer;
    
    if( Q->Array == NULL )
        return NULL;
    
    Q->Capacity = MaxElements;
    
    QueueMakeEmpty( Q );

    return Q;
}
/*
*********************************************************************************************************
*                                          QueueCreate3
*
* Description : 创建短信接收队列
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : 仅用于创建短信接收队列，不能用于创建其它的队列
*********************************************************************************************************
*/
Queue QueueCreate3(int MaxElements)
{
    Queue Q;

    if(MaxElements < MIN_QUEUE_SIZE)
        return NULL;

    Q = (Queue)malloc( sizeof(struct QueueRecord));
    if( Q == NULL )
        return NULL;

    Q->Array = RadioProtocolRxSmsBuffer;
    
    if( Q->Array == NULL )
        return NULL;
    
    Q->Capacity = MaxElements;
    
    QueueMakeEmpty( Q );

    return Q;
}
/*
*********************************************************************************************************
*                                          ALGQueueCreate()
*
* Description : 创建一个循环队列
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
Queue QueueCreate(int MaxElements)
{
    Queue Q;

    if(MaxElements < MIN_QUEUE_SIZE)
        return NULL;

    Q = (Queue)malloc( sizeof(struct QueueRecord));
    if( Q == NULL )
        return NULL;

    Q->Array = malloc( sizeof(ElementType) * MaxElements);
    
    if( Q->Array == NULL )
        return NULL;
    
    Q->Capacity = MaxElements;
    
    QueueMakeEmpty( Q );

    return Q;
}
/*
*********************************************************************************************************
*                                          ALGQueueMakeEmpty()
*
* Description : 使队列为空
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
void QueueMakeEmpty(Queue Q)
{
    Q->Size = 0;
    Q->Front = 1;
    Q->Rear = 0;
}
/*
*********************************************************************************************************
*                                          ALGQueueDispose()
*
* Description : 释放一个队列
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : 一般嵌入式应用中不要经常创建、释放，即不使用释放函数
*********************************************************************************************************
*/
void QueueDispose(Queue Q)
{
    if( Q != NULL )
    {
        free( Q->Array );
        free( Q );
    }
}
/*
*********************************************************************************************************
*                                          ALGQueueSucc()
*
* Description : value值加1
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
int QueueSucc(int Value, Queue Q)
{
    if( ++Value == Q->Capacity )
        Value = 0;
    return Value;
}
/*
*********************************************************************************************************
*                                          ALGQueueIn()
*
* Description : 向队列添加一个元素，即入队列
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
void QueueIn(ElementType X, Queue Q)
{
    if(QueueIsFull(Q))
    {
        return ;
    }
    else
    {
        Q->Size++;
        Q->Rear = QueueSucc( Q->Rear, Q );
        Q->Array[ Q->Rear ] = X;
     }
}
/*
*********************************************************************************************************
*                                          ALGQueueInBuffer()
*
* Description : 向队列一次添加多个元素
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
void QueueInBuffer(ElementType *pBuffer, int BufferLen, Queue Q)
{
    int i = 0;
    ElementType X;
    
    if((Q->Capacity - Q->Size) > (BufferLen))
    {
        for(i=0; i<BufferLen; i++)
        {
            X = *(pBuffer+i);
            Q->Size++;
            Q->Rear = QueueSucc( Q->Rear, Q );
            Q->Array[ Q->Rear ] = X;
        }
    } 
}
/*
*********************************************************************************************************
*                                          ALGQueueFront()
*
* Description : 读队列首元素
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
ElementType QueueFront(Queue Q)
{
    if(!QueueIsEmpty(Q))
    {
        return Q->Array[ Q->Front ];
    }
    else
    {
        return 0;  /* Return value used to avoid warning */
    }
}
/*
*********************************************************************************************************
*                                          ALGQueueOut()
*
* Description : 删除队列中的一个元素，即出队列
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
void QueueOut(Queue Q)
{
    if(QueueIsEmpty(Q))
    {
        return ;
    }
    else
    {
        Q->Size--;
        Q->Front = QueueSucc( Q->Front, Q );
    }
}
/*
*********************************************************************************************************
*                                          ALGQueueFrontAndOut()
*
* Description : 读取队列中的首元素，然后出队列
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
ElementType QueueFrontAndOut(Queue Q)
{
    ElementType X = 0;
    
    if(QueueIsEmpty(Q))
    {
                
    }
    else
    {
        Q->Size--;
        X = Q->Array[ Q->Front ];
        Q->Front = QueueSucc( Q->Front, Q );
    }
    return X;
}
/*
*********************************************************************************************************
*                                          ALGQueueFrontAndOut2()
*
* Description : 读取队列中的首元素，然后出队列，并返回是否帧尾的标志符
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : *pFlag == 1：当前读出字符是有效的；*pFlag == 0：当前读出字符是无效的；
*********************************************************************************************************
*/
ElementType QueueFrontAndOut2(Queue Q, u8 *pFlag)
{
    ElementType X = 0;
    
    if(QueueIsEmpty(Q))
    {
        *pFlag = 0;
        return X;
    }

    Q->Size--;
    X = Q->Array[ Q->Front ];
    Q->Front = QueueSucc( Q->Front, Q );
    
    *pFlag = 1;
    return X;
}
/*
*********************************************************************************************************
*                                          ALGQueueOutBuffer()
*
* Description : 向队列一次读取多个元素
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
int  QueueOutBuffer(ElementType *pBuffer, int BufferLen, Queue Q)
{
    int count = 0;
    ElementType *p = NULL;
    
    if(NULL != p)
    {
        return 0;
    }
    
    p = pBuffer;
    
    while(count < BufferLen)
    {
        if(QueueIsEmpty(Q))
        {
            break;
        }
        else
        {
            Q->Size--;
            *p++ = Q->Array[ Q->Front ];
            Q->Front = QueueSucc( Q->Front, Q );
            count++;
        }
        
    }
    return count;
}

/*********************************************************************************************************
*                                          IsCirQueueEmpty
*
* Description : 队列判空，如果长度=0则为空，这时没有元素可以出队
*
* Argument(s) : 循环队列指针
*
* Return(s)   : 队列状态
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
Q_STATE IsCirQueueEmpty(pCirQueue pQ)
{
  if (pQ->m_iQueueLenth == 0)
  {
      return Q_STATE_SUCCESS;
  }
  return Q_STATE_FAILED;
}
/*********************************************************************************************************
*                                          IsCirQueueFull
*
* Description : 队列判满，如果长度=容量，则此时不能有元素入队
*
* Argument(s) : 循环队列指针
*
* Return(s)   : 队列状态
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
Q_STATE IsCirQueueFull(pCirQueue pQ)
{
  if (pQ->m_iQueueLenth == pQ->m_iQueueCapacity)
  {
      return Q_STATE_SUCCESS;
  }
	return Q_STATE_FAILED;
}
/*********************************************************************************************************
*                                          GetCirQueueLength
*
* Description : 获取队列长度
*
* Argument(s) : 循环队列指针
*
* Return(s)   : 队列长度
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
int	GetCirQueueLength(pCirQueue pQ)
{
	return	pQ->m_iQueueLenth;
}
/*********************************************************************************************************
*                                          EnCirQueue
*
* Description : 元素入队
*
* Argument(s) : element:待入队的元素   
*								pQ:循环队列指针
*
* Return(s)   : 队列状态
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
Q_STATE EnCirQueue(ElementType element,pCirQueue pQ)
{
  if (Q_STATE_SUCCESS == IsCirQueueFull(pQ))
  {
      return Q_STATE_FAILED;
  }
  pQ->m_pQueue[pQ->m_iTail++] = element;
  //因为队列是环形，所以tail需要通过取模来实现转回到0位置
  pQ->m_iTail = pQ->m_iTail % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth++;
	
  return Q_STATE_SUCCESS;
}
/*********************************************************************************************************
*                                          BufferEnCirQueue
*
* Description : 多元素入队
*
* Argument(s) : pBuffer:待入队的数据指针 
*								BufferLen:入队数据长度
*								pQ:循环队列指针
*
* Return(s)   : 入队数据长度
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
int BufferEnCirQueue(ElementType *pBuffer, int BufferLen, pCirQueue pQ)
{
	int	increaseLens;
	int residueLen = pQ->m_iQueueCapacity - pQ->m_iQueueLenth;//剩余容量
	int	preZeroLen = pQ->m_iQueueCapacity - pQ->m_iTail;//队尾置0前的长度
	
	if((NULL == pQ)||(0 == BufferLen))return 0;
	if(BufferLen > residueLen)
	{
		increaseLens = residueLen;
	}
	else
	{
		increaseLens = BufferLen;
	}
	if(preZeroLen >= increaseLens)
	{
		memcpy((void*)&pQ->m_pQueue[pQ->m_iTail],pBuffer,increaseLens);
	}
	else
	{
		memcpy((void*)&pQ->m_pQueue[pQ->m_iTail],pBuffer,preZeroLen);
		memcpy((void*)&pQ->m_pQueue[0],pBuffer+preZeroLen,increaseLens-preZeroLen);//补上余下的
	}
	pQ->m_iTail += increaseLens;
  //因为队列是环形，所以tail需要通过取模来实现转回到0位置
  pQ->m_iTail = pQ->m_iTail % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth += increaseLens;
  
  return increaseLens;
}
/*********************************************************************************************************
*                                          DeCirQueue
*
* Description : 元素出队
*
* Argument(s) : element:出队的数据指针 
*								pQ:循环队列指针
*
* Return(s)   : 队列状态
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
Q_STATE DeCirQueue(ElementType *element,pCirQueue pQ)
{
  if (Q_STATE_SUCCESS == IsCirQueueEmpty(pQ))
  {
      return Q_STATE_FAILED;
  }
  *element = pQ->m_pQueue[pQ->m_iHead++];
  //因为队列是环形，所以head需要通过取模来实现转回到0位置
  pQ->m_iHead = pQ->m_iHead % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth--;  
	
  return Q_STATE_SUCCESS;

}
/*********************************************************************************************************
*                                          BufferDeCirQueue
*
* Description : 多元素出队
*
* Argument(s) : pBuffer:待出队的数据指针 
*								BufferLen:出队数据长度
*								pQ:循环队列指针
*
* Return(s)   : 出队数据长度
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
int BufferDeCirQueue(ElementType *pBuffer, int BufferLen, pCirQueue pQ)
{
	int	increaseLens;
  int	preZeroLen = pQ->m_iQueueCapacity - pQ->m_iHead;//队头置0前的长度
  
  if (Q_STATE_SUCCESS == IsCirQueueEmpty(pQ))return 0;
  if((NULL == pQ)||(0 == BufferLen))return 0;
	
	if(BufferLen > pQ->m_iQueueLenth)
	{
		increaseLens = pQ->m_iQueueLenth;
	}
	else
	{
		increaseLens = BufferLen;
	}
 	if(preZeroLen >= increaseLens)
	{
		memcpy(pBuffer,&pQ->m_pQueue[pQ->m_iHead],increaseLens);
	}
	else
	{
		memcpy(pBuffer,&pQ->m_pQueue[pQ->m_iHead],preZeroLen);
		memcpy(pBuffer+preZeroLen,&pQ->m_pQueue[0],increaseLens-preZeroLen);//补上余下的
	}
  
	pQ->m_iHead += increaseLens;
  //因为队列是环形，所以head需要通过取模来实现转回到0位置
  pQ->m_iHead = pQ->m_iHead % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth -= increaseLens; 
	
	return increaseLens;
}
/*********************************************************************************************************
*                                          AllBufferDeCirQueue
*
* Description : 所有元素出队
*
* Argument(s) : pBuffer:待出队的数据指针 
*								pQ:循环队列指针
*
* Return(s)   : 队列状态
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
Q_STATE AllBufferDeCirQueue(ElementType *pBuffer,pCirQueue pQ)
{
  int	preZeroLen = pQ->m_iQueueCapacity - pQ->m_iHead;//队头置0前的长度

	if((NULL == pQ)||(NULL == pBuffer))return Q_STATE_FAILED;
	if (Q_STATE_SUCCESS == IsCirQueueEmpty(pQ))return Q_STATE_FAILED;
	
	if(preZeroLen > pQ->m_iQueueLenth)
	{
		memcpy(pBuffer,&pQ->m_pQueue[pQ->m_iHead],pQ->m_iQueueLenth);
	}
	else
	{
		memcpy(pBuffer,&pQ->m_pQueue[pQ->m_iHead],preZeroLen);
		memcpy(pBuffer+preZeroLen,&pQ->m_pQueue[0],pQ->m_iQueueLenth-preZeroLen);//补上余下的
	}
  
	pQ->m_iHead += pQ->m_iQueueLenth;
  //因为队列是环形，所以head需要通过取模来实现转回到0位置
  pQ->m_iHead = pQ->m_iHead % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth = 0; 
	
	return Q_STATE_SUCCESS;
}
/*********************************************************************************************************
*                                          CreatCirQueue
*
* Description :队列初始化，确认队列容量，申请内存。头尾和长度为0
*
* Argument(s) : queueCapacity:队列容量
*
* Return(s)   : 申请的队列指针
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
pCirQueue CreatCirQueue(int queueCapacity)
{
	pCirQueue	m_pQueue;

	if(queueCapacity < MIN_QUEUE_SIZE)
      return NULL;

  m_pQueue = (pCirQueue)malloc(sizeof(CirQueue));
  if( m_pQueue == NULL )
  	return NULL;
  m_pQueue->m_iQueueCapacity = queueCapacity;
  m_pQueue->m_iHead = 0;
  m_pQueue->m_iTail = 0;
  m_pQueue->m_iQueueLenth = 0;
	m_pQueue->m_pQueue = (ElementType*)malloc(queueCapacity);
	if( m_pQueue->m_pQueue == NULL )
		return NULL;
	memset(m_pQueue->m_pQueue,0,queueCapacity);
	
	return m_pQueue;
}
#ifdef	QUEUE_HAVE_NODE
pCirQueue xDynamicCirQueueGenericCreate(int queueCapacity,int nodeNum)
{
	pCirQueue	m_pQueue;

	if(queueCapacity < MIN_QUEUE_SIZE)
      return NULL;

  m_pQueue = (pCirQueue)malloc(sizeof(CirQueue));
  if( m_pQueue == NULL )
  	return NULL;
  m_pQueue->m_iQueueCapacity 	= queueCapacity;
  m_pQueue->m_iHead 					= 0;
  m_pQueue->m_iTail 					= 0;
  m_pQueue->m_iQueueLenth 		= 0;
	m_pQueue->m_iNodeHead 			= 0;
	m_pQueue->m_iNodeTail 			= 0;
	m_pQueue->m_iNodeNum 				= 0;
	m_pQueue->m_pQueue = (ElementType*)malloc(queueCapacity*sizeof(ElementType));
	if( m_pQueue->m_pQueue == NULL )
		return NULL;
	m_pQueue->m_pNode = (Q_NODE*)malloc(nodeNum*sizeof(Q_NODE));
	if( m_pQueue->m_pNode == NULL )
		return NULL;
	m_pQueue->m_iMaxNodeNum = nodeNum;
	m_pQueue->m_pNode->lens = 0;
	//m_pQueue->m_pNode->isValid = 0;
	
	return m_pQueue;
}
Q_STATE eStaticCirQueueGenericCreate(int queueCapacity,
																									int nodeNum,
																									pCirQueue	m_pQueue,
																									ElementType *pQBuffer,
																									Q_NODE *pNodeBuffer)
{

	if(queueCapacity < MIN_QUEUE_SIZE)return Q_STATE_FAILED;

  memset((pCirQueue)m_pQueue,0,sizeof(CirQueue));

  m_pQueue->m_iQueueCapacity 	= queueCapacity;
	m_pQueue->m_pQueue 					= pQBuffer;
	m_pQueue->m_pNode 					= pNodeBuffer;
	m_pQueue->m_iMaxNodeNum 		= nodeNum;
	m_pQueue->m_pNode->lens 		= 0;

	return Q_STATE_SUCCESS;
}

/*********************************************************************************************************
*                                          BufferNodeEnCirQueue
*
* Description : 节点入队
*
* Argument(s) : pBuffer:待入队的数据指针 
*								BufferLen:入队数据长度
*								pQ:循环队列指针
*
* Return(s)   : 入队数据长度
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
int BufferNodeEnCirQueue(ElementType *pBuffer, int BufferLen, pCirQueue pQ)
{
	int	increaseLens;
	
	int residueLen = pQ->m_iQueueCapacity - pQ->m_iQueueLenth;//剩余容量
	int	preZeroLen = pQ->m_iQueueCapacity - pQ->m_iTail;//队尾置0前的长度

	Q_NODE *pNode;
	
	if((NULL == pQ)||(0 == BufferLen))return 0;
	if((pQ->m_iNodeNum+1) >= pQ->m_iMaxNodeNum)
  {
      return 0;//节点缓存已满
  }

	pNode = pQ->m_pNode + pQ->m_iNodeTail;
	
	if(BufferLen > residueLen)
	{
		increaseLens = residueLen;
	}
	else
	{
		increaseLens = BufferLen;
	}
	
	if(preZeroLen >= increaseLens)
	{
		memcpy((void*)&pQ->m_pQueue[pQ->m_iTail],pBuffer,increaseLens);
	}
	else
	{
		memcpy((void*)&pQ->m_pQueue[pQ->m_iTail],pBuffer,preZeroLen);
		memcpy((void*)&pQ->m_pQueue[0],pBuffer+preZeroLen,increaseLens-preZeroLen);//补上余下的
	}
	pQ->m_iTail += increaseLens;
  //因为队列是环形，所以tail需要通过取模来实现转回到0位置
  pQ->m_iTail = pQ->m_iTail % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth += increaseLens;

	pNode->lens = increaseLens;
	pQ->m_iNodeTail += 1;//节点尾加1
  pQ->m_iNodeTail %= pQ->m_iMaxNodeNum;
  pQ->m_iNodeNum += 1;//节点个数加1
  
  return increaseLens;
}
/*********************************************************************************************************
*                                          BufferNodeDeCirQueue
*
* Description : 节点出队
*
* Argument(s) : pBuffer:待出队的数据指针 
*								BufferLen:出队数据长度
*								pQ:循环队列指针
*
* Return(s)   : 出队数据长度
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
int BufferNodeDeCirQueue(ElementType *pBuffer,pCirQueue pQ)
{
	int deBufferLen;
	
  int	preZeroLen = pQ->m_iQueueCapacity - pQ->m_iHead;//队头置0前的长度

	Q_NODE *pNode;
	
  if (Q_STATE_SUCCESS == IsCirQueueEmpty(pQ))return 0;
  if(NULL == pQ)return 0;

	if(0==pQ->m_iNodeNum)return 0;//说明没有数据
	
	//出队数据信息
	pNode = pQ->m_pNode + pQ->m_iNodeHead;//节点地址
  deBufferLen   = pNode->lens;
	
 	if(preZeroLen >= deBufferLen)
	{
		memcpy(pBuffer,&pQ->m_pQueue[pQ->m_iHead],deBufferLen);
	}
	else
	{
		memcpy(pBuffer,&pQ->m_pQueue[pQ->m_iHead],preZeroLen);
		memcpy(pBuffer+preZeroLen,&pQ->m_pQueue[0],deBufferLen-preZeroLen);//补上余下的
	}
  
	pQ->m_iHead += deBufferLen;
  //因为队列是环形，所以head需要通过取模来实现转回到0位置
  pQ->m_iHead = pQ->m_iHead % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth -= deBufferLen; 

	pQ->m_iNodeHead += 1;         //节点头加1
  pQ->m_iNodeHead %= pQ->m_iMaxNodeNum;
  pQ->m_iNodeNum -= 1;         //节点个数减1
  
  pNode->lens = 0;  //销毁长度
  
	return deBufferLen;
}
#endif
/*********************************************************************************************************
*                                          DestroyCirQueue
*
* Description :队列构销
*
* Argument(s) : pQ:循环队列指针
*
* Return(s)   : none
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
void DestroyCirQueue(pCirQueue pQ)
{
	if( pQ != NULL )
  {
    free( pQ->m_pQueue);pQ->m_pQueue = NULL;
		#ifdef	QUEUE_HAVE_NODE
    free( pQ->m_pNode);pQ->m_pNode = NULL;
		#endif
		free( pQ );pQ = NULL;
  }
}
#if 0
static pCirQueue	pQ_Self_Test;
ElementType	InBuffer1[10] = {1,2,6,7,8,9,8,11,12,70};
ElementType	InBuffer2[10] = {4,5,6,7,8,4,10,11,12,0};
ElementType	InBuffer3[10] = {3,9,6,7,8,9,10,11,12,1};
ElementType	OutBuffer1[10];
ElementType QBuffer[100];
Q_NODE 			NodeBuffer[10];
CirQueue		Q_Static;

void CirQueue_Self_Test(void)
{
	ElementType	InBuffer[10] = {4,5,6,7,8,9,10,11,12,13};
	ElementType	OutBuffer[20];
	ElementType	el;
	
	pCirQueue CircleQueue = CreatCirQueue(20);

	if(NULL == CircleQueue)return;
	
	EnCirQueue(1,CircleQueue);
	EnCirQueue(2,CircleQueue);
	EnCirQueue(3,CircleQueue);

	BufferEnCirQueue(InBuffer,10,CircleQueue);
	//BufferDeCirQueue(OutBuffer,10,CircleQueue);
	EnCirQueue(6,CircleQueue);
	EnCirQueue(7,CircleQueue);
	EnCirQueue(8,CircleQueue);
	DeCirQueue(&el,CircleQueue);
	DeCirQueue(&el,CircleQueue);
	DeCirQueue(&el,CircleQueue);
	BufferDeCirQueue(OutBuffer, 10, CircleQueue);
	BufferEnCirQueue(InBuffer,10,CircleQueue);
  AllBufferDeCirQueue(OutBuffer,CircleQueue);
#ifdef	QUEUE_HAVE_NODE
	vDynamicCirQueueCreate(pQ_Self_Test,100,5);
	BufferNodeEnCirQueue(InBuffer1,8,pQ_Self_Test);
	BufferNodeEnCirQueue(InBuffer2,9,pQ_Self_Test);
	BufferNodeEnCirQueue(InBuffer3,10,pQ_Self_Test);
	BufferNodeDeCirQueue(OutBuffer1,pQ_Self_Test);
	BufferNodeDeCirQueue(OutBuffer1,pQ_Self_Test);
	BufferNodeDeCirQueue(OutBuffer1,pQ_Self_Test);
	BufferNodeDeCirQueue(OutBuffer1,pQ_Self_Test);
	
	if(Q_STATE_FAILED == eStaticCirQueueGenericCreate(100,
																									5,
																									&Q_Static,
																									QBuffer,
																									NodeBuffer))
		return;
	BufferNodeEnCirQueue(InBuffer1,8,&Q_Static);
	BufferNodeEnCirQueue(InBuffer2,9,&Q_Static);
	BufferNodeEnCirQueue(InBuffer3,10,&Q_Static);
	BufferNodeDeCirQueue(OutBuffer1,&Q_Static);
	BufferNodeDeCirQueue(OutBuffer1,&Q_Static);
	BufferNodeDeCirQueue(OutBuffer1,&Q_Static);
#endif
}
#endif
