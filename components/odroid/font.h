#pragma once

#include "esp_system.h"
//#include "typedefs.h"

 typedef struct fontCharInfo{
        uint16_t fontType;         // font type
        uint16_t firstChar;        // ASCII code of first character
        uint16_t lastChar;         // ASCII code of last character
        uint16_t maxWidth;         // maximum character width
        uint16_t kernMax;          // negative of maximum character kern
        uint16_t nDescent;         // negative of descent
        uint16_t fRectWidth;       // width of font rectangle
        uint16_t fRectHeight;      // height of font rectangle
        uint16_t owTLoc;           // offset to offset/width table
        uint16_t ascent;           // ascent
        uint16_t descent;          // descent
        uint16_t leading;          // leading
        uint16_t rowWords;         // row width of bit image / 2
		uint16_t width;
};


// Font structures for newer Adafruit_GFX (1.1 and later).
// Example fonts are included in 'Fonts' directory.
// To use a font in your Arduino sketch, #include the corresponding .h
// file and pass address of GFXfont struct to setFont().  Pass NULL to
// revert to 'classic' fixed-space bitmap font.

#ifndef _GFXFONT_H_
#define _GFXFONT_H_

/// Font data stored PER GLYPH
typedef struct {
	uint16_t bitmapOffset;     ///< Pointer into GFXfont->bitmap
	uint8_t  width;            ///< Bitmap dimensions in pixels
        uint8_t  height;           ///< Bitmap dimensions in pixels
	uint8_t  xAdvance;         ///< Distance to advance cursor (x axis)
	int8_t   xOffset;          ///< X dist from cursor pos to UL corner
        int8_t   yOffset;          ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct { 
	uint8_t  *bitmap;      ///< Glyph bitmaps, concatenated
	GFXglyph *glyph;       ///< Glyph array
	uint8_t   first;       ///< ASCII extents (first char)
        uint8_t   last;        ///< ASCII extents (last char)
	uint8_t   yAdvance;    ///< Newline distance (y axis)
} GFXfont;

#endif // _GFXFONT_H_

typedef struct fontCharInfo FontCharInfoType;
typedef FontCharInfoType* FontPtr;

void draw_text(uint16_t x, uint16_t y, const char* message, uint16_t count, uint16_t color);
void drawGlyph(int16_t x, int16_t y, GFXglyph *glyph, uint8_t  *bitmap, uint16_t color);

uint16_t FntWordWrap(const char* message, uint16_t width);
uint16_t FntLineHeight();
uint16_t FntLineWidth(const char* message, uint16_t count);

void set_font(GFXfont* font);
void set_drawbuffer(uint16_t* buffer,  uint16_t width, uint16_t height);