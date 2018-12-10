#ifndef _FONT_LIB_H
#define _FONT_LIB_H 

typedef struct _ch_table
{
	unsigned short gbk;
	const char *image;
}s_CH_TABLE;


//extern const struct _ch_table ch_font_16x16_table[1];

extern const struct _ch_table ch_font_24x24_table[127];

extern const char ascii_24x24[];

extern const struct _ch_table ch_font_32x32_table[3];


#endif

