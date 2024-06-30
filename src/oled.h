/*
 *  OLED.H
 *
 *  Modified on: Jun 26, 2024
 *      Author : adamanteye
 *
 *   Created on: Jan 2, 2014
 *      Author : millin
 */

#ifndef OLED_H_
#define OLED_H_

#ifndef _GLOBAL_DECL_
#define _GLOBAL_DECL_ extern
#endif

/******************************************/
/*****              OLED Define        ****/
/******************************************/

#define SSD1325 1 // 另一个型号OLED，TW28642270B，2.7吋
#define SSD1306 2 // 教学系统标配屏幕，型号OLED_UG2864HSWEG01，0.97吋

#define OLED_TYPE SSD1306

/******************************************/
// 相应标志位赋值与检测功能
/******************************************/
#define BIT0  0x01
#define BIT1  (1 << 1)
#define BIT2  (1 << 2)
#define BIT3  (1 << 3)
#define BIT4  (1 << 4)
#define BIT5  (1 << 5)
#define BIT6  (1 << 6)
#define BIT7  (1 << 7)
#define BIT8  (1 << 8)
#define BIT9  (1 << 9)
#define BIT10 (1 << 10)
#define BIT11 (1 << 11)
#define BIT12 (1 << 12)
#define BIT13 (1 << 13)
#define BIT14 (1 << 14)
#define BIT15 (1 << 15)
#define BIT16 (1 << 16)
#define BIT17 (1 << 17)
#define BIT18 (1 << 18)
#define BIT19 (1 << 19)
#define BIT20 (1 << 20)
#define BIT21 (1 << 21)
#define BIT22 (1 << 22)
#define BIT23 (1 << 23)
#define BIT24 (1 << 24)
#define BIT25 (1 << 25)
#define BIT26 (1 << 26)
#define BIT27 (1 << 27)
#define BIT28 (1 << 28)
#define BIT29 (1 << 29)
#define BIT30 (1 << 30)
#define BIT31 (1 << 31)

#define OLED_DC_H    (GPIOD_PDOR |= BIT2)
#define OLED_RESET_H (GPIOE_PDOR |= BIT31)

#define OLED_DC_L    (GPIOD_PDOR &= ~BIT2)
#define OLED_RESET_L (GPIOE_PDOR &= ~BIT31)

#define DDR_OLED_DC    (GPIOD_PDDR |= BIT2)
#define DDR_OLED_RESET (GPIOE_PDDR |= BIT31)

#include "derivative.h" /* include peripheral declarations */

_GLOBAL_DECL_ const unsigned char ASCII[];

void oled_spi0_init(void)
{
    unsigned short i;

    /* enable PORTD clock */
    SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; // PORTD1-3对应OLED控制
    SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK; // PTE31
    SIM_SCGC4 |= SIM_SCGC4_SPI0_MASK;

    /*portD set to GPIO*/
    PORTE_PCR31 = PORT_PCR_MUX(0X1); // PTE31:OLED RESET
    PORTD_PCR2 = PORT_PCR_MUX(0X1);  // PTD2:OLED D/C

    PORTD_PCR1 = PORT_PCR_MUX(0X2); // PTD1:OLED SPI SCK
    PORTD_PCR3 = PORT_PCR_MUX(0X5); // PTD3:OLED SPI MOSI

    // Init SPI0
    SPI0_C1 |= 0x5C; // SPOL=1,CPHA=1,LSBFE=0
    SPI0_BR = 0x10;  // Bus Clock 再2分频

    // set as output
    DDR_OLED_DC;
    DDR_OLED_RESET;

    OLED_DC_H;

    // 产生硬件RESET信号
    OLED_RESET_L;
    for (i = 0; i < 30000; i++)
    {
        asm("nop");
    }
    OLED_RESET_H;
}

// write a control word 'cw' to OLED 'cs'(CSA/CSB)
void oled_w_ctl(unsigned char cw)
{
    unsigned char i;

    OLED_DC_L; // write ctl

    while ((SPI0_S & 0x20) == 0)
        ; // SPTEF发送不为空

    SPI0_D = cw;

    while ((SPI0_S & 0x80) == 0)
        ; // SPRF接收未满

    i = SPI0_D;

    OLED_DC_H;
}

void oled_init(void) // 系统初始化调用这个
{
    oled_spi0_init();

    if (OLED_TYPE == SSD1325)
    {
        oled_w_ctl(0xae); // OLED off
        oled_w_ctl(0xb3); // Clock div
        oled_w_ctl(0x91);
        oled_w_ctl(0xa8); // 设置为64行
        oled_w_ctl(0x3f);
        oled_w_ctl(0xa2); // offset = 76？
        oled_w_ctl(0x4C);
        oled_w_ctl(0xa1); // 从0开始
        oled_w_ctl(0x00);
        oled_w_ctl(0xa0); // remap，A[6]=1 A[4]=1
        oled_w_ctl(0x50);
        oled_w_ctl(0xad); // 启用外来VCC
        oled_w_ctl(0x02);
        oled_w_ctl(0x81); // 对比度
        oled_w_ctl(0x40);
        oled_w_ctl(0x86); // Current Range
        oled_w_ctl(0xbe); // Vcomh设置
        oled_w_ctl(0x02);
        oled_w_ctl(0xbf); // Set Seg Low Voltage
        oled_w_ctl(0x0e);
        oled_w_ctl(0xa4); // Normal Mode
        oled_w_ctl(0x23); // Graphic 加速设置，启用了Rect绘制
        oled_w_ctl(0x01);
        oled_w_ctl(0x24); // 设置Rect范围
        oled_w_ctl(0x00);
        oled_w_ctl(0x00);
        oled_w_ctl(0x3f);
        oled_w_ctl(0x3f);
        oled_w_ctl(0x00);
        oled_w_ctl(0xaf); // OLED on
    }

    if (OLED_TYPE == SSD1306)
    {
        oled_w_ctl(0xae); // OLED off
        oled_w_ctl(0xd5); // Set Clock
        oled_w_ctl(0x80);
        oled_w_ctl(0xa8); // 行数
        oled_w_ctl(0x3f);
        oled_w_ctl(0xd3); // 行offset
        oled_w_ctl(0x00);
        oled_w_ctl(0x40); // Start Line
        oled_w_ctl(0x8D); // 电荷泵
        oled_w_ctl(0x14);
        oled_w_ctl(0xa1); // 设置方向 127对应Seg0
        oled_w_ctl(0xc8); // Com[n-1] -> Com0
        oled_w_ctl(0xda); // 无Left/Right Remap，Alter Com Pin，A[5]=0 A[4]=1
        oled_w_ctl(0x12);
        oled_w_ctl(0x81); // 对比度
        oled_w_ctl(0xcf);
        oled_w_ctl(0xd9); // PreCharge
        oled_w_ctl(0xf1);
        oled_w_ctl(0xdb); // Vcom
        oled_w_ctl(0x40);
        oled_w_ctl(0xa4); // 显示
        oled_w_ctl(0xa6); // 1对应像素on
        oled_w_ctl(0xaf); // OLED on
    }
}

// write a data word 'dw' to OLED 'cs'(CSA/CSB)
void oled_w_dat(unsigned char dw)
{
    unsigned char i;

    OLED_DC_H; // write data

    while ((SPI0_S & 0x20) == 0)
        ; // SPTEF发送不为空

    SPI0_D = dw;

    while ((SPI0_S & 0x80) == 0)
        ; // SPRF接收未满

    i = SPI0_D;

    OLED_DC_H;
}

// clear OLED screen
void oled_clr(void)
{
    unsigned char i, j;

    if (OLED_TYPE == SSD1325)
    {
        oled_w_ctl(0x75); /* set row address */
        oled_w_ctl(0x00); /* set row start address */
        oled_w_ctl(0x4f); /* set row end address */
        oled_w_ctl(0x15); /* set column address */
        oled_w_ctl(0x00); /* set column start address */
        oled_w_ctl(0x3f); /* set column end address */
        for (i = 0; i < 64; i++)
        {
            for (j = 0; j < 80; j++)
            {
                oled_w_dat(0x00);
            }
        }
    }

    if (OLED_TYPE == SSD1306)
    {
        for (i = 0; i < 8; i++)
        {
            oled_w_ctl(0xB0 + i); // 选择行
            oled_w_ctl(0x00);     // 选择列（0-127的Low）
            oled_w_ctl(0x10);     // 选择列（0-127的High）
            for (j = 0; j < 128; j++)
            {
                oled_w_dat(0x00);
            }
        }
    }
}

void oled_w_dot(unsigned char lx, unsigned char ly, char val)
{
    unsigned char temp; // i,j,

    if (OLED_TYPE == SSD1325)
    {
        // Not finished yet
    }

    if (OLED_TYPE == SSD1306)
    {
        oled_w_ctl(0xB0 + ((lx / 8) % 8));

        temp = (ly % 128);
        oled_w_ctl(0x00 + (temp & 0x0f));        // 低四位
        oled_w_ctl(0x10 + ((temp >> 4) & 0x0f)); // 高四位

        temp = lx % 8;

        oled_w_dat(0x01 << temp);
    }
}

// write a 8*8 character 'ch' to OLED at (lx,ly)
// don't set the 'ly' equal to 121-???
// don't set the 'lx' equal to 8-??
void oled_w_ch(unsigned char lx, unsigned char ly, char ch)
{
    unsigned char i, j, nChar;
    unsigned short temp;

    if (OLED_TYPE == SSD1325)
    {
        oled_w_ctl(0x75);               /* set row address */
        oled_w_ctl((lx % 8) * 8);       /* set row start address */
        oled_w_ctl((lx % 8) * 8 + 7);   /* set row end address */
        oled_w_ctl(0x15);               /* set column address */
        oled_w_ctl((ly % 128) / 2);     /* set column start address */
        oled_w_ctl((ly % 128) / 2 + 3); /* set column end address */
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 4; j++)
            {
                temp = (ch - 0x20) * 8;
                nChar = (ASCII[temp + j * 2] & (1 << i)) ? 0x0F : 0x00;
                nChar |= (ASCII[temp + j * 2 + 1] & (1 << i)) ? 0xF0 : 0x00;
                oled_w_dat(nChar);
            }
        }
    }

    if (OLED_TYPE == SSD1306)
    {
        oled_w_ctl(0xB0 + (lx % 8));
        nChar = (ly % 128);
        oled_w_ctl(0x00 + (nChar & 0x0f));        // 低四位
        oled_w_ctl(0x10 + ((nChar >> 4) & 0x0f)); // 高四位
        temp = (ch - 0x20) * 8;
        for (i = 0; i < 8; i++)
        {
            oled_w_dat(ASCII[temp + i]);
        }
    }
}

// write a string 'sch' to OLED  at (lx,ly)
void oled_w_str(unsigned char lx, unsigned char ly, char *sch)
{
    char *p2ch = sch;
    unsigned char L_y = ly;
    while (*p2ch)
    {

        oled_w_ch(lx, L_y, *p2ch++);
        L_y += 8;
    }
}

// #pragma CONST_SEG DATAS  //Tell the linker to allocate the words in ROM.
// ASCII Table from 0x20 to 7F
const unsigned char ASCII[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //' '
    0x00, 0x00, 0x00, 0x00, 0x9F, 0x00, 0x00, 0x00, //'!'
    0x00, 0x00, 0x0F, 0x03, 0x00, 0x0F, 0x03, 0x00, //'"'
    0x0c, 0x1e, 0x3f, 0x7e, 0x7e, 0x3f, 0x1e, 0x0c, //'#'//已改成心形
    0x00, 0x00, 0x66, 0x49, 0xC9, 0x33, 0x00, 0x00, //'$'
    0x00, 0x12, 0x15, 0x52, 0xA8, 0x48, 0x00, 0x00, //'%'
    0x00, 0x00, 0x60, 0x9C, 0xB2, 0xC2, 0xA2, 0x00, //'&'
    0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, //'''
    0x00, 0x00, 0x00, 0x00, 0xFC, 0x03, 0x00, 0x00, //'('
    0x00, 0x00, 0x03, 0xFC, 0x00, 0x00, 0x00, 0x00, //')'
    0x00, 0x00, 0x02, 0x1A, 0x07, 0x1A, 0x02, 0x00, //'*'
    0x00, 0x10, 0x10, 0x10, 0xFE, 0x10, 0x10, 0x10, //'+'
    0x00, 0x00, 0x00, 0xC0, 0x40, 0x00, 0x00, 0x00, //','
    0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, //'-'
    0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, //'.'
    0x00, 0x80, 0x60, 0x18, 0x06, 0x01, 0x00, 0x00, //'/'
    0x00, 0x7E, 0x81, 0x81, 0x81, 0x81, 0x7E, 0x00, //'0'
    0x00, 0x00, 0x82, 0x82, 0xFF, 0x80, 0x80, 0x00, //'1'
    0x00, 0xC2, 0xA1, 0x91, 0x89, 0xC6, 0x00, 0x00, //'2'
    0x00, 0x42, 0x81, 0x89, 0x89, 0x76, 0x00, 0x00, //'3'
    0x00, 0x10, 0x1C, 0x12, 0x91, 0xFF, 0x90, 0x00, //'4'
    0x00, 0x40, 0x8F, 0x89, 0x89, 0x89, 0x71, 0x00, //'5'
    0x00, 0x00, 0x7C, 0x8A, 0x89, 0x89, 0x71, 0x00, //'6'
    0x00, 0x03, 0x01, 0x01, 0xE1, 0x19, 0x07, 0x00, //'7'
    0x00, 0x76, 0x89, 0x89, 0x89, 0x89, 0x76, 0x00, //'8'
    0x00, 0x8E, 0x91, 0x91, 0x91, 0x51, 0x3E, 0x00, //'9'
    0x00, 0x00, 0x00, 0xCC, 0xCC, 0x00, 0x00, 0x00, //':'
    0x00, 0x00, 0x80, 0xCC, 0x4C, 0x00, 0x00, 0x00, //';'
    0x00, 0x10, 0x10, 0x28, 0x44, 0x44, 0x82, 0x00, //'<'
    0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, //'='
    0x00, 0x82, 0x44, 0x44, 0x28, 0x10, 0x10, 0x00, //'>'
    0x00, 0x00, 0x02, 0x81, 0xA1, 0x11, 0x0E, 0x00, //'?'
    0x00, 0x7e, 0x81, 0x99, 0xa5, 0xbd, 0xa1, 0x7e, //'@'
    0x00, 0x80, 0xC0, 0xBD, 0x23, 0xBC, 0xC0, 0x80, //'A'
    0x00, 0x81, 0xFF, 0x89, 0x89, 0x89, 0x76, 0x00, //'B'
    0x00, 0x3C, 0x42, 0x81, 0x81, 0x81, 0x43, 0x00, //'C'
    0x00, 0x81, 0xFF, 0x81, 0x81, 0x42, 0x3C, 0x00, //'D'
    0x00, 0x81, 0xFF, 0x89, 0x9D, 0x81, 0xC3, 0x00, //'E'
    0x00, 0x81, 0xFF, 0x89, 0x1D, 0x01, 0x03, 0x00, //'F'
    0x00, 0x3C, 0x42, 0x81, 0x81, 0x91, 0x73, 0x10, //'G'
    0x00, 0x81, 0xFF, 0x89, 0x08, 0x89, 0xFF, 0x81, //'H'
    0x00, 0x00, 0x81, 0x81, 0xFF, 0x81, 0x81, 0x00, //'I'
    0x00, 0x70, 0x80, 0x81, 0x81, 0x7F, 0x01, 0x00, //'J'
    0x00, 0x81, 0xFF, 0x91, 0x18, 0x25, 0xC3, 0x81, //'K'
    0x00, 0x81, 0xFF, 0x81, 0x80, 0x80, 0xE0, 0x00, //'L'
    0x00, 0x81, 0xFF, 0x87, 0x18, 0x87, 0xFF, 0x81, //'M'
    0x81, 0xFF, 0x83, 0x0C, 0x30, 0xC1, 0xFF, 0x01, //'N'
    0x00, 0x3C, 0x42, 0x81, 0x81, 0x81, 0x42, 0x3C, //'O'
    0x00, 0x81, 0xFF, 0x91, 0x11, 0x11, 0x0E, 0x00, //'P'
    0x3C, 0x42, 0x81, 0xA1, 0xC1, 0x42, 0x3C, 0xC0, //'Q'
    0x00, 0x81, 0xFF, 0x91, 0x11, 0x31, 0x4E, 0x80, //'R'
    0x00, 0xC6, 0x49, 0x89, 0x89, 0x8A, 0x73, 0x00, //'S'
    0x00, 0x03, 0x01, 0x81, 0xFF, 0x81, 0x01, 0x03, //'T'
    0x00, 0x01, 0x7F, 0x81, 0x80, 0x81, 0x7F, 0x01, //'U'
    0x01, 0x07, 0x39, 0xC0, 0xC0, 0x39, 0x07, 0x01, //'V'
    0x00, 0x01, 0x7F, 0x81, 0x78, 0x81, 0x7F, 0x01, //'W'
    0x00, 0x81, 0xC3, 0xA5, 0x18, 0xA5, 0xC3, 0x81, //'X'
    0x00, 0x01, 0x03, 0x8D, 0xF0, 0x8D, 0x03, 0x01, //'Y'
    0x00, 0x00, 0xC3, 0xA1, 0x99, 0x85, 0xC3, 0x00, //'Z'
    0x00, 0x00, 0x00, 0xFF, 0x01, 0x01, 0x00, 0x00, //'['
    0x00, 0x01, 0x06, 0x38, 0xC0, 0x00, 0x00, 0x00, //'\'
    0x00, 0x01, 0x01, 0xFF, 0x00, 0x00, 0x00, 0x00, //']'
    0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 0x00, //'^'
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //'_'
    0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, //'`'
    0x00, 0x68, 0x94, 0x94, 0x94, 0x54, 0xF8, 0x80, //'a'
    0x81, 0xFF, 0x48, 0x84, 0x84, 0x84, 0x78, 0x00, //'b'
    0x00, 0x78, 0x84, 0x84, 0x84, 0x88, 0x4C, 0x00, //'c'
    0x00, 0x78, 0x84, 0x84, 0x84, 0x49, 0xFF, 0x80, //'d'
    0x00, 0x78, 0x94, 0x94, 0x94, 0x94, 0x98, 0x00, //'e'
    0x00, 0x84, 0x84, 0xFE, 0x85, 0x85, 0x85, 0x00, //'f'
    0x00, 0x1e, 0xa1, 0xa1, 0xa1, 0x92, 0x7f, 0x01, //'g'
    0x00, 0x81, 0xFF, 0x88, 0x04, 0x84, 0xF8, 0x80, //'h'
    0x00, 0x00, 0x84, 0x84, 0xFD, 0x80, 0x80, 0x00, //'i'
    0x00, 0x80, 0x84, 0x84, 0x85, 0x7C, 0x00, 0x00, //'j'
    0x00, 0x81, 0xFF, 0x10, 0x34, 0xCC, 0x84, 0x84, //'k'
    0x00, 0x00, 0x80, 0x81, 0xFF, 0x80, 0x80, 0x00, //'l'
    0x84, 0xFC, 0x88, 0x04, 0xF8, 0x88, 0x04, 0xF8, //'m'
    0x00, 0x84, 0xFC, 0x88, 0x04, 0x84, 0xF8, 0x80, //'n'
    0x00, 0x78, 0x84, 0x84, 0x84, 0x84, 0x78, 0x00, //'o'
    0x00, 0x04, 0xFC, 0x88, 0x84, 0x84, 0x78, 0x00, //'p'
    0x00, 0x78, 0x84, 0x84, 0x84, 0x48, 0xFC, 0x04, //'q'
    0x00, 0x84, 0xFC, 0x88, 0x84, 0x84, 0x04, 0x00, //'r'
    0x00, 0xC8, 0x94, 0x94, 0x94, 0x94, 0x6C, 0x00, //'s'
    0x00, 0x04, 0x7E, 0x84, 0x84, 0x84, 0x40, 0x00, //'t'
    0x00, 0x04, 0x7C, 0x80, 0x80, 0x44, 0xFC, 0x80, //'u'
    0x04, 0x0C, 0x34, 0xC0, 0xC0, 0x34, 0x0C, 0x04, //'v'
    0x00, 0x04, 0x7C, 0x84, 0x70, 0x84, 0x7C, 0x04, //'w'
    0x00, 0x84, 0xCC, 0x30, 0x30, 0xCC, 0x84, 0x00, //'x'
    0x00, 0x01, 0x87, 0x99, 0xe0, 0x19, 0x07, 0x01, //'y'
    0x00, 0x00, 0xCC, 0xA4, 0x94, 0x8C, 0xC4, 0x00, //'z'
    0x00, 0x00, 0x10, 0xEE, 0x01, 0x00, 0x00, 0x00, //'{'
    0x00, 0x00, 0x7E, 0x42, 0x42, 0x7E, 0x00, 0x00, //'|'
    0x00, 0x00, 0x01, 0xEE, 0x10, 0x00, 0x00, 0x00, //'}'
    0x00, 0x10, 0x08, 0x08, 0x10, 0x10, 0x08, 0x00, //'~'
    0x01, 0x3D, 0x42, 0x81, 0x81, 0x81, 0x43, 0x00, //'DEL'
};

#endif /* OLED_H_ */