/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PLATFORM_Esp32_H
#define PLATFORM_Esp32_H

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "backends/mutex/mutex.h"
#include "backends/base-backend.h"
#include "graphics/palette.h"
#include "base/main.h"
#include "audio/mixer_intern.h"
#include "audio/mixer.h"
#include "backends/graphics/graphics.h"
#include "components/graphics/surface.h"
//#include "backends/platform/3ds/sprite.h"
#include "common/rect.h"
#include "common/queue.h"
#include "common/events.h"

#include "backends/plugins/posix/posix-provider.h"
#include "backends/fs/posix/posix-fs-factory.h"

#include <time.h>
#include <sys/time.h>
#include "esp_system.h"

#define TICKS_PER_MSEC 268123

namespace _Esp32 {

enum {
	GFX_LINEAR = 0,
	GFX_NEAREST = 1
};

enum InputMode {
	MODE_HOVER,
	MODE_DRAG,
};

static const OSystem::GraphicsMode s_graphicsModes[] = {
	{"default", "Default Test", GFX_LINEAR},
	{ 0, 0, 0 }
};

class OSystem_Esp32 : public EventsBaseBackend, public PaletteManager {
public:

	OSystem_Esp32();
	virtual ~OSystem_Esp32();

	void (*drawImageToScreen)(uint8_t*, uint16_t*, uint16_t);
	void receiveKeyState(uint16_t lastKeystate);
	
	volatile bool exiting;
	volatile bool sleeping;

	virtual void initBackend();

	virtual bool hasFeature(OSystem::Feature f);
	virtual void setFeatureState(OSystem::Feature f, bool enable);
	virtual bool getFeatureState(OSystem::Feature f);

	virtual bool pollEvent(Common::Event &event);

	virtual uint32 getMillis(bool skipRecord = false);
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(TimeDate &t) const;

	virtual MutexRef createMutex();
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual void logMessage(LogMessageType::Type type, const char *message);
	virtual Audio::Mixer *getMixer();
	virtual PaletteManager *getPaletteManager() { return this; }
	virtual Common::String getSystemLanguage() const;
	/*
	virtual void fatalError();
	*/
	virtual void quit();
/*
	virtual Common::String getDefaultConfigFileName();
*/
	// Graphics
	
	int getDefaultGraphicsMode() const;
	bool setGraphicsMode(int mode);
	void resetGraphicsScale();
	int getGraphicsMode() const;
	inline Graphics::PixelFormat getScreenFormat() const { return _pfGame; }
	//virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	Common::List<Graphics::PixelFormat> getSupportedFormats() const  {
		Common::List<Graphics::PixelFormat> list;
		list.push_back(Graphics::PixelFormat::createFormatCLUT8());
		return list;
	}
	void initSize(uint width, uint height,
	              const Graphics::PixelFormat *format = NULL);
	/*
	virtual int getScreenChangeID() const { return 0; };

	void beginGFXTransaction();
	OSystem::TransactionError endGFXTransaction();
	*/
	int16 getHeight(){ return _gameHeight; }
	int16 getWidth(){ return _gameWidth; }
	void setPalette(const byte *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num) const override {}
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w,
	                      int h);
	Graphics::Surface *lockScreen();
	void unlockScreen();
	void updateScreen();
	void setShakePos(int shakeOffset);
	void setFocusRectangle(const Common::Rect &rect);
	void clearFocusRectangle();
	void showOverlay();
	void hideOverlay();
	Graphics::PixelFormat getOverlayFormat() const;
	void clearOverlay();
	void grabOverlay(void *buf, int pitch);
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w,
	                       int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	/*
	virtual void displayMessageOnOSD(const char *msg);
*/
	bool showMouse(bool visible);
	void warpMouse(int x, int y);
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX,
	                    int hotspotY, uint32 keycolor, bool dontScale = false,
	                    const Graphics::PixelFormat *format = NULL);
	/*
	void setCursorPalette(const byte *colors, uint start, uint num);

	// Transform point from touchscreen coords into gamescreen coords
	void transformPoint(touchPosition &point);

	void setCursorDelta(float deltaX, float deltaY);

	void updateFocus();*/
	void updateConfig();
//	void updateSize();

private:
	void initGraphics();
	void destroyGraphics();
	void initAudio();
	void destroyAudio();
	void initEvents();
	void destroyEvents();

	void flushGameScreen();
	void flushCursor();
	 
	
	
protected:
	Audio::MixerImpl *_mixer;
	//Audio::Mixer *_mixer;
private:
	uint16 _gameWidth, _gameHeight;

	struct timeval _startTime;
	
	/*


	u16 _gameTopX, _gameTopY;
	u16 _gameBottomX, _gameBottomY;
*/
	// Audio
	bool hasAudio;
/*	
	Thread audioThread;
*/
	// Graphics
	Graphics::PixelFormat _pfGame;
	
	Graphics::PixelFormat _pfGameTexture;
	Graphics::PixelFormat _pfCursor;
	byte _palette[3 * 256];
	byte _cursorPalette[3 * 256];
	byte *_gamePixels;
	Graphics::Surface _surface;
	bool _gameDirty = false; 
	
	/*
	Sprite _gameTopTexture;
	Sprite _gameBottomTexture;
	Sprite _overlay;
	*/

	int _screenShakeOffset;
	bool _overlayVisible;
	
	/*

	DVLB_s *_dvlb;
	shaderProgram_s _program;
	int _projectionLocation;
	int _modelviewLocation;
	C3D_Mtx _projectionTop;
	C3D_Mtx _projectionBottom;
	C3D_RenderTarget* _renderTargetTop;
	C3D_RenderTarget* _renderTargetBottom;

	// Focus
	Common::Rect _focusRect;
	bool _focusDirty;
	C3D_Mtx _focusMatrix;
	int _focusPosX, _focusPosY;
	int _focusTargetPosX, _focusTargetPosY;
	float _focusStepPosX, _focusStepPosY;
	float _focusScaleX, _focusScaleY;
	float _focusTargetScaleX, _focusTargetScaleY;
	float _focusStepScaleX, _focusStepScaleY;
	uint32 _focusClearTime;

	// Events
	Thread _eventThread;
	Thread _timerThread;
	*/
	Common::Queue<Common::Event> _eventQueue;
/*
	// Cursor
	Graphics::Surface _cursor;
	Sprite _cursorTexture;
	bool _cursorPaletteEnabled;
	bool _cursorVisible;
	bool _cursorScalable;
	float _cursorX, _cursorY;
	float _cursorDeltaX, _cursorDeltaY;
	int _cursorHotspotX, _cursorHotspotY;
	uint32 _cursorKeyColor;
	*/
	
};

} // namespace _Esp32

#endif
