#ifndef _RENDERABLE_H
#define _RENDERABLE_H

/*********************************************
The interface for anything that can be rendered
**********************************************/
class IRenderable{
protected:
	int iWorldX;
	int iWorldY;
	int iWorldZ;
public:
	virtual void render()=0;
	
	//Getters
	virtual int getWorldX(){return iWorldX;}
	virtual int getWorldY(){return iWorldY;}
	virtual int getWorldZ(){return iWorldZ;}
	
	virtual ~IRenderable(){};
	
};

#endif
