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
#include "scumm/scumm_v5.h"
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
		#include "../components/odroid/odroid_audio.h"
		#include "../components/odroid/odroid_system.h"
		#include "../components/odroid/odroid_sdcard.h"

		
		//Common::Language language = Common::GR_GRE;
		
		

		odroid_gamepad_state lastJoysticState;
		
		_Esp32::OSystem_Esp32 *esp_system;
		
		void init_scumm(){
			Common::String specialDebug;
			Common::String command;
	
			printf("init_scumm(1)\r\n");
			esp_system = new _Esp32::OSystem_Esp32();
			g_system = esp_system;
			esp_system->drawImageToScreen = &ili9341_write_frame_scumm;
			
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
		
		Scumm::ScummEngine_v5 *engine;
		
		int lastmode = 0;
		
		void readInputTask(void *arg){
			while(true){
				
				
				int mode = engine->get_charset_mode();
				if(mode!=lastmode){
					printf("main.cpp: Charset mode changed: %d!!!\n", mode);
					lastmode = mode;
				}
		
				
				odroid_gamepad_state joysticState;
				odroid_input_gamepad_read(&joysticState);
				
				if(joysticState.values[ODROID_INPUT_START]!=lastJoysticState.values[ODROID_INPUT_START])
				{
					esp_system->receiveKeyState(joysticState.values[ODROID_INPUT_START]?1:0);
				}
				lastJoysticState = joysticState;
				
				vTaskDelay(100 / portTICK_RATE_MS);
			}
		}

		
		
		void app_main(void) {
			printf( "\nHello World from C++!\n" );
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
			ili9341_write_frame_scumm(image_splash, image_splash_palette, 240);
			
			// Prepare SD-Card
			esp_err_t r = odroid_sdcard_open(SD_BASE_PATH);
			if (r != ESP_OK)
			{
				odroid_display_show_sderr(ODROID_SD_ERR_NOCARD);
				abort();
			}
			
			// Prepare Scum-Game
			init_scumm();
			//Scumm::GameSettings settings = {"monkey", "CD",           0, Scumm::GID_MONKEY,     5, 0, MDT_ADLIB,                        Scumm::GF_AUDIOTRACKS, UNK, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)};
			
			//char *base = NULL, *dir = NULL;
			char const *base = "monkey";
			char const *dir = "/sd/roms/scummvm/monkey1";
			
			Common::Language language = Common::UNK_LANG;
			Common::Platform platform = Common::kPlatformUnknown;

			//if (!selectGame(base, dir, language, platform, icon))
			//g_system->quit();

			// Set the game path.
			ConfMan.addGameDomain(base);
			//if (dir != NULL)
			ConfMan.set("path", dir, base);

			// Set the game language.
			ConfMan.set("language", Common::getLanguageCode(language), base);

			// Set the game platform.
			ConfMan.set("platform", Common::getPlatformCode(platform), base);

			// Set the target.
			ConfMan.setActiveDomain(base);
			
			//const Plugin *plugin = detectPlugin();
			//PluginManager::instance().unloadPluginsExcept(PLUGIN_TYPE_ENGINE, plugin);

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
			dr.extra = 0;
			
			//Scumm::ScummEngine_v5 *engine = new Scumm::ScummEngine_v5(g_system,dr);
			engine = new Scumm::ScummEngine_v5(g_system,dr);
			
			// Inform backend that the engine is about to be run
			g_system->engineInit();

			// Clear screen
			ili9341_write_frame_scumm(NULL,NULL,240);
			
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