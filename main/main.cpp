#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "driver/i2s.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_task_wdt.h"
#include "esp_spiffs.h"
#include "driver/rtc_io.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include <string.h>

//#include "../components/ugui/ugui.h"
//#include "../components/scumm/scummVM.h"

#include "engines/engine.h"
#include "engines/metaengine.h"
#include "base/commandLine.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/language.h"
#include "scumm/detection.h"
#include "scumm/detection_tables.h"
#include "scumm/scumm.h"
#include "scumm/scummmetaengine.h"
#include "scumm/scumm_v5.h"

#include "cine/cinemetaengine.h"


#include "common/config-manager.h"
//namespace Scumm {
	
#include "backends/platform/esp32/osystem.h"

#define SD_BASE_PATH "/sd"
	
//OSystem *g_system;
	
/*
static bool launcherDialog() {

	// Discard any command line options. Those that affect the graphics
	// mode and the others (like bootparam etc.) should not
	// blindly be passed to the first game launched from the launcher.
	ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();

#if defined(_WIN32_WCE)
	CELauncherDialog dlg;
#elif defined(__DC__)
	DCLauncherDialog dlg;
#else
	//GUI::LauncherDialog dlg;
#endif
	//return (dlg.runModal() != -1);
	return true;
}
*/

/*
static const Plugin *detectPlugin() {
	const Plugin *plugin = nullptr;

	// Make sure the gameid is set in the config manager, and that it is lowercase.
	Common::String gameid(ConfMan.getActiveDomainName());
	assert(!gameid.empty());
	if (ConfMan.hasKey("gameid")) {
		gameid = ConfMan.get("gameid");

		// Set last selected game, that the game will be highlighted
		// on RTL
		ConfMan.set("lastselectedgame", ConfMan.getActiveDomainName(), Common::ConfigManager::kApplicationDomain);
		ConfMan.flushToDisk();
	}

	gameid.toLowercase();
	ConfMan.set("gameid", gameid);

	// Query the plugins and find one that will handle the specified gameid
	printf("User picked target '%s' (gameid '%s')...\n", ConfMan.getActiveDomainName().c_str(), gameid.c_str());
	printf("  Looking for a plugin supporting this gameid... ");

	PlainGameDescriptor game = EngineMan.findGame(gameid, &plugin);

	if (plugin == 0) {
		printf("failed\n");
		warning("%s is an invalid gameid. Use the --list-games option to list supported gameid", gameid.c_str());
	} else {
		printf("%s\n  Starting '%s'\n", plugin->getName(), game.description);
	}

	return plugin;
	
}*/

// TODO: specify the possible return values here
/*static Common::Error runGame(const Plugin *plugin, OSystem &system, const Common::String &edebuglevels) {
	
	// Determine the game data path, for validation and error messages
	Common::FSNode dir(ConfMan.get("path"));
	Common::Error err = Common::kNoError;
	Engine *engine = 0;

#if defined(SDL_BACKEND) && defined(USE_OPENGL) && defined(USE_RGB_COLOR)
	// HACK: We set up the requested graphics mode setting here to allow the
	// backend to switch from Surface SDL to OpenGL if necessary. This is
	// needed because otherwise the g_system->getSupportedFormats might return
	// bad values.
	g_system->beginGFXTransaction();
		g_system->setGraphicsMode(ConfMan.get("gfx_mode").c_str());
	if (g_system->endGFXTransaction() != OSystem::kTransactionSuccess) {
		warning("Switching graphics mode to '%s' failed", ConfMan.get("gfx_mode").c_str());
		return Common::kUnknownError;
	}
#endif

	// Verify that the game path refers to an actual directory
	if (!dir.exists()) {
		err = Common::kPathDoesNotExist;
	} else if (!dir.isDirectory()) {
		err = Common::kPathNotDirectory;
	}

	// Create the game engine
	if (err.getCode() == Common::kNoError) {
		const MetaEngine &metaEngine = plugin->get<MetaEngine>();
		// Set default values for all of the custom engine options
		// Apparently some engines query them in their constructor, thus we
		// need to set this up before instance creation.
		const ExtraGuiOptions engineOptions = metaEngine.getExtraGuiOptions(Common::String());
		for (uint i = 0; i < engineOptions.size(); i++) {
			ConfMan.registerDefault(engineOptions[i].configOption, engineOptions[i].defaultState);
		}

		err = metaEngine.createInstance(&system, &engine);
	}

	// Check for errors
	if (!engine || err.getCode() != Common::kNoError) {

		// Print a warning; note that scummvm_main will also
		// display an error dialog, so we don't have to do this here.
		warning("%s failed to instantiate engine: %s (target '%s', path '%s')",
			plugin->getName(),
			err.getDesc().c_str(),
			ConfMan.getActiveDomainName().c_str(),
			dir.getPath().c_str()
			);

		// If a temporary target failed to launch, remove it from the configuration manager
		// so it not visible in the launcher.
		// Temporary targets are created when starting games from the command line using the game id.
		if (ConfMan.hasKey("id_came_from_command_line")) {
			ConfMan.removeGameDomain(ConfMan.getActiveDomainName().c_str());
		}

		return err;
	}

	// Set the window caption to the game name
	Common::String caption(ConfMan.get("description"));

	if (caption.empty()) {
		PlainGameDescriptor game = EngineMan.findGame(ConfMan.get("gameid"));
		if (game.description) {
			caption = game.description;
		}
	}
	if (caption.empty())
		caption = ConfMan.getActiveDomainName(); // Use the domain (=target) name
	if (!caption.empty())	{
		system.setWindowCaption(caption.c_str());
	}

	//
	// Setup various paths in the SearchManager
	//

	// Add the game path to the directory search list
	engine->initializePath(dir);

	// Add extrapath (if any) to the directory search list
	if (ConfMan.hasKey("extrapath")) {
		dir = Common::FSNode(ConfMan.get("extrapath"));
		SearchMan.addDirectory(dir.getPath(), dir);
	}

	// If a second extrapath is specified on the app domain level, add that as well.
	// However, since the default hasKey() and get() check the app domain level,
	// verify that it's not already there before adding it. The search manager will
	// check for that too, so this check is mostly to avoid a warning message.
	if (ConfMan.hasKey("extrapath", Common::ConfigManager::kApplicationDomain)) {
		Common::String extraPath = ConfMan.get("extrapath", Common::ConfigManager::kApplicationDomain);
		if (!SearchMan.hasArchive(extraPath)) {
			dir = Common::FSNode(extraPath);
			SearchMan.addDirectory(dir.getPath(), dir);
		}
	}

	// On creation the engine should have set up all debug levels so we can use
	// the command line arguments here
	Common::StringTokenizer tokenizer(edebuglevels, " ,");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token.equalsIgnoreCase("all"))
			DebugMan.enableAllDebugChannels();
		else if (!DebugMan.enableDebugChannel(token))
			warning(_("Engine does not support debug level '%s'"), token.c_str());
	}

#ifdef USE_TRANSLATION
	Common::String previousLanguage = TransMan.getCurrentLanguage();
	if (ConfMan.hasKey("gui_use_game_language")
	    && ConfMan.getBool("gui_use_game_language")
	    && ConfMan.hasKey("language")) {
		TransMan.setLanguage(ConfMan.get("language"));
#ifdef USE_TTS
		Common::TextToSpeechManager *ttsMan;
		if ((ttsMan = g_system->getTextToSpeechManager()) != nullptr) {
			Common::String language = ConfMan.get("language");
			language.setChar(2, '\0');
			ttsMan->setLanguage(language);
		}
#endif // USE_TTS
	}
#endif // USE_TRANSLATION

	// Initialize any game-specific keymaps
	engine->initKeymap();

	// Inform backend that the engine is about to be run
	system.engineInit();

	// Run the engine
	Common::Error result = engine->run();

	// Inform backend that the engine finished
	system.engineDone();

	// Clean up any game-specific keymaps
	engine->deinitKeymap();

	// Free up memory
	delete engine;

	// We clear all debug levels again even though the engine should do it
	DebugMan.clearAllDebugChannels();

	// Reset the file/directory mappings
	SearchMan.clear();

#ifdef USE_TRANSLATION
	TransMan.setLanguage(previousLanguage);
#ifdef USE_TTS
		Common::TextToSpeechManager *ttsMan;
		if ((ttsMan = g_system->getTextToSpeechManager()) != nullptr) {
			Common::String language = ConfMan.get("language");
			language.setChar(2, '\0');
			ttsMan->setLanguage(language);
		}
#endif // USE_TTS
#endif // USE_TRANSLATION

	// Return result (== 0 means no error)
	return result;
}
	*/

static void setupGraphics(OSystem &system) {
/*
	system.beginGFXTransaction();
		// Set the user specified graphics mode (if any).
		system.setGraphicsMode(ConfMan.get("gfx_mode").c_str());

		system.initSize(320, 200);

		if (ConfMan.hasKey("aspect_ratio"))
			system.setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio"));
		if (ConfMan.hasKey("fullscreen"))
			system.setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen"));
		if (ConfMan.hasKey("filtering"))
			system.setFeatureState(OSystem::kFeatureFilteringMode, ConfMan.getBool("filtering"));
		if (ConfMan.hasKey("stretch_mode"))
			system.setStretchMode(ConfMan.get("stretch_mode").c_str());
	system.endGFXTransaction();

	// When starting up launcher for the first time, the user might have specified
	// a --gui-theme option, to allow that option to be working, we need to initialize
	// GUI here.
	// FIXME: Find a nicer way to allow --gui-theme to be working
	GUI::GuiManager::instance();

	// Set initial window caption
	system.setWindowCaption(gScummVMFullVersion);

	// Clear the main screen
	system.fillScreen(0);*/
}

static void setupKeymapper(OSystem &system) {}
	
	extern "C" {
		#include "hourglass_empty_black_48dp.h"
		#include "../components/odroid/image_splash.h"
		#include "../components/odroid/odroid_settings.h"
		#include "../components/odroid/odroid_input.h"
		#include "../components/odroid/odroid_display.h"
		#include "../components/odroid/odroid_gui.h"
		#include "../components/odroid/odroid_audio.h"
		#include "../components/odroid/odroid_system.h"
		#include "../components/odroid/odroid_sdcard.h"

		
		//Common::Language language = Common::GR_GRE;
		char base[256]; //= "game1";//directories[selectedFolder].getName().c_str();
		char dir[256];
		char savedir[256];

		odroid_gamepad_state lastJoysticState;
		
		_Esp32::OSystem_Esp32 *esp_system;
		
		void init_system(){
			printf("init_system(1)\r\n");
			esp_system = new _Esp32::OSystem_Esp32();
			g_system = esp_system;
		}
		
		void init_scumm(){
			Common::String specialDebug;
			Common::String command;
	
			esp_system->drawImageToScreen = &ili9341_write_frame_scumm;
			esp_system->setSaveDir(savedir);
			//g_system = new _Esp32::OSystem_Esp32();
			
			// Verify that the backend has been initialized (i.e. g_system has been set).
			printf("init_scumm(2)\r\n");
			assert(g_system);
			printf("init_scumm(3)\r\n");
			OSystem &system = *g_system;
			
			// Register config manager defaults
			Base::registerDefaults();

			// Parse the command line
			Common::StringMap settings;
			

			// Load the config file
			ConfMan.loadDefaultConfigFile();
			// As we have no sound 
			ConfMan.setBool("subtitles", true);
			
			
			// Update the config file
			//ConfMan.set("versioninfo", "", Common::ConfigManager::kApplicationDomain);
			
			PluginManager::instance().init();
			PluginManager::instance().loadAllPlugins(); // load plugins for cached plugin manager

			// Init the backend. Must take place after all config data (including
			// the command line params) was read.
			system.initBackend();
			settings["gfx-mode"] = "default";
			
			setupGraphics(system);
			setupKeymapper(system);
			/*
			const Plugin *plugin = detectPlugin();
			Common::Error result = runGame(plugin, system, specialDebug);
			*/
			printf("init_scumm done.\r\n");
		}
		
		//Scumm::ScummEngine_v5 *engine;
		Engine *engine;
		
		int lastmode = 0;
		
		void readInputTask(void *arg){
			while(true){
				
				/*
				int mode = engine->get_charset_mode();
				if(mode!=lastmode){
					printf("main.cpp: Charset mode changed: %d!!!\n", mode);
					lastmode = mode;
				}*/
		
				
				odroid_gamepad_state joysticState;
				odroid_input_gamepad_read(&joysticState);

				uint16_t keyState = 0;
				//bool state_changed = false;

				//ODROID_INPUT_START	1
				//ODROID_INPUT_SELECT	2
				//ODROID_INPUT_LEFT	64
				//ODROID_INPUT_RIGHT	128
				//ODROID_INPUT_UP	16
				//ODROID_INPUT_DOWN	32
				//ODROID_INPUT_A	4
				//ODROID_INPUT_B	8
				//ODROID_INPUT_MENU 256
				//ODROID_INPUT_VOLUME 512
				
				//if(joysticState.values[ODROID_INPUT_START]!=lastJoysticState.values[ODROID_INPUT_START]){
					//state_changed = true;
					if(joysticState.values[ODROID_INPUT_START]){
						keyState|=1;
				}//}
				
				//if(joysticState.values[ODROID_INPUT_SELECT]!=lastJoysticState.values[ODROID_INPUT_SELECT]){
					//state_changed = true;
					if(joysticState.values[ODROID_INPUT_SELECT]){
						keyState|=2;
				}//}
				//if(joysticState.values[ODROID_INPUT_LEFT]!=lastJoysticState.values[ODROID_INPUT_LEFT]){
					//state_changed = true;
					if(joysticState.values[ODROID_INPUT_LEFT]){
						keyState|=64;
				}//}
				//if(joysticState.values[ODROID_INPUT_RIGHT]!=lastJoysticState.values[ODROID_INPUT_RIGHT]){
					//state_changed = true;
					if(joysticState.values[ODROID_INPUT_RIGHT]){
						keyState|=128;
				}//}
				//if(joysticState.values[ODROID_INPUT_UP]!=lastJoysticState.values[ODROID_INPUT_UP]){
					//state_changed = true;
					if(joysticState.values[ODROID_INPUT_UP]){
						keyState|=16;
				}//}
				//if(joysticState.values[ODROID_INPUT_DOWN]!=lastJoysticState.values[ODROID_INPUT_DOWN]){
					//state_changed = true;
					if(joysticState.values[ODROID_INPUT_DOWN]){
						keyState|=32;
				}//}
				//if(joysticState.values[ODROID_INPUT_A]!=lastJoysticState.values[ODROID_INPUT_A]){
					//state_changed = true;
					if(joysticState.values[ODROID_INPUT_A]){
						keyState|=4;
				}//}
				//if(joysticState.values[ODROID_INPUT_B]!=lastJoysticState.values[ODROID_INPUT_B]){
					//state_changed = true;
					if(joysticState.values[ODROID_INPUT_B]){
						keyState|=8;
				}//}
				
				
				/*
				if(joysticState.values[ODROID_INPUT_MENU]!=lastJoysticState.values[ODROID_INPUT_MENU]){
					if(joysticState.values[ODROID_INPUT_MENU]){
						printf("Save state...");
						Common::String filename = Common::String("/sd/roms/scummvm/monkey1/save1.sav");
						
						engine->saveState(1,false,filename);
						printf("State saved.");
					}
				}*/
				
				
				
				if(joysticState.values[ODROID_INPUT_MENU]){
						keyState|=256;
				}
				
				if(joysticState.values[ODROID_INPUT_VOLUME]){
						keyState|=512;
				}
				//if(state_changed){
					//esp_system->receiveKeyState(joysticState.values[ODROID_INPUT_START]?1:0);
					esp_system->receiveKeyState(keyState);
				//}
				
				
				lastJoysticState = joysticState;
				
				vTaskDelay(100 / portTICK_RATE_MS);
			}
		}

		
		
		void app_main(void) {
			printf("Scumm GO (%s-%s).\n", COMPILEDATE, GITREV);

			nvs_flash_init();
			
			odroid_system_init();
			odroid_input_gamepad_init();

			// Display
			ili9341_prepare();
			ili9341_init();
			
			// blue led
			gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
			gpio_set_level(GPIO_NUM_2, 0);

			// Charge
			odroid_input_battery_level_init();
			
			// Read Gamepad once
			odroid_input_gamepad_read(&lastJoysticState);
			
			// Show Splash-Screen
			ili9341_write_frame_scumm(image_splash, image_splash_palette, 240, NULL, 0, 0);
			
			// Prepare SD-Card
			esp_err_t r = odroid_sdcard_open(SD_BASE_PATH);
			if (r != ESP_OK)
			{
				odroid_display_show_sderr(ODROID_SD_ERR_NOCARD);
				abort();
			}
			
			// Prepare system
			init_system();
			
			// Show Folder Selection
			char const *basedir = "/sd/roms/scummvm";
			
			uint16_t c_black = 0x0000;
			
			drawString(30,180,"Select Game:",c_black);
			drawString(30,210,"Press Start to play!",c_black);
			
			int selectedFolder = 0;
			Common::FSNode basedirNode = Common::FSNode(basedir);
			Common::FSList directories;
			basedirNode.getChildren(directories, Common::FSNode::kListDirectoriesOnly);
			
			odroid_gamepad_state joysticState;
			odroid_gamepad_state lastJoysticState;
			
			odroid_input_gamepad_read(&lastJoysticState);
			
			bool keypressed = false;
			bool startPressed = false;
			
			while(!startPressed){
				Common::String gameName = Common::String("> ") + directories[selectedFolder].getName() + Common::String("                    ");
				
				drawString(30,195,gameName.c_str(),c_black);
				
				keypressed = false;
				while(!keypressed){
					odroid_input_gamepad_read(&joysticState);
					if(joysticState.values[ODROID_INPUT_RIGHT] && !lastJoysticState.values[ODROID_INPUT_RIGHT]){
						if(selectedFolder<directories.size()-1){
							selectedFolder++;
						}
						keypressed = true;
					}

					if(joysticState.values[ODROID_INPUT_LEFT] && !lastJoysticState.values[ODROID_INPUT_LEFT]){
						if(selectedFolder>0){
							selectedFolder--;
						}
						keypressed = true;
					}
					
					// Game should start if user presses Start or A...
					if((joysticState.values[ODROID_INPUT_START] && !lastJoysticState.values[ODROID_INPUT_START])||
					(joysticState.values[ODROID_INPUT_A] && !lastJoysticState.values[ODROID_INPUT_A]))
					{
						startPressed = true;
						keypressed = true;
					}
					lastJoysticState = joysticState;
				}
			}
			

			
			

			//Scumm::GameSettings settings = {"monkey", "CD",           0, Scumm::GID_MONKEY,     5, 0, MDT_ADLIB,                        Scumm::GF_AUDIOTRACKS, UNK, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)};
			
			//char *base = NULL, *dir = NULL;
			//char const *base = "loom";
			//char const *dir = "/sd/roms/scummvm/loom";
			//char base[256]; //= "game1";//directories[selectedFolder].getName().c_str();
			//char dir[256];
			//char savedir[256];
			
			sprintf(base,"%s",directories[selectedFolder].getName().c_str());
			sprintf(dir,"%s",directories[selectedFolder].getPath().c_str());
			sprintf(savedir,"%s/saves",directories[selectedFolder].getPath().c_str());
			
			//= directories[selectedFolder].getPath().c_str();
			
			
			printf("Base: %s\n",base);
			printf("Selected path: %s\n",dir);
			
			
			// Make folder for savegames
			mkdir(savedir,ACCESSPERMS);
			
			// Prepare Scum-Game
			init_scumm();
			
			
			Common::Language language = Common::UNK_LANG;
			Common::Platform platform = Common::kPlatformUnknown;

			//if (!selectGame(base, dir, language, platform, icon))
			//g_system->quit();

			// Set the game path.
			ConfMan.addGameDomain(base);
			//if (dir != NULL)
			ConfMan.set("path", dir,base);

			// Set the game language.
			ConfMan.set("language", Common::getLanguageCode(language), base);

			// Set the game platform.
			ConfMan.set("platform", Common::getPlatformCode(platform), base);

			// Set the target.
			ConfMan.setActiveDomain(base);
			
			//const Plugin *plugin = detectPlugin();
			//PluginManager::instance().unloadPluginsExcept(PLUGIN_TYPE_ENGINE, plugin);

			//Common::FSNode dirNode = Common::FSNode("/sd/roms/scummvm/monkey1");
			Common::FSNode dirNode = Common::FSNode(dir);
			Common::FSList files;
			int res = dirNode.getChildren(files, Common::FSNode::kListAll);
			printf("Files listed: %d\n", res);
			
			Cine::CineMetaEngine cmEngine;
			Scumm::ScummMetaEngine smEngine;
			
			MetaEngine *engines[] = {&smEngine,&cmEngine};
			
			MetaEngine *mEngine;// = smEngine;
			DetectedGames games;// = mEngine.detectGames(files);
			
			Common::List<Scumm::DetectorResult> results;
			
			for(int i = 0; i< 2;i++){
				mEngine = engines[i];
				games = mEngine->detectGames(files);
				if(!games.empty())break;
			}

			if(games.empty()){
				printf("No Engine found to run this game!\n");
				abort();
			}
			
			//DetectedGames games = mEngine.detectGames(files);
			//detectGames(files, results, NULL);
			printf("gameid: %s\n",games[0].gameId.c_str());
			ConfMan.set("gameid", games[0].gameId, base);
/*
			Scumm::DetectorResult dr;
			dr.language = Common::EN_ANY;
			
			dr.fp.pattern = "monkey1.%03d";
			dr.fp.genMethod = Scumm::kGenDiskNum;
			dr.game.gameid = "monkey";
			dr.game.variant = "cd";
			dr.game.id = Scumm::GID_MONKEY;
			dr.game.version = 5;
			dr.game.heversion = 0;
			dr.game.midi = MDT_ADLIB; 
			dr.game.features = Scumm::GF_AUDIOTRACKS;
			dr.game.platform = UNK;
			dr.game.guioptions = GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI);
			dr.language = Scumm::UNK_LANG;
			dr.md5 = "2d1e891fe52df707c30185e52c50cd92";
			dr.extra = 0;*/
			
			//Scumm::ScummEngine_v5 *engine = new Scumm::ScummEngine_v5(g_system,dr);
			//engine = new Scumm::ScummEngine_v5(g_system,dr);
			mEngine->createInstance(g_system,&engine);
			printf("Engine created.\n");
			// Inform backend that the engine is about to be run
			g_system->engineInit();

			// Clear screen
			ili9341_write_frame_scumm(NULL,NULL,240, NULL, 0, 0);
			
			xTaskCreatePinnedToCore(&readInputTask, "readInputTask", 4096, NULL, 5, NULL, 0);
			
			// Run the engine
			printf("Trying to run the engine...\n");
			//engine->initializePath(dir);
			//engine->initKeymap();
			Common::Error result = engine->run();
			printf("The End.\n");
			
			
			// Try to run the game
			//Common::Error result = runGame(plugin, g_system);
			
		}
		
	}

//}