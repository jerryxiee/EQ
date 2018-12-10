/*
********************************************************************************
*
*
* Filename      : vdr_usb.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* Time          : 20140723
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "Vdr.h"
#include "ff.h"
/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/

/*
********************************************************************************
*                         LOCAL DATA TYPES
********************************************************************************
*/



/*
********************************************************************************
*                          CONST VARIABLES
********************************************************************************
*/
const u16 CarPlate_Gb2312[]=
{
0xBEA9,0xCFE6,0xBDF2,0xB6F5,0xBBA6,0xD4C1,0xD3E5,0xC7ED,0xBCBD,0xB4A8,
0xBDFA,0xB9F3,0xC7AD,0xC1C9,0xD4C6,0xB5E1,0xBCAA,0xC9C2,0xC7D8,0xD4A5,
0xB8CA,0xC2A4,0xCBD5,0xC7E0,0xD5E3,0xCDEE,0xB2D8,0xC3F6,0xC3C9,0xB8D3,
0xB9F0,0xC2B3,0xC4FE,0xBADA,0xD0C2,0xB8DB,0xB0C4,0xCCA8,0xBEFC,0xBFD5,
0xBAA3,0xB1B1,0xC9F2,0xC0BC,0xBCC3,0xC4CF,0xB9E3,0xB3C9,0xBEAF
};
const u16 CarPlate_Unicode[]=
{
0x4EAC,0x6E58,0x6D25,0x9102,0x6CAA,0x7CA4,0x6E1D,0x743C,0x5180,0x5DDD,
0x664B,0x8D35,0x9ED4,0x8FBD,0x4E91,0x6EC7,0x5409,0x9655,0x79E6,0x8C6B,
0x7518,0x9647,0x82CF,0x9752,0x6D59,0x7696,0x85CF,0x95FD,0x8499,0x8D63,
0x6842,0x9C81,0x5B81,0x9ED1,0x65B0,0x6E2F,0x6FB3,0x53F0,0x519B,0x7A7A,
0x6D77,0x5317,0x6C88,0x5170,0x6D4E,0x5357,0x5E7F,0x6210,0x8B66
};

const u16 CarPlate_Unknown[]=
{
0x672A,0x77E5,0x8F66,0x724C//δ֪����
};

const u16 CarPlate_ExNm[]=
{
0x002E,0x0056,0x0044,0x0052//��չ�� .VDR
};

const u8 Usb_Data_Name[][18]=//��������
{
    "ִ�б�׼�汾���  ",            
    "��ǰ��ʻ����Ϣ    ",
    "ʵʱʱ��          ",
    "�ۼ���ʻ���      ",
    "����ϵ��          ",
    "������Ϣ          ",
    "״̬�ź�������Ϣ  ",
    "��¼��Ψһ�Ա��  ",
    "��ʻ�ٶȼ�¼      ",
    "λ����Ϣ��¼      ",
    "�¹��ɵ��¼      ",
    "��ʱ��ʻ��¼      ",
    "��ʻ����ݼ�¼    ",
    "�ⲿ�����¼      ",
    "�����޸ļ�¼      ",
    "�ٶ�״̬��־      "    
};

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/


/*
********************************************************************************
*                          EXTERN VARIABLES
********************************************************************************
*/
extern FIL file;

/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/


/*
********************************************************************************
*                           FUNCTIONS EXTERN
********************************************************************************
*/

/*
********************************************************************************
*                              FUNCTIONS
********************************************************************************
*/

/**
* @brief  �������У��
* @param  *p:ָ��У������ݣ�
* @param  len:У�����ݵĳ��ȣ�
* @retval None
*/
void Vdr_Usb_XorVer(u8 *pVer,u8 *p,u16 len)
{ 
    u16  i;
    
    for(i=0;i<len;i++)
    {
        *pVer ^= *(p+i);
    }
}
/**
* @brief  д��У��
* @param  None
* @retval None
*/
void Vdr_Usb_WrVer(u16 *p_name)
{ 
    u32 flen;
    u32 fsize; 
    u32 freadnum;//�ļ���С 256��������
    u32 freadrem;//���� 
    u32 faddr;

    u32 i;
    u8  verf;//У��
    u8  buf[260];

    verf  = 0;
    faddr = 0;

    f_close(&file);//�ȹر��ļ�
    
    f_open(&file,p_name,FA_WRITE|FA_READ);//�Կɶ�д��ʽ��

    memset(buf,0,sizeof(buf));
    
    fsize = f_size(&file);//��ȡ�ļ���С
    freadnum = fsize/256;
    freadrem = fsize%256;

    for(i=0;i<freadnum;i++)//��������У��
    {
        f_read(&file,buf,256,&flen);
        faddr += 256;
        f_lseek(&file,faddr);
        Vdr_Usb_XorVer(&verf,buf,256);
    }

    if(freadrem)//��������У��
    {
        f_read(&file,buf,freadrem,&flen);
        faddr += freadrem;
        f_lseek(&file,faddr);
        Vdr_Usb_XorVer(&verf,buf,freadrem);
    }
    
    f_write (&file, &verf, 1, &flen);//д��У��
    f_close(&file);                  //�ȹر��ļ�
    
}
/**
  * @brief  ��¼��unicode�����ļ���
  * @param  None
  * @retval None
  */
void Vdr_Usb_UniNm(WCHAR *p_name)
{
    u8  tab[10];
    u8  plate[20];
    u8  len;
    u8  i;
    u16 tmp;
    WCHAR *pname = p_name;

    memset(plate,0,sizeof(plate));
    Vdr_Mem_Carplate(VDR_MEM_READ,plate);//��ȡ���ƺ�

    len = sizeof(CarPlate_Gb2312);
    tmp = (plate[0]<<8)|plate[1];
    for(i=0;i<len;i++)
    {
        if(tmp == CarPlate_Gb2312[i])
        {
            tmp = CarPlate_Unicode[i];//תΪunicode
            break;
        }
    }
    
    if(strlen((char*)plate) == 0)//���洢��
    {
        len = 0;
        i   = 0;
    }
    if(i == len)//��鳵�ƺ�
    {
        memcpy(pname,CarPlate_Unknown,sizeof(CarPlate_Unknown));
        pname += 4;
        memcpy(pname,CarPlate_ExNm,sizeof(CarPlate_ExNm));
        return;
    }
    
    Public_ConvertNowTimeToBCDEx((u8*)&tab);
    *pname++ = 'D';
    *pname++ = (tab[0]>>4)   + 0x30;
    *pname++ = (tab[0]&0x0f) + 0x30;
    *pname++ = (tab[1]>>4)   + 0x30;
    *pname++ = (tab[1]&0x0f) + 0x30;
    *pname++ = (tab[2]>>4)   + 0x30;
    *pname++ = (tab[2]&0x0f) + 0x30;
    *pname++ = '_';
    *pname++ = (tab[3]>>4)   + 0x30;
    *pname++ = (tab[3]&0x0f) + 0x30;
    *pname++ = (tab[4]>>4)   + 0x30;
    *pname++ = (tab[4]&0x0f) + 0x30;  
    *pname++ = '_';
    *pname++ = tmp;
    
    len = strlen((char*)plate)-2;
    for(i=0;i<len;i++)
    {
        *pname++ = plate[2+i];
    }
    memcpy(pname,CarPlate_ExNm,sizeof(CarPlate_ExNm));
    pname += (sizeof(CarPlate_ExNm)/2);

    *pname = 0;
}

/**
  * @brief  ���ݿ�����
  * @param  None
  * @retval �����ļ�ָ��
  */
u32 Vdr_Usb_BlkNm(u8 cmd_addr)
{
    u8  cmd;
    u32 flen;
    u32 flensl;

    cmd = Vdr_Cmd_All[cmd_addr];
    f_write (&file, (u8*)&cmd, 1, &flen);               //����
    f_write (&file, Usb_Data_Name[cmd_addr], 18, &flen);//����

    flensl = f_size(&file);                             //�����ļ�ָ��
    f_write (&file, (u8*)&cmd, 4, &flen);               //����

    return flensl;
}

/**
  * @brief  ���ݿ鳤��
  * @param  flensl->���ȵ�ַ
  * @param  fsize ->���ݳ���
  * @retval None
  */
void Vdr_Usb_BlkLn(u32 flensl, u32 fsize)
{
    u8  tab[5];
    u32 flen;
    
    tab[0] =  fsize>>24;
    tab[1] = (fsize>>16)&0xff;
    tab[2] = (fsize>>8)&0xff;
    tab[3] =  fsize&0xff;
    
    f_lseek(&file,flensl);           //�ƶ��ļ�ָ��������
    f_write (&file, tab, 4, &flen);  //����
    f_lseek(&file,flensl+4+fsize);   //�ָ��ļ�ָ��
}
/**
  * @brief  ���ݿ�����
  * @param  None
  * @retval None
  */
void Vdr_Usb_BlkDt(u8 *p_data, u16 len_data,u32 *pfsize)
{
    u32 flen;
    
    *pfsize += len_data;
    f_write (&file, p_data, len_data, &flen);//д��U��
}

/**
  * @brief  д��00-07��������
  * @param  None
  * @retval None
  */
void Vdr_Usb_Base(void)
{
    u8  tab[25];
    u8  data[100];//��������100��������
    u8  i;
    u8  len;
    u32 flen; 
    VDR_HEAD head;

    tab[0] = 0;
    tab[1] = 16;//00H-15H 16�����ݿ�
    f_write (&file, tab, 2, &flen);                 //���ݿ����

    for(i=0;i<8;i++)
    {
        len       = 0;
        head.pbuf = data;        
        Vdr_Get_Base(i,&head);                      //��ȡ����
        tab[len++] = i;                             //���ݴ���
        memcpy(tab+len,Usb_Data_Name[i],18);        //��������
        len += 18;
        tab[len++] = 0;
        tab[len++] = 0;
        tab[len++] = 0;
        tab[len++] = head.len;                      //����
        f_write (&file, tab, len, &flen);           //д��ͷ
        f_write (&file, head.pbuf, head.len, &flen);//д������
    }
}

/**
  * @brief  10H-15H�������
  * @param  None
  * @retval None
  */
void Vdr_Usb_Package(u8 cmdnum)
{
    u32  flensl= 0;  
    u32  fsize = 0;
    char display[50];
    char *pDsp;
    
    TIME_T      now_time;  //��ǰʱ��
    VDR_PACKINF packageinf;//�ְ���Ϣ
    VDR_PACKSER packageser;//�ְ�����    
    
    ////////////////////////////////////////////////////////////////////////��ʾ
    pDsp = display;
    memcpy(display,Usb_Data_Name[cmdnum],18);
    pDsp += 15;

    ////////////////////////////////////////////////////////////////��ȡ������Ϣ
    RTC_GetCurTime(&now_time);
    now_time.year -= 1;
    packageinf.timestar = ConverseGmtime(&now_time);//��ʼʱ��
    now_time.year += 2;
    packageinf.timeend = ConverseGmtime(&now_time);//����ʱ��
    packageinf.cmdnum  = cmdnum;                   //�������

    //////////////////////////////////////////////////////////////////////��ʼ��
    sprintf(pDsp," ���ڼ����ܰ���...");
    LcdShowMsgEx(display,100);
    Vdr_Get_PackageInf(packageinf,&packageser);
    
    ////////////////////////////////////////////////////////////////////////����
    flensl = Vdr_Usb_BlkNm(cmdnum);

    ////////////////////////////////////////////////////////////////////////����
    if(packageser.cnt < packageser.all)
    {
        for(;;)
        {
            sprintf(pDsp," All=%d,Cnt=%d",packageser.all,packageser.cnt);
            LcdShowMsgEx(display,100);
            Vdr_Get_PackageSer(&packageser);//��ȡ����
            Vdr_Usb_BlkDt(packageser.pbuf,packageser.len,&fsize);//д���ļ� 
            if(packageser.cnt >= packageser.all)
            {
                break;
            }
        }
    }
    ////////////////////////////////////////////////////////////////////////����  
    Vdr_Usb_BlkLn(flensl,fsize);//д�볤�� 
}
/**
  * @brief  д���¼����
  * @param  None
  * @retval None
  */
void Vdr_Usb_Ser(void)
{
    u8 i;
    for(i=8;i<16;i++)
    {
        Vdr_Usb_Package(i);
    } 
}

/**
  * @brief  
  * @param  None
  * @retval None
  */
void Vdr_Usb_Handle(void)
{
    WCHAR filename[50];

    LCD_LIGHT_ON();
    
    Vdr_Usb_UniNm(filename);//�����ļ���
    
    if(f_open(&file,(WCHAR*)&filename,FA_CREATE_ALWAYS|FA_WRITE) == FR_OK)
    {
        Vdr_Usb_Base(); //00H-07H��������
        
        Vdr_Usb_Ser();  //08H-15H��¼����

        Vdr_Usb_WrVer(filename);//д��У��
        
        LcdShowMsgEx("���ݵ������",100);
    }
    else
    {
        LcdShowMsgEx("���ݵ�������!!",100);
    }
}




