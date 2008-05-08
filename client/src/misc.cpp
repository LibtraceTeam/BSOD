#include "misc.h"
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

/*********************************************
			Random numbers
**********************************************/
int App::randInt(int low, int high){
	return rand() % (high-low) + low;
}

float App::randFloat(){
	return (float) rand() / (float) 0x7fffffff;
}

float App::randFloat(float low, float high){
	return (randFloat() * (high-low)) + low;
}

static char message[512];

void LOG(const char *fmt, ...){
  	va_list args;
    va_start(args, fmt);
    vsnprintf(message, 512, fmt, args);
    va_end(args);
    
   // mConsole->write(CONSOLE_COLOR_NORMAL, message);
    
    printf("%s", message);

}

void ERR(const char *fmt, ...){
  	va_list args;
    va_start(args, fmt);
    vsnprintf(message, 512, fmt, args);
    va_end(args);
    
   // mConsole->write(CONSOLE_COLOR_ERR, message);
    
    printf("ERR: %s", message);

}

void WARN(const char *fmt, ...){
  	va_list args;
    va_start(args, fmt);
    vsnprintf(message, 512, fmt, args);
    va_end(args);
    
   // mConsole->write(CONSOLE_COLOR_WARN, message);
    
    printf("WARN: %s", message);

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
        if( i == 0 ) 
        { 
            s = input.substr( i, positions[i] ); 
        }
        int offset = positions[i-1] + sizeS2;
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

