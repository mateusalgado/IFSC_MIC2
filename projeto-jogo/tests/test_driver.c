#include "st7735.h"
#include <assert.h>
#include <stdio.h>

SPI_HandleTypeDef hspi1;
static unsigned dc,cs=1,command,calls,pixel_count,cursor;
static unsigned left,right,top,bottom;
static bool fail_next;
static uint16_t screen[128][160];

void HAL_Delay(uint32_t ms) { (void)ms; }
void HAL_GPIO_WritePin(int port,int pin,int value)
{
    (void)port;
    if (pin==DC_Pin) dc=(unsigned)value;
    if (pin==CS_Pin) cs=(unsigned)value;
}
HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *spi,uint8_t *data,uint16_t size,uint32_t timeout)
{
    (void)spi;
    assert(!cs && timeout==100 && size>0);
    ++calls;
    if (fail_next) { fail_next=false; return HAL_ERROR; }
    if (!dc) {
        assert(size==1);command=data[0];
        if (command==ST7735_RAMWR) { cursor=0; assert(left<=right && top<=bottom && right<160 && bottom<128); }
    } else if (command==ST7735_CASET) {
        assert(size==4);left=data[0]*256U+data[1];right=data[2]*256U+data[3];
    } else if (command==ST7735_RASET) {
        assert(size==4);top=data[0]*256U+data[1];bottom=data[2]*256U+data[3];
    } else if (command==ST7735_RAMWR) {
        assert(size%2==0);
        for (unsigned i=0;i<size;i+=2) {
            unsigned x=left+cursor%(right-left+1),y=top+cursor/(right-left+1);
            assert(x<=right && y<=bottom);
            screen[y][x]=(uint16_t)(data[i]*256U+data[i+1]);++cursor;++pixel_count;
        }
    }
    return HAL_OK;
}

int main(void)
{
    ST7735_Init();assert(!ST7735_HasError());
    ST7735_FillScreen(0x0843);assert(pixel_count==160*128 && screen[127][159]==0x0843);
    ST7735_DrawPixel(159,127,0x07e0);assert(screen[127][159]==0x07e0 && right==159 && bottom==127);
    unsigned before=calls;
    ST7735_FillRectangle(0,0,0,1,0);ST7735_FillRectangle(0,0,1,0,0);
    ST7735_DrawImage(0,0,0,1,NULL);ST7735_DrawPixel(160,128,0);
    assert(calls==before);
    ST7735_FillRectangle(158,126,100,100,0xf800);
    assert(screen[127][159]==0xf800 && screen[126][158]==0xf800);
    uint16_t strip[160*8];
    for (unsigned i=0;i<160*8;++i) strip[i]=0xe007;
    ST7735_DrawImage(0,120,160,8,strip);assert(screen[127][159]==0x07e0);
    ST7735_WriteString(0,0,"M01 v0.1",Font_7x10,0xffff,0);
    before=calls;ST7735_WriteChar(159,127,'A',Font_7x10,0xffff,0);assert(calls==before);
    fail_next=true;ST7735_FillRectangle(0,0,1,1,0);
    assert(ST7735_HasError());before=calls;
    ST7735_FillRectangle(0,0,100,100,0);assert(calls==before);
    ST7735_Init();assert(!ST7735_HasError());
    puts("Real ST7735 driver passed: edge pixels, clipping, empty regions, byte order, bounded SPI errors.");
    return 0;
}
