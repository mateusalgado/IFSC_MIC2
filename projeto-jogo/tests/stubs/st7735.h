#ifndef TEST_ST7735_H
#define TEST_ST7735_H
#include <stdint.h>
#include <stdbool.h>
#include "fonts.h"
void ST7735_FillRectangle(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color);
void ST7735_FillScreen(uint16_t color);
void ST7735_WriteString(uint16_t x,uint16_t y,const char *str,FontDef font,uint16_t color,uint16_t bg);
void ST7735_DrawImage(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint16_t *data);
#endif
