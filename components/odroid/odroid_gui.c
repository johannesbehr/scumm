

#pragma GCC optimize ("O3")

#include "odroid_gui.h"
#include "odroid_display.h"

#include "esp_system.h"
#include "esp_event.h"
#include <string.h>
#include "font.h"

uint16_t buffer[11*320*2];

void drawString(uint16_t x, uint16_t y, const char* text, uint16_t color){
	
	uint16_t background = 0xffff;
	
	uint16_t width = FntLineWidth(text, 256);
	uint16_t height = FntLineHeight();
	set_drawbuffer(buffer,  width, height);
	for(int i = 0; i<width*height*2; i++){
		buffer[i] = background;
	}
	draw_text(0,height-2,text,256,color);
	ili9341_write_frame_rectangleLE(x,y,width,height,buffer);
}