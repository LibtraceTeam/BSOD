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
	
	float sum(){
		return (r * 1) + (g * 10) + (b * 100);
	}	
	
	//The same formatting that CEGUI uses
	string toString(){
		byte rb = r * 255;
		byte gb = g * 255;
		byte bb = b * 255;
		
		char buf[32];
		sprintf(buf, "FF%s%X%s%X%s%X", 	rb < 0xF ? "0" : "", rb, 
										gb < 0xF ? "0" : "", gb, 
										bb < 0xF ? "0" : "", bb);
		
		//printf("%s\n", buf);
		
		return string(buf);
	}
	
	void bind(){
		glColor3f(r,g,b);
	}
	
	
};

struct ColorSort{
    public: bool operator() (Color *a, Color *b){
		return a->sum() > b->sum();
    }
};
