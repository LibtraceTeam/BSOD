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

#ifndef MENUITEM_H
#define MENUITEM_H

#include <vector>

class CMenuItem;

typedef std::vector<CMenuItem*> MIVec;

class CMenuItem
{
public:
	CMenuItem( char *label );
	CMenuItem( char *label, byte colour[3] );
	CMenuItem( char *label, byte colour[3], byte hover[3], void(*callback)(int) = NULL, bool(*checked)(int) = NULL );
	~CMenuItem(void);

	static void SetDefaultColour( byte colour[3] );
	static void SetDefaultHover(  byte colour[3] );

	void AddItem( CMenuItem *pItem );
	/* Note: CMenuItem Draw calls _must_ be done inside
			 the begin/end 2d draw block or they will fail! */
	void ClearSubmenu();
	int Draw( int x, int y, int width, int mouse_x, int mouse_y, bool click, bool open );
	char *GetLabel();
	
	// Keyboard Navigation (KN) functions:
	CMenuItem *KN_Close();
	CMenuItem *KN_Down();
	CMenuItem *KN_Open();
	CMenuItem *KN_Up();
	
	//bool IsKeepOpen();
	//void SetKeepOpen( bool state );
	void SetID( int id );
	void SetParent( CMenuItem *parent );
	void SetSwatch( byte swatch[3] );
	void ResetAll();

protected:
	void Init();

	char *label;
	short int type;
	byte colour[3];
	byte hover[3];
	byte swatch[3]; // Color swatch block to display.
	void(*callback)(int);
	bool(*checked)(int);
	MIVec *pSubMenu;
	int submenu_max_width;
	CMenuItem *pParent;
	unsigned int menu_id; // Position of this item in the parent. If pParent == NULL this is not applicable (it should be 0).
	int menu_open; // (-1 if none open, else number of the menu_item in the vector).
	int id; // Used for special return codes in the callback.
	bool highlighted; // For keyboard navigation.
	
	static CTexture *bg_texture;
	static CTexture *chkbx_yes;
	static CTexture *chkbx_no;
	static CTexture *arrow;
	static byte default_colour[3];
	static byte default_hover[3];
	static byte chkbx_colour[3];

	friend class CGui;
};

#endif
