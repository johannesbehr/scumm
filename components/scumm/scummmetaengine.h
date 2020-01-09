

#include "engines/metaengine.h"


/**
 * This is the namespace of the SCUMM engine.
 *
 * Status of this engine:
 * Complete support for all SCUMM based LucasArts adventures.
 * Complete support for many Humongous Entertainment games,
 * but for some of the newer ones, this is still work in progress.
 *
 * Games using this engine:
 * - Classic 2D LucasArts adventures
 * - numerous Humongous Entertainment games
 */
 
namespace Scumm {

/**
 * Scumm Meta Engine
 */   
class ScummMetaEngine : public MetaEngine {
public:
	virtual const char *getName() const;
	virtual const char *getOriginalCopyright() const;

	virtual bool hasFeature(MetaEngineFeature f) const;
	PlainGameList getSupportedGames() const override;
	PlainGameDescriptor findGame(const char *gameid) const override;
	virtual DetectedGames detectGames(const Common::FSList &fslist) const override;

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const;

	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const;
};


} // End of namespace Scumm
