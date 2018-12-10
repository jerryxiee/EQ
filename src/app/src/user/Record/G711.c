/* Includes ------------------------------------------------------------------*/
#include "G711.h" 

#define	SIGN_BIT	(0x80)	/* 注册了一个A-law字节位 , Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)   /* 量化领域的掩码 , Quantization field mask. */
#define	NSEGS		(8)		/* a-law段的数目 , Number of A-law segments. */
#define	SEG_SHIFT	(4)		/* 段数左移 , Left shift for segment number. */
#define	SEG_MASK	(0x70)	/* 段领域的掩码 , Segment field mask. */

static short seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF,0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};

/* CCITT G.711规范的复制 , copy from CCITT G.711 specifications */
unsigned char _u2a[128] = {			/* u-law带A-law的转换 , u- to A-law conversions */
	1,	1,	2,	2,	3,	3,	4,	4,
	5,	5,	6,	6,	7,	7,	8,	8,
	9,	10,	11,	12,	13,	14,	15,	16,
	17,	18,	19,	20,	21,	22,	23,	24,
	25,	27,	29,	31,	33,	34,	35,	36,
	37,	38,	39,	40,	41,	42,	43,	44,
	46,	48,	49,	50,	51,	52,	53,	54,
	55,	56,	57,	58,	59,	60,	61,	62,
	64,	65,	66,	67,	68,	69,	70,	71,
	72,	73,	74,	75,	76,	77,	78,	79,
	81,	82,	83,	84,	85,	86,	87,	88,
	89,	90,	91,	92,	93,	94,	95,	96,
	97,	98,	99,	100,	101,	102,	103,	104,
	105,	106,	107,	108,	109,	110,	111,	112,
	113,	114,	115,	116,	117,	118,	119,	120,
	121,	122,	123,	124,	125,	126,	127,	128};

unsigned char _a2u[128] = {			/* A-law到u-law的转换 , A- to u-law conversions */
	1,	3,	5,	7,	9,	11,	13,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	32,	33,	33,	34,	34,	35,	35,
	36,	37,	38,	39,	40,	41,	42,	43,
	44,	45,	46,	47,	48,	48,	49,	49,
	50,	51,	52,	53,	54,	55,	56,	57,
	58,	59,	60,	61,	62,	63,	64,	64,
	65,	66,	67,	68,	69,	70,	71,	72,
	73,	74,	75,	76,	77,	78,	79,	79,
	80,	81,	82,	83,	84,	85,	86,	87,
	88,	89,	90,	91,	92,	93,	94,	95,
	96,	97,	98,	99,	100,	101,	102,	103,
	104,	105,	106,	107,	108,	109,	110,	111,
	112,	113,	114,	115,	116,	117,	118,	119,
	120,	121,	122,	123,	124,	125,	126,	127};


//
//table值:0xFF, 0x1FF, 0x3FF, 0x7FF,0xFFF, 0x1FFF, 0x3FFF, 0x7FFF
//size值 :8
//
static int search(int val,short *table,int size)
{
	int i;

	for (i = 0; i < size; i++) {
		if (val <= *table++)
			return (i);
	}
	return (size);
}

/*
 * linear2alaw() - Convert a 16-bit linear PCM value to 8-bit A-law
 *                 将一个16位的线性PCM值转换为8位a-law
 *
 *
 * linear2alaw() accepts an 16-bit integer and encodes it as A-law data.
 *               接受一个16位整数编码为a-law数据。
 * 
 *	Linear Input Code	Compressed Code
 *  线性输入代码        压缩代码
 *	-------------------	---------------
 *	0000000wxyza			000wxyz
 *	0000001wxyza			001wxyz
 *	000001wxyzab			010wxyz
 *	00001wxyzabc			011wxyz
 *	0001wxyzabcd			100wxyz
 *	001wxyzabcde			101wxyz
 *	01wxyzabcdef			110wxyz
 *	1wxyzabcdefg			111wxyz
 *
 * 负数表示方法:
 * 5 ->B0000 0101
 * 反码 ->B1111 1010
 * 反码的补码 ->B1111 1011
 * -5 = B1111 1011 
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 *
 *如需进一步信息，请参阅约翰C ·贝拉米的数字电话，1982年，约翰威利父子，PPS98-111和472-476。
 */
unsigned char linear2alaw(int pcm_val)	/* 2的补码（16位范围）, 2's complement (16-bit range) */
{
    int		mask;
    int		seg;
    unsigned char	aval;
    
    if (pcm_val >= 0) {
            mask = 0xD5;		/* 标志位最高位(第7位)=1,sign (7th) bit = 1 */
    } else {
            mask = 0x55;		/* 标志位最高位(第7位)=0,sign bit = 0 */
            pcm_val = -pcm_val - 8; /* 取反-8*/
    }
    
    /* 转换段数缩放的幅度。Convert the scaled magnitude to segment number. */
    seg = search(pcm_val, seg_end, 8);
    
    /* 结合的符号，段和量化位。Combine the sign, segment, and quantization bits. */
    if (seg >= 8)		/* 超出范围，返回的最大值 , out of range, return maximum value. */
            return (0x7F ^ mask);
    else {
            aval = seg << SEG_SHIFT;
            if (seg < 2)
                    aval |= (pcm_val >> 4) & QUANT_MASK;
            else
                    aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
            return (aval ^ mask);
    }
}

/*
 * alaw2linear() - Convert an A-law value to 16-bit linear PCM
 *                 将一个a-law值转换为16位线性PCM
 *
 */
int alaw2linear(unsigned char	a_val)
{
    int		t;
    int		seg;
    
    a_val ^= 0x55;
    
    t = (a_val & QUANT_MASK) << 4;
    seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
    switch (seg) {
    case 0:
            t += 8;
            break;
    case 1:
            t += 0x108;
            break;
    default:
            t += 0x108;
            t <<= seg - 1;
    }
    return ((a_val & SIGN_BIT) ? t : -t);
}


//将一个单声道,8000HZ,16位的PCM数据转换为G711编码数据
//
int PCM16ConverG711(u8 *buf,uint16_t *pcmBuf,uint32_t len)
{
    int16_t pcm = 0;
    unsigned char a_law;  
    int i = 0,j  = 0;
    
    //将16位PCM数据转换为a-law式
    for(i =0; i <  len; i++){
        pcm = *(pcmBuf+i);
        a_law   = linear2alaw((int)pcm); 
        buf[j++] = a_law;
        i++;                 //跳过右声道数据
    }
    return j;                //返回转换后的长度
}

//
//将a-law格式转换为16位PCM数据
//
int G711ToPCM16(uint16_t *pcmBuf,u8 *g7Buf,uint32_t len)
{
    uint16_t pcm = 0;
    unsigned char a_law;
    int i = 0 , j = 0;
    if(len > 0){
        //将a-law格式转换为16位PCM数据
        for(i =0; i <  len; i++){
            a_law = *g7Buf++;
            pcm   = (uint16_t)alaw2linear(a_law);
            *pcmBuf++ = pcm;
            *pcmBuf++ = pcm;   //填充右声道数据
        }
        return j;              //返回转换后的长度
    }else{
        return 0;
    }
}