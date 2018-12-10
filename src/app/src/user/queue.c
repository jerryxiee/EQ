
/*
*********************************************************************************************************
*
*                                               �㷨
*                                             ѭ������
*
*
* Filename      : queue.c
* Version       : V1.00
* Programmer(s) : dxl,�ɱ�׼�㷨��΢�ı����
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

extern u8	RadioProtocolRx1Buffer[];//����1���ջ���
extern u8	RadioProtocolRx2Buffer[];//����1���ջ���
extern u8	RadioProtocolRxSmsBuffer[];//���Ž��ջ���
/*
*********************************************************************************************************
*                                          ALGQueueIsEmpty()
*
* Description : �ж�һ�������Ƿ�Ϊ��
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
* Description : �ж�һ�������Ƿ�Ϊ��
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
* Description : ��������1���ն���
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : �����ڴ�������1���ն��У��������ڴ��������Ķ���
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
* Description : ��������2���ն���
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : �����ڴ�������2���ն��У��������ڴ��������Ķ���
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
* Description : �������Ž��ն���
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : �����ڴ������Ž��ն��У��������ڴ��������Ķ���
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
* Description : ����һ��ѭ������
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
* Description : ʹ����Ϊ��
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
* Description : �ͷ�һ������
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : һ��Ƕ��ʽӦ���в�Ҫ�����������ͷţ�����ʹ���ͷź���
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
* Description : valueֵ��1
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
* Description : ��������һ��Ԫ�أ��������
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
* Description : �����һ����Ӷ��Ԫ��
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
* Description : ��������Ԫ��
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
* Description : ɾ�������е�һ��Ԫ�أ���������
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
* Description : ��ȡ�����е���Ԫ�أ�Ȼ�������
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
* Description : ��ȡ�����е���Ԫ�أ�Ȼ������У��������Ƿ�֡β�ı�־��
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : *pFlag == 1����ǰ�����ַ�����Ч�ģ�*pFlag == 0����ǰ�����ַ�����Ч�ģ�
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
* Description : �����һ�ζ�ȡ���Ԫ��
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
* Description : �����пգ��������=0��Ϊ�գ���ʱû��Ԫ�ؿ��Գ���
*
* Argument(s) : ѭ������ָ��
*
* Return(s)   : ����״̬
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
* Description : �����������������=���������ʱ������Ԫ�����
*
* Argument(s) : ѭ������ָ��
*
* Return(s)   : ����״̬
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
* Description : ��ȡ���г���
*
* Argument(s) : ѭ������ָ��
*
* Return(s)   : ���г���
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
* Description : Ԫ�����
*
* Argument(s) : element:����ӵ�Ԫ��   
*								pQ:ѭ������ָ��
*
* Return(s)   : ����״̬
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
  //��Ϊ�����ǻ��Σ�����tail��Ҫͨ��ȡģ��ʵ��ת�ص�0λ��
  pQ->m_iTail = pQ->m_iTail % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth++;
	
  return Q_STATE_SUCCESS;
}
/*********************************************************************************************************
*                                          BufferEnCirQueue
*
* Description : ��Ԫ�����
*
* Argument(s) : pBuffer:����ӵ�����ָ�� 
*								BufferLen:������ݳ���
*								pQ:ѭ������ָ��
*
* Return(s)   : ������ݳ���
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
int BufferEnCirQueue(ElementType *pBuffer, int BufferLen, pCirQueue pQ)
{
	int	increaseLens;
	int residueLen = pQ->m_iQueueCapacity - pQ->m_iQueueLenth;//ʣ������
	int	preZeroLen = pQ->m_iQueueCapacity - pQ->m_iTail;//��β��0ǰ�ĳ���
	
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
		memcpy((void*)&pQ->m_pQueue[0],pBuffer+preZeroLen,increaseLens-preZeroLen);//�������µ�
	}
	pQ->m_iTail += increaseLens;
  //��Ϊ�����ǻ��Σ�����tail��Ҫͨ��ȡģ��ʵ��ת�ص�0λ��
  pQ->m_iTail = pQ->m_iTail % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth += increaseLens;
  
  return increaseLens;
}
/*********************************************************************************************************
*                                          DeCirQueue
*
* Description : Ԫ�س���
*
* Argument(s) : element:���ӵ�����ָ�� 
*								pQ:ѭ������ָ��
*
* Return(s)   : ����״̬
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
  //��Ϊ�����ǻ��Σ�����head��Ҫͨ��ȡģ��ʵ��ת�ص�0λ��
  pQ->m_iHead = pQ->m_iHead % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth--;  
	
  return Q_STATE_SUCCESS;

}
/*********************************************************************************************************
*                                          BufferDeCirQueue
*
* Description : ��Ԫ�س���
*
* Argument(s) : pBuffer:�����ӵ�����ָ�� 
*								BufferLen:�������ݳ���
*								pQ:ѭ������ָ��
*
* Return(s)   : �������ݳ���
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
int BufferDeCirQueue(ElementType *pBuffer, int BufferLen, pCirQueue pQ)
{
	int	increaseLens;
  int	preZeroLen = pQ->m_iQueueCapacity - pQ->m_iHead;//��ͷ��0ǰ�ĳ���
  
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
		memcpy(pBuffer+preZeroLen,&pQ->m_pQueue[0],increaseLens-preZeroLen);//�������µ�
	}
  
	pQ->m_iHead += increaseLens;
  //��Ϊ�����ǻ��Σ�����head��Ҫͨ��ȡģ��ʵ��ת�ص�0λ��
  pQ->m_iHead = pQ->m_iHead % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth -= increaseLens; 
	
	return increaseLens;
}
/*********************************************************************************************************
*                                          AllBufferDeCirQueue
*
* Description : ����Ԫ�س���
*
* Argument(s) : pBuffer:�����ӵ�����ָ�� 
*								pQ:ѭ������ָ��
*
* Return(s)   : ����״̬
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
Q_STATE AllBufferDeCirQueue(ElementType *pBuffer,pCirQueue pQ)
{
  int	preZeroLen = pQ->m_iQueueCapacity - pQ->m_iHead;//��ͷ��0ǰ�ĳ���

	if((NULL == pQ)||(NULL == pBuffer))return Q_STATE_FAILED;
	if (Q_STATE_SUCCESS == IsCirQueueEmpty(pQ))return Q_STATE_FAILED;
	
	if(preZeroLen > pQ->m_iQueueLenth)
	{
		memcpy(pBuffer,&pQ->m_pQueue[pQ->m_iHead],pQ->m_iQueueLenth);
	}
	else
	{
		memcpy(pBuffer,&pQ->m_pQueue[pQ->m_iHead],preZeroLen);
		memcpy(pBuffer+preZeroLen,&pQ->m_pQueue[0],pQ->m_iQueueLenth-preZeroLen);//�������µ�
	}
  
	pQ->m_iHead += pQ->m_iQueueLenth;
  //��Ϊ�����ǻ��Σ�����head��Ҫͨ��ȡģ��ʵ��ת�ص�0λ��
  pQ->m_iHead = pQ->m_iHead % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth = 0; 
	
	return Q_STATE_SUCCESS;
}
/*********************************************************************************************************
*                                          CreatCirQueue
*
* Description :���г�ʼ����ȷ�϶��������������ڴ档ͷβ�ͳ���Ϊ0
*
* Argument(s) : queueCapacity:��������
*
* Return(s)   : ����Ķ���ָ��
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
* Description : �ڵ����
*
* Argument(s) : pBuffer:����ӵ�����ָ�� 
*								BufferLen:������ݳ���
*								pQ:ѭ������ָ��
*
* Return(s)   : ������ݳ���
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
int BufferNodeEnCirQueue(ElementType *pBuffer, int BufferLen, pCirQueue pQ)
{
	int	increaseLens;
	
	int residueLen = pQ->m_iQueueCapacity - pQ->m_iQueueLenth;//ʣ������
	int	preZeroLen = pQ->m_iQueueCapacity - pQ->m_iTail;//��β��0ǰ�ĳ���

	Q_NODE *pNode;
	
	if((NULL == pQ)||(0 == BufferLen))return 0;
	if((pQ->m_iNodeNum+1) >= pQ->m_iMaxNodeNum)
  {
      return 0;//�ڵ㻺������
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
		memcpy((void*)&pQ->m_pQueue[0],pBuffer+preZeroLen,increaseLens-preZeroLen);//�������µ�
	}
	pQ->m_iTail += increaseLens;
  //��Ϊ�����ǻ��Σ�����tail��Ҫͨ��ȡģ��ʵ��ת�ص�0λ��
  pQ->m_iTail = pQ->m_iTail % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth += increaseLens;

	pNode->lens = increaseLens;
	pQ->m_iNodeTail += 1;//�ڵ�β��1
  pQ->m_iNodeTail %= pQ->m_iMaxNodeNum;
  pQ->m_iNodeNum += 1;//�ڵ������1
  
  return increaseLens;
}
/*********************************************************************************************************
*                                          BufferNodeDeCirQueue
*
* Description : �ڵ����
*
* Argument(s) : pBuffer:�����ӵ�����ָ�� 
*								BufferLen:�������ݳ���
*								pQ:ѭ������ָ��
*
* Return(s)   : �������ݳ���
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
int BufferNodeDeCirQueue(ElementType *pBuffer,pCirQueue pQ)
{
	int deBufferLen;
	
  int	preZeroLen = pQ->m_iQueueCapacity - pQ->m_iHead;//��ͷ��0ǰ�ĳ���

	Q_NODE *pNode;
	
  if (Q_STATE_SUCCESS == IsCirQueueEmpty(pQ))return 0;
  if(NULL == pQ)return 0;

	if(0==pQ->m_iNodeNum)return 0;//˵��û������
	
	//����������Ϣ
	pNode = pQ->m_pNode + pQ->m_iNodeHead;//�ڵ��ַ
  deBufferLen   = pNode->lens;
	
 	if(preZeroLen >= deBufferLen)
	{
		memcpy(pBuffer,&pQ->m_pQueue[pQ->m_iHead],deBufferLen);
	}
	else
	{
		memcpy(pBuffer,&pQ->m_pQueue[pQ->m_iHead],preZeroLen);
		memcpy(pBuffer+preZeroLen,&pQ->m_pQueue[0],deBufferLen-preZeroLen);//�������µ�
	}
  
	pQ->m_iHead += deBufferLen;
  //��Ϊ�����ǻ��Σ�����head��Ҫͨ��ȡģ��ʵ��ת�ص�0λ��
  pQ->m_iHead = pQ->m_iHead % pQ->m_iQueueCapacity;
  pQ->m_iQueueLenth -= deBufferLen; 

	pQ->m_iNodeHead += 1;         //�ڵ�ͷ��1
  pQ->m_iNodeHead %= pQ->m_iMaxNodeNum;
  pQ->m_iNodeNum -= 1;         //�ڵ������1
  
  pNode->lens = 0;  //���ٳ���
  
	return deBufferLen;
}
#endif
/*********************************************************************************************************
*                                          DestroyCirQueue
*
* Description :���й���
*
* Argument(s) : pQ:ѭ������ָ��
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
