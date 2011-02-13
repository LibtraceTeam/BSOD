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
#include <stdarg.h>




/*********************************************
		Color object
**********************************************/
Color::Color(){
	r = g = b = 0.0f;
}
	
void Color::copy(Color *c){
	r = c->r;
	g = c->g;
	b = c->b;
}

int App::randInt(int low, int high){
	return rand() % (high-low) + low;
}

float App::randFloat(){
	return randFloat(0,1);
}

float App::randFloat(float min, float max){
	return min + (((float)rand()/(float)RAND_MAX) * (max - min));
}

static char message[512];


/*********************************************
			Logging system
**********************************************/

std::ofstream mLogFile;

void openLog(string filename){
	mLogFile.open(filename.c_str());
}

void closeLog(){
	mLogFile.close();
} 

void LOG(const char *fmt, ...){
  	va_list args;
    va_start(args, fmt);
    vsnprintf(message, 512, fmt, args);
    va_end(args);
    
   // mConsole->write(CONSOLE_COLOR_NORMAL, message);
    
    printf("%s", message);
    
    mLogFile << message;
	mLogFile.flush();
}

void ERR(const char *fmt, ...){
  	va_list args;
    va_start(args, fmt);
    vsnprintf(message, 512, fmt, args);
    va_end(args);
    
   // mConsole->write(CONSOLE_COLOR_ERR, message);
    
    printf("ERR: %s", message);
    
    mLogFile << "ERR: " << message;
	mLogFile.flush();

}

void WARN(const char *fmt, ...){
  	va_list args;
    va_start(args, fmt);
    vsnprintf(message, 512, fmt, args);
    va_end(args);
    
   // mConsole->write(CONSOLE_COLOR_WARN, message);
    
    printf("WARN: %s", message);

    mLogFile << "WARN: " << message;
	mLogFile.flush();
}


//http://www.codeproject.com/string/stringsplit.asp
int splitString(const std::string &input, const std::string &delimiter, 
				std::vector<std::string> &results, bool includeEmpties)
{
   int iPos = 0;
    int newPos = -1;
    int sizeS2 = (int)delimiter.size();
    int isize = (int)input.size();

    if( 
        ( isize == 0 )
        ||
        ( sizeS2 == 0 )
    )
    {
        return 0;
    }

    std::vector<int> positions;

    newPos = input.find (delimiter, 0);

    if( newPos < 0 )
    { 
        return 0; 
    }

    int numFound = 0;

    while( newPos >= iPos )
    {
        numFound++;
        positions.push_back(newPos);
        iPos = newPos;
        newPos = input.find (delimiter, iPos+sizeS2);
    }

    if( numFound == 0 )
    {
        return 0;
    }

    for( int i=0; i <= (int)positions.size(); ++i )
    {
        std::string s("");

		int offset = 0;

        if( i == 0 ) 
        { 
            s = input.substr( i, positions[i] ); 
        }
		else if(i > 0){
			offset = positions[i-1] + sizeS2;
		}
        if( offset < isize )
        {
            if( i == (int)positions.size() )
            {
                s = input.substr(offset);
            }
            else if( i > 0 )
            {
                s = input.substr( positions[i-1] + sizeS2, 
                      positions[i] - positions[i-1] - sizeS2 );
            }
        }
        if( includeEmpties || ( s.size() > 0 ) )
        {
            results.push_back(s);
        }
    }
    return numFound;

}

