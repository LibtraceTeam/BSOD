#!/bin/bash

tar -cvvf bsod-macosx.tar \
	src/*.cpp src/include/*.h \
	runtime/bsod2.cfg runtime/data/arial.ttf \
	runtime/data/banner.png  \
	runtime/data/mouse.png runtime/data/particle.png \
	runtime/data/gui/configs/CEGUIConfig.xsd \
	runtime/data/gui/fonts/*.ttf \
	runtime/data/gui/fonts/*.font \
	runtime/data/gui/fonts/*.txt \
	runtime/data/gui/fonts/README \
	runtime/data/gui/fonts/Font.xsd \
	runtime/data/gui/fonts/fkp.de.pcf \
	runtime/data/gui/imagesets/sleekspace_bsod.* \
	runtime/data/gui/imagesets/Imageset.xsd \
	runtime/data/gui/layouts/*.layout  \
	runtime/data/gui/layouts/Console.wnd \
	runtime/data/gui/layouts/GUILayout.xsd \
	runtime/data/gui/looknfeel/Falagard.xsd \
	runtime/data/gui/looknfeel/sleekspace_bsod.looknfeel \
	runtime/data/gui/lua_scripts/*.lua \
	runtime/data/gui/schemes/GUIScheme.xsd \
	runtime/data/gui/schemes/SleekSpaceBSOD.scheme \
	runtime/data/gui/XMLRefSchema/*.xsd \
	runtime/data/shaders/*.frag \
	runtime/data/shaders/*.vert \
	runtime/data/bsod.icns \
	macosx/README.txt \
	macosx/config.h \
	macosx/build_process \
	macosx/bsodclient/bsodclient_Prefix.pch \
	macosx/bsodclient/bsodclient.xcodeproj/project.pbxproj \
	macosx/bsodclient/bsodclient.xcodeproj/wand.mode1v3 \
	macosx/bsodclient/bsodclient.xcodeproj/wand.pbxuser \
	macosx/bsodclient/English.lproj/InfoPlist.strings \
	macosx/bsodclient/Info.plist \
	macosx/bsodclient/SDLMain.h \
	macosx/bsodclient/SDLMain.m

gzip -1 bsod-macosx.tar




