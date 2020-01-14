#include "engines/advancedDetector.h"
#include "engines/metaengine.h"
#include "cine/cine.h"
#include "cine/various.h"
#include "common/translation.h"

/**
 * This is the namespace of the Cine engine.
 *
 */
 namespace Cine {

 #define GAMEOPTION_ORIGINAL_SAVELOAD   GUIO_GAMEOPTIONS1
 
 static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};
 
static const PlainGameDescriptor cineGames[] = {
	{"cine", "Cinematique evo.1 engine game"},
	{"fw", "Future Wars"},
	{"os", "Operation Stealth"},
	{0, 0}
};

static const Engines::ObsoleteGameID obsoleteGameIDsTable[] = {
	{"fw", "cine", Common::kPlatformUnknown},
	{"os", "cine", Common::kPlatformUnknown},
	{0, 0, Common::kPlatformUnknown}
};
 
/**
 * Cine Meta Engine
 */   
class CineMetaEngine : public AdvancedMetaEngine {
public:

	CineMetaEngine();

	PlainGameDescriptor findGame(const char *gameId) const override;

	virtual const char *getName() const;

	virtual const char *getOriginalCopyright() const;

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const ;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};


} // End of namespace Cine
