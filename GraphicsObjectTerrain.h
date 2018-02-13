#ifndef GRAPHICS_OBJECT_TERRAIN_H
#define GRAPHICS_OBJECT_TERRAIN_H


#include "GraphicsObject.h"
#include "Texture.h"


class GraphicsObjectTerrain :public GraphicsObject
{
public:
	// Default parameters with pointers in middle of arg list are non-conventional, opted for overloading technique
	GraphicsObjectTerrain(Model *model, Texture* deform_tex, Texture *pTex, GLfloat t0_max, Vect &LightColor, Vect &LightPos);
	GraphicsObjectTerrain(Model *model, Texture* deform_tex, Texture *pTex, GLfloat t0_max, GLfloat t1_min, Texture *pTex2, GLfloat t1_max, Vect &LightColor, Vect &LightPos);
	GraphicsObjectTerrain(Model *model, Texture* deform_tex, Texture *pTex, GLfloat t0_max, GLfloat t1_min, Texture *pTex2, GLfloat t1_max, GLfloat t2_min, Texture *pTex3, GLfloat t2_max, Vect &LightColor, Vect &LightPos);
	GraphicsObjectTerrain(Model *model, Texture* deform_tex, Texture *pTex, GLfloat t0_max, GLfloat t1_min, Texture *pTex2, GLfloat t1_max, GLfloat t2_min, Texture *pTex3, GLfloat t2_max, GLfloat t3_min, Texture *pTex4, Vect &LightColor, Vect &LightPos);

	~GraphicsObjectTerrain();

protected:
	virtual void privSetState() override;
	virtual void privSetDataGPU(Camera *pCam) override;
	virtual void privDraw() override;
	virtual void privRestoreState() override;

public:
	// data:  place uniform instancing here
	GLuint textureID;
	GLuint textureID2;
	GLuint textureID3;
	GLuint textureID4;
	GLuint texture_deform;

	GLfloat t0_max;
	GLfloat t1_min;
	GLfloat t1_max;
	GLfloat t2_min;
	GLfloat t2_max;
	GLfloat t3_min;

	Vect *pLightColor;
	Vect *pLightPos;

	GraphicsObjectTerrain() = delete;
};

#endif // !GRAPHICS_OBJECT_TERRAIN_H