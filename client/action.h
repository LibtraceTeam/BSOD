/*
Copyright (c) Sam Jansen and Jesse Baker.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The end-user documentation included with the redistribution, if
any, must include the following acknowledgment: "This product includes
software developed by Sam Jansen and Jesse Baker 
(see http://www.wand.net.nz/~stj2/bung)."
Alternately, this acknowledgment may appear in the software itself, if
and wherever such third-party acknowledgments normally appear.

4. The hosted project names must not be used to endorse or promote
products derived from this software without prior written
permission. For written permission, please contact sam@wand.net.nz or
jtb5@cs.waikato.ac.nz.

5. Products derived from this software may not use the "Bung" name
nor may "Bung" appear in their names without prior written
permission of Sam Jansen or Jesse Baker.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// $Id$
#ifndef _ACTION_H_
#define _ACTION_H_

class CActionHandler
{
private:
	
public:
	
	// This will need to be kept up to date with what is in the system drivers, as
	// the system drivers need to map system-specific keys to BuNg key codes (that's
	// what BKC is - (B)uNg (K)ey (C)ode).
	enum Keycode {
		//
		BKC_ESCAPE = 256, BKC_BACKSPACE, BKC_RETURN, BKC_TAB,
		// Mouse clicks aren't exactly keys but they might as well be handled as such
		BKC_LEFTMOUSEBUT,
		
		// Arrow keys
		BKC_LEFT, BKC_RIGHT, BKC_UP, BKC_DOWN,

		// Function keys
		BKC_F1 = 282, // = SDLK_F1

		// Normal ASCII codes:
		// Alphabet
		BKC_A = 'a', BKC_B = 'b', BKC_C = 'c', BKC_D = 'd', BKC_E = 'e', BKC_F = 'f', 
		BKC_G = 'g', BKC_H = 'h', BKC_I = 'i', BKC_J = 'j', BKC_K = 'k', BKC_L = 'l', 
		BKC_M = 'm', BKC_N = 'n', BKC_O = 'o', BKC_P = 'p', BKC_Q = 'q', BKC_R = 'r', 
		BKC_S = 's', BKC_T = 't', BKC_U = 'u', BKC_V = 'v', BKC_W = 'w', BKC_X = 'x',
		BKC_Y = 'y', BKC_Z = 'z',
		// Numerical
		BKC_1 = '1', BKC_2 = '2', BKC_3 = '3', BKC_4 = '4', BKC_5 = '5', BKC_6 = '6',
		BKC_7 = '7', BKC_8 = '8', BKC_9 = '9', BKC_0 = '0',
		// Stuff
		BKC_EXCLAMATION = '!', BKC_AT = '@', BKC_HASH = '#', BKC_DOLLAR = '$',
		BKC_PERCENT = '%', BKC_HAT = '^', BKC_AMPERSAND = '&', BKC_ASTERISK = '*',
		BKC_LBRACKET = '(', BKC_RBRACKET = ')', BKC_MINUS = '-', BKC_UNDERSCORE = '_',
		BKC_PLUS = '+', BKC_EQUALS = '=', BKC_PIPE = '|', BKC_BACKSLASH = '\\',
		BKC_SLASH = '/', BKC_COLON = ':', BKC_SEMICOLON = ';', BKC_APOSTRAPHE = '\'',
		BKC_QUOTE = '"', BKC_SPACE = ' ', BKC_PERIOD = '.', BKC_COMMA = ',',
		BKC_QUESTION_MARK = '?'
		// Not in there yet:
		// < > [ ] { } ~ `
		// numpad stuff
		// insert delete home end pageup pagedown scrolllock numlock pause/break
		// cursor keys
		// f1 f2 f3 ... f12
	};
	
	CActionHandler();

	void KeyDown(Keycode key);
	void KeyUp(Keycode key);
	
	// Actions
	void BeginMovingForward();
	void EndMovingForward();
	void BeginMovingBackward();
	void EndMovingBackward();
	void BeginStrafingLeft();
	void EndStrafingLeft();
	void BeginStrafingRight();
	void EndStrafingRight();
	void Quit();
	void Fire();
	void ToggleWireframe();
	void ToggleBackfaceCull();
	void ToggleOctreeBoxes();
	void ToggleGhostMode();
	void Pause();
	void Screenshot();
    void ToggleDebugDisplay();
	void ToggleFilter();
	void ToggleShowDark();
	void ToggleBackFilter();
	void ToggleHelp();
	void Faster();
	void Slower();

	void TurnUp();
	void TurnDown();
	void TurnLeft();
	void TurnRight();
};

#endif // _ACTION_H_

