/*******************************************************************************
							BSOD2 Client - misc.h
							
 Basically anything weird that didn't fit got lumped into here :)
*******************************************************************************/


/*********************************************
	 	Other #defines and stuff
**********************************************/
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define ABS(a)	   (((a) < 0) ? -(a) : (a))






/*********************************************
			 	Typedefs
**********************************************/
typedef unsigned char byte;






/*********************************************
	 	Simple Color object
**********************************************/
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

