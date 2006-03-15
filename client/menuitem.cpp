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
#include "texture_manager.h"

#include "menuitem.h"

#define CHAR_WIDTH 9
#define MI_HEIGHT 20
#define MI_ALPHA 0.6f

byte CMenuItem::default_colour[3] = {  0,  0, 148 };
byte CMenuItem::default_hover[3]  = { 90, 90, 240 };
byte CMenuItem::chkbx_colour[3] = { 200, 200, 255 };
CTexture *CMenuItem::bg_texture = NULL;
CTexture *CMenuItem::chkbx_no = NULL;
CTexture *CMenuItem::chkbx_yes = NULL;
CTexture *CMenuItem::arrow = NULL;

CMenuItem::CMenuItem( char *label )
{
	Init();
	this->label = new char[strlen(label)+1];
	strcpy( this->label, label );
}

CMenuItem::CMenuItem( char *label, byte colour[3] )
{
	Init();
	this->label = new char[strlen(label)+1];
	strcpy( this->label, label );

	this->colour[0] = colour[0];
	
	for( int i=0; i<3; i++ )
	{
		this->colour[i] = colour[i];
		swatch[i] = colour[i];
		hover[i]  = default_hover[i];
	}
}

CMenuItem::CMenuItem( char *label, byte colour[3], byte hover[3], void(*callback)(int), bool(*checked)(int) )
{
	Init();
	this->label = new char[strlen(label)+1];
	strcpy( this->label, label );

	//this->colour[0] = colour[0];

	for( int i=0; i<3; i++ )
	{
		if( colour != NULL )
		{
			this->colour[i] = colour[i];
			swatch[i] = colour[i];
		}
		else
		{
			this->colour[i] = default_colour[i];
			swatch[i] = default_colour[i];
		}
		if( hover != NULL )
			this->hover[i]  = hover[i];
		else
			this->hover[i]  = default_hover[i];
	}

	this->callback = callback;
	this->checked = checked;
}

CMenuItem::~CMenuItem(void)
{
}

void CMenuItem::SetDefaultColour( byte colour[3] )
{
	default_colour[0] = colour[0];
	default_colour[1] = colour[1];
	default_colour[2] = colour[2];
}

void CMenuItem::SetDefaultHover( byte colour[3] )
{
	default_hover[0] = colour[0];
	default_hover[1] = colour[1];
	default_hover[2] = colour[2];
}

void CMenuItem::AddItem( CMenuItem *pItem )
{
	ASSERT( pItem != NULL );

	if( pSubMenu == NULL )
		pSubMenu = new MIVec();

	pItem->SetParent( this );

	pItem->menu_id = (unsigned int)(pSubMenu->size());
	pSubMenu->insert( pSubMenu->end(), pItem );

	int width = ((int)strlen( pItem->GetLabel() )*CHAR_WIDTH) + 2;
	width += 19; // Reserve for swatches
	
	if( checked != NULL )
		width += 19; // Reserve extra space for the check-box.

	if( pSubMenu != NULL )
		width += 20; // Reserve extra space for '>'

	if( width > submenu_max_width )
		submenu_max_width = width;
}

void CMenuItem::ClearSubmenu()
{
	if( pSubMenu != NULL )
		pSubMenu->clear();
}

int CMenuItem::Draw( int x, int y, int width, int mouse_x, int mouse_y, bool click, bool open )
{
	// Draw this.
	bool ret = false;
	bool hovering = PointInRect( mouse_x, mouse_y, x, y, width, MI_HEIGHT );

	if( hovering )
	{
		world.partVis->pGui->ResetHighlighted();
		highlighted = true;
		world.partVis->pGui->highlighted = this;
	}

	// Draw shadow:
	world.display->BindTexture(NULL);
	world.display->SetColour(0.0f, 0.0f, 0.0f, 0.6f);
	world.display->SetBlend(true);
	world.display->SetBlendMode(CDisplayManager::Transparent);
	world.display->Draw2DQuad( x+3, y+5, x+width+3, y+MI_HEIGHT+5 );
	//world.display->SetBlend(false);

	world.display->BindTexture(bg_texture);
	if( highlighted )
		world.display->SetColour( world.partVis->colour_table[hover[0]], world.partVis->colour_table[hover[1]], world.partVis->colour_table[hover[2]], MI_ALPHA );
	else
		world.display->SetColour( world.partVis->colour_table[colour[0]], world.partVis->colour_table[colour[1]], world.partVis->colour_table[colour[2]], MI_ALPHA );
	world.display->Draw2DQuad( x, y, x+width, y+MI_HEIGHT );

	world.display->BindTexture(NULL);
	world.display->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
	world.display->DrawString2( x+1, y+1, label );

	world.display->BindTexture(NULL);
	world.display->SetColour(world.partVis->colour_table[swatch[0]], world.partVis->colour_table[swatch[1]], world.partVis->colour_table[swatch[2]], 1.0f);
	
	if( pSubMenu != NULL )
	{
		if( !((swatch[0]==colour[0]) && (swatch[1]==colour[1]) && (swatch[2]==colour[2])) )
		{
			world.display->Draw2DQuad( x+width-29, y+2, x+width-14, y+MI_HEIGHT-3 );
			if( checked != NULL )
			{
				if( checked(id) )
					world.display->BindTexture(chkbx_yes);
				else
					world.display->BindTexture(chkbx_no);

				world.display->SetColour(world.partVis->colour_table[chkbx_colour[0]], world.partVis->colour_table[chkbx_colour[1]], world.partVis->colour_table[chkbx_colour[2]], 1.0f);
				world.display->Draw2DQuad( x+width-49, y+2, x+width-34, y+MI_HEIGHT-3 );
			}
		}
		else if( checked != NULL )
		{
			if( checked(id) )
				world.display->BindTexture(chkbx_yes);
			else
				world.display->BindTexture(chkbx_no);
			
			world.display->SetColour(world.partVis->colour_table[chkbx_colour[0]], world.partVis->colour_table[chkbx_colour[1]], world.partVis->colour_table[chkbx_colour[2]], 1.0f);
			world.display->Draw2DQuad( x+width-29, y+2, x+width-14, y+MI_HEIGHT-3 );
		}

		world.display->BindTexture(arrow);
		world.display->SetBlend(true);
		world.display->SetBlendMode(CDisplayManager::Transparent2);
		world.display->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
		
		world.display->Draw2DQuad( x+width-12, y+3, x+width, y+MI_HEIGHT-3 );
		//world.display->DrawString2( x+width-12, y+1, ">" );
	}
	else
	{	
		if( !((swatch[0]==colour[0]) && (swatch[1]==colour[1]) && (swatch[2]==colour[2])) )
		{
			world.display->Draw2DQuad( x+width-17, y+2, x+width-2, y+MI_HEIGHT-3 );

			if( checked != NULL )
			{
				if( checked(id) )
					world.display->BindTexture(chkbx_yes);
				else
					world.display->BindTexture(chkbx_no);

				world.display->SetColour(world.partVis->colour_table[chkbx_colour[0]], world.partVis->colour_table[chkbx_colour[1]], world.partVis->colour_table[chkbx_colour[2]], 1.0f);
				world.display->Draw2DQuad( x+width-37, y+2, x+width-22, y+MI_HEIGHT-3 );
			}
		}
		else if( checked != NULL )
		{
			if( checked(id) )
				world.display->BindTexture(chkbx_yes);
			else
				world.display->BindTexture(chkbx_no);
			
			world.display->SetColour(world.partVis->colour_table[chkbx_colour[0]], world.partVis->colour_table[chkbx_colour[1]], world.partVis->colour_table[chkbx_colour[2]], 1.0f);
			world.display->Draw2DQuad( x+width-17, y+2, x+width-2, y+MI_HEIGHT-3 );
		}
	}

	if( hovering && click && (callback != NULL) )
		callback(id);

	if( !open )
	{
		if( hovering )
		{
			if( pParent != NULL )
				pParent->menu_open = menu_id;
			return(1);
		}
		else
			return(-1);
	}
	
	/*if( hovering )
	{
		open = true;
		if( pParent != NULL )
			pParent->menu_open = menu_id;
	}*/

	// Draw sub-menu (if exists && mouse is over this):
	if( pSubMenu != NULL )
	{
		if( open )
		{
			for( uint32 i=0; i<pSubMenu->size(); i++ )
			{
				if( i == menu_open )
				{
					if( (*pSubMenu)[i]->Draw( x+width+2, y+((i*(MI_HEIGHT+2))), submenu_max_width, mouse_x, mouse_y, click, true ) == 1 )
						ret = true;
				}
				else
				{
					if( (*pSubMenu)[i]->Draw( x+width+2, y+((i*(MI_HEIGHT+2))), submenu_max_width, mouse_x, mouse_y, click, false ) == 1 )
						ret = true;
				}
			}
		}
	}

	if( hovering )
		return( 1 );

	if( !click )
		return( -1 );

	/*if( pSubMenu != NULL )
	{
		if( PointInRect(mouse_x, mouse_y, x+width, y, width+submenu_max_width, (int)(pSubMenu->size())*(MI_HEIGHT+2)) )
			return( true );
	}*/

	if( ret )
		return( 1 );

	return( 0 );
}

char *CMenuItem::GetLabel()
{
	return( label );
}

void CMenuItem::Init()
{
	for( int i=0; i<3; i++ )
	{
		colour[i] = default_colour[i];
		swatch[i] = colour[i];
		hover[i]  = default_hover[i];
	}

	callback = NULL;
	pSubMenu = NULL;
	pParent = NULL;

	submenu_max_width = 0;

	menu_open = -1;
	menu_id = 0;
	id = 0;
	checked = NULL;

	bg_texture = CTextureManager::tm.LoadTexture( "data/mi_bg.png" );
	chkbx_yes = CTextureManager::tm.LoadTexture( "data/chkbx_yes.png" );
	chkbx_no = CTextureManager::tm.LoadTexture( "data/chkbx_no.png" );
	arrow = CTextureManager::tm.LoadTexture( "data/arrow.png" );

	highlighted = false;
}

CMenuItem *CMenuItem::KN_Close()
{
	ASSERT( highlighted );

	if( pParent != NULL && pParent->pParent != NULL )
	{
		highlighted = false;
		pParent->highlighted = true;
		//if( pParent->pParent != NULL )
			pParent->pParent->menu_open = -1;
		pParent->menu_open = -1;

		return( pParent );
	}

	return(this);
}

CMenuItem *CMenuItem::KN_Down()
{
	ASSERT( highlighted );

	if( pParent != NULL )
	{
		//highlighted = false;

		/*
		pParent->menu_open++;
		(*pParent->pSubMenu)[pParent->menu_open]->highlighted = true;
		world.partVis->pGui->highlighted = (*pParent->pSubMenu)[pParent->menu_open];
		*/

		// Determine position of this in parent submenu:
		unsigned int i = 0;
		bool found = false;
		for( ; i<pParent->pSubMenu->size(); i++ )
		{
			if( (*pParent->pSubMenu)[i]->highlighted ) // this.
			{
				found = true;
				break;
			}
		}

		ASSERT(found);
		
		i++;
		if( i>=pParent->pSubMenu->size() )
			i = 0; // Cycle back to top.
		highlighted = false;
		(*pParent->pSubMenu)[i]->highlighted = true;

		return( (*pParent->pSubMenu)[i] );
	}

	return(this);
}

CMenuItem *CMenuItem::KN_Open()
{
	ASSERT( highlighted );

	if( pSubMenu != NULL && pParent != NULL )
	{
		// Determine position of this in parent submenu:
		unsigned int i = 0;
		bool found = false;
		for( ; i<pParent->pSubMenu->size(); i++ )
		{
			if( (*pParent->pSubMenu)[i]->highlighted ) // this.
			{
				found = true;
				break;
			}
		}

		ASSERT(found);

		highlighted = false;
		pParent->menu_open = i;
		(*pSubMenu)[0]->highlighted = true;
		return( (*pSubMenu)[0] );
	}

	return(this);
}

CMenuItem *CMenuItem::KN_Up()
{
	ASSERT( highlighted );

	if( pParent != NULL )
	{

		// Determine position of this in parent submenu:
		int i = 0;
		bool found = false;
		for( ; i<(int)(pParent->pSubMenu->size()); i++ )
		{
			if( (*pParent->pSubMenu)[i]->highlighted ) // this.
			{
				found = true;
				break;
			}
		}

		ASSERT(found);

		i--;
		if( i < 0 )
			i = (int)(pParent->pSubMenu->size()) - 1; // Cycle to bottom.
		highlighted = false;
		(*pParent->pSubMenu)[i]->highlighted = true;

		return( (*pParent->pSubMenu)[i] );
	}

	return(this);
}

void CMenuItem::SetID( int id )
{
	this->id = id;
}

void CMenuItem::SetParent( CMenuItem *parent )
{
	pParent = parent;
}

void CMenuItem::SetSwatch( byte swatch[3] )
{
	this->swatch[0] = swatch[0];
	this->swatch[1] = swatch[1];
	this->swatch[2] = swatch[2];
}

void CMenuItem::ResetAll()
{
	menu_open = -1;
	if( pSubMenu != NULL )
	{
		for( unsigned int i=0; i<pSubMenu->size(); i++ )
			(*pSubMenu)[i]->ResetAll();
	}
}
