/*****************************************************************************
*Function Name:  Polygon.h
*Input  Parameter:  None
*Output Parameter:  None 
*Function Describtion:Polygon area recognition 
*Writer:mark
*Update Data:2013.10.15
******************************************************************************/
#ifndef __POLYGON__H_
#define __POLYGON__H_
/*****************************************************************************
*Function Name:  Polygon_ParameterInitialize
*Input  Parameter:  None
*Output Parameter:  None 
*Function Describtion:Initialize the area parameter 
*Writer:mark
*Update Data:2013.10.15
******************************************************************************/
void Polygon_ParameterInitialize(void);
/*****************************************************************************
*Function Name:  Polygon_TimeTask
*Input  Parameter:  None
*Output Parameter:  None 
*Function Describtion:Arrange all timer task 
*Writer:mark
*Update Data:2013.10.15
******************************************************************************/
FunctionalState Polygon_TimeTask(void) ;
/*****************************************************************************
*Function Name:  Polygon_SetArea(unsigned char *pBuffer, unsigned short BufferLen)
*Input  Parameter:  *pbuffer,bufferlen
*Output Parameter:  Polygon area 
*Function Describtion:Set polygon area
*Writer:mark
*Update Data:2013.10.15
******************************************************************************/
unsigned char Polygon_SetArea(unsigned char *pBuffer, unsigned short BufferLen);

/*****************************************************************************
*Function Name:  Polygon_DeleteArea(unsigned char *pBuffer, unsigned short BufferLen)
*Input  Parameter:  None
*Output Parameter:  None 
*Function Describtion:Delete polygon area 
*Writer:mark
*Update Data:2013.10.15
******************************************************************************/
unsigned char Polygon_DeleteArea(unsigned char *pBuffer, unsigned short BufferLen);
/*************************************************************
** 函数名称: Polygon_AreaModifyUserData
** 功能描述: 修改多边形区域自定义部分
** 入口参数: 
** 出口参数: 
** 返回参数: 
*************************************************************/ 
unsigned char Polygon_AreaModifyUserData(unsigned char *pBuffer, unsigned short BufferLen);
/*************************************************************
** 函数名称: Polygon_GetAreaTotalNum
** 功能描述: 获得多边形区域总数
** 入口参数: 
** 出口参数: 
** 返回参数: 多边形区域总数
*************************************************************/ 
unsigned char Polygon_GetAreaTotalNum(void);
/*****************************************************************************
*Function Name: Polygon_ClearArea()
*Input  Parameter:  None
*Output Parameter:  None
*Function Describtion:clear Polygon area flash sector  
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
void Polygon_ClearArea(void);
#endif