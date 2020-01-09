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
 
#include "backends/platform/esp32/osystem.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/events/default/default-events.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace _Esp32 {

	OSystem_Esp32::OSystem_Esp32(){
		//chdir("sdmc:/");
		
		printf("OSystem_Esp32::OSystem_Esp32(1)\r\n");
		_fsFactory = new POSIXFilesystemFactory();
		//Posix::assureDirectoryExists("/3ds/scummvm/saves/");
		printf("OSystem_Esp32::OSystem_Esp32(2)\r\n");
	}
	
	
	OSystem_Esp32::~OSystem_Esp32(){}
	
	void OSystem_Esp32::initBackend() {
		printf("OSystem_Esp32::initBackend(1)\n");
		loadConfig();
		ConfMan.registerDefault("fullscreen", true);
		ConfMan.registerDefault("aspect_ratio", true);
		/*
		if (!ConfMan.hasKey("vkeybd_pack_name"))
			ConfMan.set("vkeybd_pack_name", "vkeybd_small");
		if (!ConfMan.hasKey("vkeybdpath"))
			ConfMan.set("vkeybdpath", "/3ds/scummvm/kb");
		if (!ConfMan.hasKey("themepath"))
			ConfMan.set("themepath", "/3ds/scummvm");
		if (!ConfMan.hasKey("gui_theme"))
			ConfMan.set("gui_theme", "builtin");
		*/
		_timerManager = new DefaultTimerManager();
		gettimeofday(&_startTime, NULL);
		
		//_savefileManager = new DefaultSaveFileManager("/sd/roms/scummvm/saves");
		printf("savepath: %s\n",savedir);
		_savefileManager = new DefaultSaveFileManager(savedir);

		initGraphics();
		initAudio();
		initEvents();
		EventsBaseBackend::initBackend();
	}
	
	
	bool OSystem_Esp32::hasFeature(OSystem::Feature f){return 1;}
	void OSystem_Esp32::setFeatureState(OSystem::Feature f, bool enable){}
	bool OSystem_Esp32::getFeatureState(OSystem::Feature f){return 1;}

	
	
	uint32 lastMilis = 0;
	
	uint32 OSystem_Esp32::getMillis(bool skipRecord) {
		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);
		return (uint32)(((currentTime.tv_sec - _startTime.tv_sec) * 1000) +
						((currentTime.tv_usec - _startTime.tv_usec) / 1000));// - _timeSuspended;
	}
	
	void OSystem_Esp32::setSaveDir(char* dir){
		savedir = dir;
	}

	
	void OSystem_Esp32::delayMillis(uint msecs) {
		vTaskDelay( msecs / portTICK_PERIOD_MS );
		//usleep(msecs * 1000);
		// ToDo: Use ets_delay_us() (defined in rom/ets_sys.h) for small amouts / busy-wait for a correct number of microseconds...
		
		
	}
	void OSystem_Esp32::getTimeAndDate(TimeDate& td) const {
		time_t curTime = time(0);
		struct tm t = *localtime(&curTime);
		td.tm_sec = t.tm_sec;
		td.tm_min = t.tm_min;
		td.tm_hour = t.tm_hour;
		td.tm_mday = t.tm_mday;
		td.tm_mon = t.tm_mon;
		td.tm_year = t.tm_year;
		td.tm_wday = t.tm_wday;
	}
	OpaqueMutex* OSystem_Esp32::createMutex() {return NULL;}
	void OSystem_Esp32::lockMutex(OSystem::MutexRef) {}
	void OSystem_Esp32::unlockMutex(OSystem::MutexRef) {}
	void OSystem_Esp32::deleteMutex(OSystem::MutexRef) {}
	Audio::Mixer* OSystem_Esp32::getMixer() {return _mixer;}
	void OSystem_Esp32::quit() {}
	void OSystem_Esp32::logMessage(LogMessageType::Type, const char*) {}
	void logMessage(LogMessageType::Type type, const char *message) {}

	
	#define KEY_START 1
	#define KEY_SELECT 2
	#define KEY_A 4
	#define KEY_B 8
	#define KEY_UP 16
	#define KEY_DOWN 32
	#define KEY_LEFT 64
	#define KEY_RIGHT 128 
	#define KEY_MENU 256 
	#define KEY_VOLUME 512 	
	uint16_t lastKeystate = 0;
	
	int ax = 0;
	int ay = 0;
	
	void OSystem_Esp32::receiveKeyState(uint16_t keystate){
		Common::Event event;		
		//printf("receiveKeyState(1):%d\n",keystate);
		

		if(!(lastKeystate & KEY_START)&&(keystate&KEY_START)){
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = Common::ASCII_ESCAPE;
			event.kbd.flags = 0;
			//pushEventQueue(_eventQueue, event);	
			printf("receiveKeyState(2):Send keydown / ESC\n");
			_eventQueue.push(event);
		}
		
		if((lastKeystate & KEY_START)&&!(keystate&KEY_START)){
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = Common::ASCII_ESCAPE;
			event.kbd.flags = 0;
			printf("receiveKeyState(3):Send keyup / ESC\n");
			//pushEventQueue(_eventQueue, event);	
			_eventQueue.push(event);
		}
		
		if(!(lastKeystate & KEY_MENU)&&(keystate&KEY_MENU)){
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_1;
			event.kbd.ascii = '1';
			if(keystate&KEY_SELECT){
				event.kbd.flags = Common::KBD_CTRL;
			}else{
				event.kbd.flags = Common::KBD_ALT;
			}
			//pushEventQueue(_eventQueue, event);	
			printf("receiveKeyState(4):Send keydown / ALT+1\n");
			_eventQueue.push(event);
		}
		
		if((lastKeystate & KEY_MENU)&&!(keystate&KEY_MENU)){
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_1;
			event.kbd.ascii = '1';
			if(keystate&KEY_SELECT){
				event.kbd.flags = Common::KBD_CTRL;
			}else{
				event.kbd.flags = Common::KBD_ALT;
			}
			printf("receiveKeyState(5):Send keyup / ALT+1\n");
			//pushEventQueue(_eventQueue, event);	
			_eventQueue.push(event);
		}
	
		

		if(!(lastKeystate & KEY_A)&&(keystate&KEY_A)){
			event.type = Common::EVENT_LBUTTONDOWN;
			event.mouse.x = _cursorX;
			event.mouse.y = _cursorY;
			_eventQueue.push(event);
		}

		if((lastKeystate & KEY_A)&&!(keystate&KEY_A)){
			event.type = Common::EVENT_LBUTTONUP;
			event.mouse.x = _cursorX;
			event.mouse.y = _cursorY;
			_eventQueue.push(event);
		}

		if(!(lastKeystate & KEY_B)&&(keystate&KEY_B)){
			event.type = Common::EVENT_RBUTTONDOWN;
			event.mouse.x = _cursorX;
			event.mouse.y = _cursorY;
			_eventQueue.push(event);
		}

		if((lastKeystate & KEY_B)&&!(keystate&KEY_B)){
			event.type = Common::EVENT_RBUTTONUP;
			event.mouse.x = _cursorX;
			event.mouse.y = _cursorY;
			_eventQueue.push(event);
		}
		
		if(!(keystate&(KEY_RIGHT|KEY_LEFT))){
			ax = 0;
		}else{
			if(keystate&KEY_RIGHT){
				ax++;
			}else if(keystate&KEY_LEFT){
				ax--;
			}
			_gameDirty = true;
		}
		
		_cursorX += ax;
		if(_cursorX<0)_cursorX=0;
		if(_cursorX>319)_cursorX=319;

		if(!(keystate&(KEY_UP|KEY_DOWN))){
			ay = 0;
		}else{
			if(keystate&KEY_UP){
				ay--;
			}else if(keystate&KEY_DOWN){
				ay++;
			}
			_gameDirty = true;
		}
		
		_cursorY += ay;
		if(_cursorY<0)_cursorY=0;
		if(_cursorY>199)_cursorX=199;

		
		lastKeystate = keystate;
	}

	/*
	static void pushEventQueue(Common::Queue<Common::Event> *queue, Common::Event &event) {
		Common::StackLock lock(*eventMutex);
		queue->push(event);
	}*/
	
	int _lastCursorX = 0;
	int _lastCursorY = 0;
	
	bool OSystem_Esp32::pollEvent(Common::Event &event) {
		
		/*
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = Common::ASCII_ESCAPE;
			event.kbd.flags = 0;
			pushEventQueue(eventQueue, event);	
			*/
	
	if((_lastCursorX!=_cursorX)||(_lastCursorY!=_cursorY)){
		Common::Event event;				
		event.type = Common::EVENT_MOUSEMOVE;
		event.mouse.x = _cursorX;
		event.mouse.y = _cursorY;
		_lastCursorX=_cursorX;
		_lastCursorY=_cursorY;
		_eventQueue.push(event);
	}
	
	if (_eventQueue.empty())
	return false;

	printf("OSystem_Esp32::pollEvent (1): Send Event\n");
	event = _eventQueue.pop();
	printf("OSystem_Esp32::pollEvent (2): Send Event\n");
	return true;
		
	}
	void OSystem_Esp32::resetGraphicsScale(){}
	void OSystem_Esp32::setFocusRectangle(const Common::Rect &rect){}
	void OSystem_Esp32::clearFocusRectangle(){}
	int16 OSystem_Esp32::getOverlayHeight() {return 0;}
	int16 OSystem_Esp32::getOverlayWidth() {return 0;}
	Common::String OSystem_Esp32::getSystemLanguage() const {	return "en_US";}
	void OSystem_Esp32::initAudio(){
		printf("OSystem_Esp32::initAudio(1)\r\n");
		
		_mixer = new Audio::MixerImpl(this, 22050);
		printf("OSystem_Esp32::initAudio(2)\r\n");

		hasAudio = false;// R_SUCCEEDED(ndspInit());
		printf("OSystem_Esp32::initAudio(3)\r\n");
		_mixer->setReady(false);
		printf("OSystem_Esp32::initAudio(4)\r\n");

		if (hasAudio) {
//			s32 prio = 0;
//			svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
//			audioThread = threadCreate(&audioThreadFunc, _mixer, 32 * 1048, prio - 1, -2, false);
		}
	}
	void OSystem_Esp32::updateConfig() {
		/*
		if (_gameScreen.getPixels()) {
			updateSize();
			warpMouse(_cursorX, _cursorY);
		}*/
	}
	void OSystem_Esp32::destroyAudio(){
		if (hasAudio) {
			//threadJoin(audioThread, U64_MAX);
			//threadFree(audioThread);
			//ndspExit();
		}

		delete _mixer;
		_mixer = 0;
	}
	void OSystem_Esp32::initEvents(){}
	void OSystem_Esp32::destroyEvents(){}

	void OSystem_Esp32::flushGameScreen(){
		printf("OSystem_Esp32::flushGameScreen(1)\n");
	}
	void OSystem_Esp32::flushCursor(){}

	
} // namespace _Esp32

