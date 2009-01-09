#include "main.h"
#include <stdarg.h>

//This is pretty quick and nasty.
//Basically I just copied a 'how to use SDL_TTF' from gamedev, but it seems to work OK
//http://www.gamedev.net/community/forums/topic.asp?topic_id=458211

const int OFFSET = 32;
const int MAX_ASCII = 128;

GLuint textureId;
GLuint* chars;

int *charWidths = NULL;

/*********************************************
 		Creates the font texturers
**********************************************/
void App::initFont(){

	TTF_Init();
	TTF_Font* font = TTF_OpenFont("data/arial.ttf", 30);  
	
	if(font){
		
	}else{
		LOG("Failed to open font!\n");
		return;
	}    

	float charWidth        = 40;
	float charHeight       = 50;
	float textureWidth     = 512;
	float textureHeight    = 512;

	SDL_Color color = { 255, 255, 255, 255 };      
	
	std::vector<SDL_Surface*> glyphCache;         
	for(int i = OFFSET; i < MAX_ASCII; i++)
		glyphCache.push_back(TTF_RenderGlyph_Blended(font, i, color));

	//create destination surface
	SDL_Surface* destination = SDL_CreateRGBSurface(SDL_SWSURFACE, 
								(int)textureWidth, (int)textureHeight, 32, 
								0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);      

	//blit glyphs onto destination       
	SDL_Rect glyphRect;
	int row = 0;   // 10 chars per row
	int col = 0;   
	for(int i = OFFSET; i < MAX_ASCII; i++) {
		int minx, maxx, miny, maxy, advance;
		TTF_GlyphMetrics(font, i, &minx, &maxx, &miny, &maxy, &advance);
		glyphRect.x = (int)(col * charWidth);
		glyphRect.y = (int)(row * charHeight + TTF_FontAscent(font) - maxy);                  

		SDL_BlitSurface(glyphCache[i - OFFSET], 0, destination, &glyphRect);

		if(col >= 10) {
			col = 0;
			row++;
		}
		else{
			col++;      
		}
	}

	/* create texture */
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);   
	glTexImage2D(GL_TEXTURE_2D, 0, 4, (int)textureWidth, (int)textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, destination->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      

	/* create display lists */   
	chars = new GLuint[MAX_ASCII];
	charWidths = new int[MAX_ASCII];

	row = 0;
	col = 0;
	for(int i = 0; i < MAX_ASCII; i++) {
		int id = glGenLists(1);
		chars[i] = id;

		int minx, maxx, miny, maxy, advance;
		TTF_GlyphMetrics(font, i + OFFSET, &minx, &maxx, &miny, &maxy, &advance);

		float minX = charWidth * col / textureWidth;
		float maxX = (advance) / textureWidth + minX;
		float minY = charHeight / textureHeight * row;
		float maxY = minY + charHeight / textureHeight;

		charWidths[i] = advance;

		glNewList(id, GL_COMPILE); 
		glBegin(GL_QUADS);
			glTexCoord2f(minX, minY);   glVertex2f(0.0f, 0.0f);       
			glTexCoord2f(maxX, minY);   glVertex2f(advance, 0.0f);      
			glTexCoord2f(maxX, maxY);   glVertex2f(advance, charHeight);   
			glTexCoord2f(minX, maxY);   glVertex2f(0.0f, charHeight);
		glEnd();
		
		glTranslatef(advance + 1, 0.0f, 0.0f);
		glEndList();

		if(col >= 10) {
			col = 0;
			row++;
		}else{
			col++;  
		}
	
	}
	
	for(unsigned int i = 0; i < glyphCache.size(); i++)
	   SDL_FreeSurface(glyphCache[i]);
	   
	SDL_FreeSurface(destination); 
	TTF_Quit();
}

GLuint getList(char c) {
   return chars[c - OFFSET];				
}

char message[512];
  
void App::writeText(int x, int y, const char *fmt, ...){

	glBindTexture(GL_TEXTURE_2D, textureId);   

  	va_list args;
    va_start(args, fmt);
    vsnprintf(message, 512, fmt, args);
    va_end(args);

	glPushMatrix();
	
	glTranslatef(x, y, 0);
	glScalef(0.5f, 0.5f, 0.5f);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
	
	unsigned int len = strlen(message);

	for(unsigned int i = 0; i < len; i++){
	   glCallList(getList(message[i]));
	   
	}
	
	glPopMatrix();
		
}


void App::writeTextCentered(int x, int y, const char *fmt, ...){

	glBindTexture(GL_TEXTURE_2D, textureId);   

  	va_list args;
    va_start(args, fmt);
    vsnprintf(message, 512, fmt, args);
    va_end(args);

	glPushMatrix();

	glTranslatef(x, y, 0);
	
	//center it
	int len = strlen(message);
	int offset = 0;
	
	for(int i=0;i<len;i++){
		offset += charWidths[message[i]];
	}	
	offset /= 2;	
	
	glScalef(0.5f, 0.5f, 0.5f);
	glTranslatef(offset, 0, 0);	

	for(unsigned int i = 0; i < strlen(message); i++){
	   glCallList(getList(message[i]));
	   
	}
	
	glPopMatrix();
	
}





