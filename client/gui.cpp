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

#include "stdafx.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include "world.h"
#include "display_manager.h"
#include "reporter.h"
#include "system_driver.h"
#include "action.h"
#include "entity.h"
#include "partflow.h"
#include "partvis.h"

#include "gui.h"

#define MIN_MENU_ALPHA 0.25f
#define MAX_MENU_ALPHA 1.0f

CGui::CGui(void)
{
	for( int i=0; i<6; i++ )
	{
		menu_alpha[i] = MIN_MENU_ALPHA;
		click_on[i] = false;
	}

	for( int i=0; i<256; i++ )
		click_flow[i] = false;

	click_speed[0] = false;
	click_speed[1] = false;

	click_dnet[0] = false;
	click_dnet[1] = false;
	click_dnet[2] = false;

	menu_open = false;
	lmb_wasDown = false;
	root_open = false;

	byte col[3];
	byte hov[3];

	col[0] = 20;
	col[1] = 20;
	col[2] = 175;

	hov[0] = 75;
	hov[1] = 75;
	hov[2] = 225;

	highlighted = NULL;

	root_item = new CMenuItem( "Menu", col, hov, &CGui::CB_OpenRoot );
	fd = new CMenuItem( "Flow display" );
	CMenuItem *dm = new CMenuItem( "Darknet mode" );
	CMenuItem *sp = new CMenuItem( "Speed       " );

	dm->AddItem( new CMenuItem( "All traffic", NULL, NULL, &CGui::CB_DMAll ) );
	dm->AddItem( new CMenuItem( "Darknet only", NULL, NULL, &CGui::CB_DMOnly ) );
	dm->AddItem( new CMenuItem( "Sans darknet", NULL, NULL, &CGui::CB_DMSans ) );
	sp->AddItem( new CMenuItem( "Faster", NULL, NULL, &CGui::CB_Faster ) );
	sp->AddItem( new CMenuItem( "Slower", NULL, NULL, &CGui::CB_Slower ) );

	root_item->AddItem( fd );
	root_item->AddItem( dm );
	root_item->AddItem( sp );
	root_item->AddItem( new CMenuItem( "Pause", NULL, NULL, &CGui::CB_Pause ) );
	root_item->AddItem( new CMenuItem( "Quit", NULL, NULL, &CGui::CB_Quit ) );
}

CGui::~CGui(void)
{
	if( root_item != NULL )
	{
		delete root_item;
		root_item = NULL;
	}
}

void CGui::Draw( int x, int y, int mouse_x, int mouse_y, bool click )
{
	world.display->Begin2D();

	bool wasopen = root_open;

	int ret = root_item->Draw( x, y, 55, mouse_x, mouse_y, click, root_open );
	if( ret == 0 )
	{
		root_open = false;
		world.actionHandler->gui_open = false;
	}
	/*if( ret == 1 )
		root_open = true;
	else if( ret == 0 )
		root_open = false;*/

	if( wasopen && !root_open )
		root_item->ResetAll();

	world.display->End2D();
}

void CGui::InitFD()
{
	FlowDescMap::iterator iter;
	fd->ClearSubmenu();

	CMenuItem *ap = new CMenuItem( "Toggle all", NULL, NULL, &CGui::CB_FlowType );
	ap->SetID(-1);
	fd->AddItem(ap);

	for( iter = world.partVis->fdmap.begin(); iter != world.partVis->fdmap.end(); iter++ )
	{
		if( strcmp( iter->second->name, "<NULL>" ) != 0 )
		{
			CMenuItem *tmp = new CMenuItem( iter->second->name, NULL, NULL, &CGui::CB_FlowType, &CGui::CB_Checked );
			tmp->SetID( iter->first );
			tmp->SetSwatch( iter->second->colour );
			fd->AddItem( tmp );
		}
	}
}

void CGui::OnKeyDown( unsigned short key )
{
	// Note: all the hardcoded values here should really be fixed to come straight from the CActionHandler::Keycode enum.
	// Log( "Keycode: %d", key );
	switch( key )
	{
	case 258:
		// Enter
		if( highlighted == NULL )
			break;
		if( highlighted->callback == NULL )
			break;
		highlighted->callback(highlighted->id);
		break;
	//case 263:
	case CActionHandler::BKC_LEFT:
		// LEFT
		if( highlighted == NULL )
			break;
		highlighted = highlighted->KN_Close();
		break;
	//case 264:
	case CActionHandler::BKC_RIGHT:
		// RIGHT
		if( highlighted == NULL )
			break;
		highlighted = highlighted->KN_Open();
		break;
	//case 265:
	case CActionHandler::BKC_UP:
		// UP
		if( highlighted == NULL )
			break;
		highlighted = highlighted->KN_Up();
		break;
	//case 266:
	case CActionHandler::BKC_DOWN:
		// DOWN
		if( highlighted == NULL )
			break;
		highlighted = highlighted->KN_Down();
		break;
	case 282:
		// F1
		root_open = !root_open;
		world.actionHandler->gui_open = !world.actionHandler->gui_open;

		if( root_open )
		{
			if( root_item->pSubMenu != NULL )
			{
				highlighted = (*root_item->pSubMenu)[0];
				highlighted->highlighted = true;
			}
		}
		else if(highlighted != NULL )
		{
			// TODO: Cycle through parents and set menu_open to -1.
			root_item->ResetAll();

			highlighted->highlighted = false;
			highlighted = NULL;
		}
		break;
	default:
		break;
	}
}

void CGui::ResetHighlighted()
{
	if( highlighted != NULL )
	{
		highlighted->highlighted = false;
		highlighted = NULL;
	}
}

void CGui::CB_DMAll(int id)
{
	world.partVis->show_dark = 0;
}

void CGui::CB_DMOnly(int id)
{
	world.partVis->show_dark = 1;
}

void CGui::CB_DMSans(int id)
{
	world.partVis->show_dark = 2;
}

void CGui::CB_Faster(int id)
{
	world.partVis->ChangeSpeed(true);
}

void CGui::CB_FlowType(int id)
{
	// world.partVis->filter_state = id;

	if( id == -1 )
	{
		// Toggle all on/off.
		FlowDescMap::iterator iter = world.partVis->fdmap.find( 0 );
		if( iter->second->show )
		{
			// All off.
			for( iter = world.partVis->fdmap.begin(); iter != world.partVis->fdmap.end(); iter++ )
				iter->second->show = false;
		}
		else
		{
			// All on.
			for( iter = world.partVis->fdmap.begin(); iter != world.partVis->fdmap.end(); iter++ )
				iter->second->show = true;
		}
	}
	else
	{
		FlowDescMap::iterator iter = world.partVis->fdmap.find( id );
		ASSERT( iter != world.partVis->fdmap.end() );

		iter->second->show = ! (iter->second->show);
	}
}

void CGui::CB_Pause(int id)
{
	world.partVis->TogglePaused();
}

void CGui::CB_Quit(int id)
{
	world.sys->Quit();
}

void CGui::CB_Slower(int id)
{
	world.partVis->ChangeSpeed(false);
}

void CGui::CB_OpenRoot(int id)
{
	world.partVis->pGui->root_open = true;
	world.actionHandler->gui_open = true;
}

bool CGui::CB_Checked(int id)
{
	FlowDescMap::iterator iter = world.partVis->fdmap.find( id );
	return( iter->second->show );
}
