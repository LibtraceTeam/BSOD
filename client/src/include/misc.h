#include "main.h"


/*********************************************
	 	Other #defines and stuff
**********************************************/
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

typedef unsigned char byte;

//screen width, height, and bit depth
//These used to be numbers, in order to keep
//compatibility, they changed to point to the vars in App 
#define SCREEN_WIDTH  App::S()->iScreenX
#define SCREEN_HEIGHT App::S()->iScreenY
#define SCREEN_BPP     32
#define SCREEN_FULLSCREEN App::S()->bFullscreen

#define PI 3.14159265

#ifndef _COLOR
#define _COLOR

//Color struct
class Color{
public:
	float r, g, b;
	
	Color();
	Color(float _r, float _g, float _b){
		r = _r; g=_g; b=_b;
		
		//hack!
		if(r > 1.0f || g > 1.0f || b > 1.0f){
			r /= 255.0f;
			g /= 255.0f;
			b /= 255.0f;
		}
	}
	
	void copy(Color *c);
};


#endif
