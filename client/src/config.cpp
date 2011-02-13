/* 
 * This file is part of BSOD client
 *
 * Copyright (c) 2011 The University of Waikato, Hamilton, New Zealand.
 *
 * Author: Paul Hunkin
 *
 * Contributors: Shane Alcock
 *
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND research
 * group. For further information please see http://www.wand.net.nz/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include "main.h"

/*********************************************
	Use libconfuse to load config file
**********************************************/
bool App::loadConfig(){
	cfg_opt_t opts[] =	{
		CFG_STR((char *)"Server", (char *)"", CFGF_NONE),
		CFG_STR((char *)"LeftTex", (char *)"", CFGF_NONE),
		CFG_STR((char *)"RightTex", (char *)"", CFGF_NONE),
		CFG_STR((char *)"ParticleTextureName", (char *)"particle.bmp", CFGF_NONE),
		CFG_INT((char *)"Port", 54567, CFGF_NONE),
		CFG_INT((char *)"ResX", 0, CFGF_NONE),
		CFG_INT((char *)"ResY", 0, CFGF_NONE),
		CFG_INT((char *)"Fullscreen", 1, CFGF_NONE),
		CFG_FLOAT((char *)"ParticleSizeScale", 1.0f, CFGF_NONE),
		CFG_FLOAT((char *)"ParticleSpeedScale", 1.0f, CFGF_NONE),
		CFG_INT((char *)"ParticleMethod", PARTICLE_SYSTEM_UNSPECIFIED, CFGF_NONE),
		CFG_INT((char *)"DropPacketThresh", 60, CFGF_NONE),
		CFG_INT((char *)"DropFlowThresh", 30, CFGF_NONE),
		CFG_INT((char *)"Fullscreen", 0, CFGF_NONE),
		CFG_INT((char *)"ShowDarknet", 1, CFGF_NONE),
		CFG_INT((char *)"ShowNonDarknet", 1, CFGF_NONE),
		CFG_INT((char *)"FlipTextures", 0, CFGF_NONE),
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
	bTexFlip = cfg_getint(cfg, "FlipTextures");
	mParticleTexName = string(cfg_getstr(cfg, "ParticleTextureName"));
	
	cfg_free(cfg);
	
	return true;
}
