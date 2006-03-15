/*
* This file is part of bsod-client
*
* Copyright (c) 2005 The University of Waikato, Hamilton, New Zealand.
* Authors: Sebastian Dusterwald
*          
* All rights reserved.
*
* This code has been developed by the University of Waikato WAND 
* research group. For further information please see http://www.wand.net.nz/
*
* bsod-client includes software developed by Sam Jansen and Jesse Baker 
* (see http://www.wand.net.nz/~stj2/bung).
*/

#ifndef GUI_H
#define GUI_H

#include "menuitem.h"

typedef enum MENU_ITEM
{
	MI_MENU = 0,
	MI_FLOW = 1,
	MI_DARKNET = 2,
	MI_SPEED = 3,
	MI_PAUSE = 4,
	MI_QUIT = 5
};

class CGui
{
public:
	CGui(void);
	~CGui(void);

	void Draw( int x, int y, int mouse_x, int mouse_y, bool click );
	void InitFD();

	void OnKeyDown( unsigned short key );
	void ResetHighlighted();

	static void CB_DMAll(int id);
	static void CB_DMOnly(int id);
	static void CB_DMSans(int id);
	static void CB_Faster(int id);
	static void CB_FlowType(int id);
	static void CB_Pause(int id);
	static void CB_Quit(int id);
	static void CB_Slower(int id);
	static void CB_OpenRoot(int id);
	static bool CB_Checked(int id);

	bool root_open;

protected:
	float menu_alpha[6];
	bool menu_open;
	bool click_on[6];
	bool click_speed[2];
	bool click_dnet[3];
	bool click_flow[256];
	bool lmb_wasDown;
	
	CMenuItem *fd;
	CMenuItem *root_item;
	CMenuItem *highlighted;

	friend class CMenuItem;
};

#endif
