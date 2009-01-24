#include "main.h"

/*********************************************
	Use libconfuse to load config file
**********************************************/
bool App::loadConfig(){
	cfg_opt_t opts[] =	{
		CFG_STR("Server", "", CFGF_NONE),
		CFG_STR("LeftTex", "", CFGF_NONE),
		CFG_STR("RightTex", "", CFGF_NONE),
		CFG_INT("Port", 54567, CFGF_NONE),
		CFG_INT("ResX", 1024, CFGF_NONE),
		CFG_INT("ResY", 768, CFGF_NONE),
		CFG_INT("Fullscreen", 0, CFGF_NONE),
		CFG_FLOAT("ParticleSizeScale", 1.0f, CFGF_NONE),
		CFG_FLOAT("ParticleSpeedScale", 1.0f, CFGF_NONE),
		CFG_INT("ParticleMethod", PARTICLE_SYSTEM_UNSPECIFIED, CFGF_NONE),
		CFG_INT("DropPacketThresh", 60, CFGF_NONE),
		CFG_INT("DropFlowThresh", 30, CFGF_NONE),
		CFG_INT("Fullscreen", 0, CFGF_NONE),
		CFG_INT("ShowDarknet", 1, CFGF_NONE),
		CFG_INT("ShowNonDarknet", 1, CFGF_NONE),
		CFG_END()
	};
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	
	if(cfg_parse(cfg, "bsod2.cfg") == CFG_PARSE_ERROR)
		return false;

	iScreenX = cfg_getint(cfg, "ResX");
	iScreenY = cfg_getint(cfg, "ResY");
	bFullscreen = cfg_getint(cfg, "Fullscreen");
	mServerAddr = string(cfg_getstr(cfg, "Server"));
	iServerPort = cfg_getint(cfg, "Port");
	fParticleSizeScale = cfg_getfloat(cfg, "ParticleSizeScale");
	fParticleSpeedScale = cfg_getfloat(cfg, "ParticleSpeedScale");
	iParticleMethod = cfg_getint(cfg, "ParticleMethod");
	iDropPacketThresh = cfg_getint(cfg, "DropPacketThresh");
	iDropFlowThresh = cfg_getint(cfg, "DropFlowThresh");
	mLeftTexName = string(cfg_getstr(cfg, "LeftTex"));
	mRightTexName = string(cfg_getstr(cfg, "RightTex"));
	bShowDarknet = cfg_getint(cfg, "ShowDarknet");
	bShowNonDarknet = cfg_getint(cfg, "ShowNonDarknet");
	
	cfg_free(cfg);
	
	return true;
}
