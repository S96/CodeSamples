#include "GraphicsObjectTerrain.h"


#include "DeltA_Assets.h"


GraphicsObjectTerrain::GraphicsObjectTerrain(Model * model, Texture* deform_tex, Texture * pTex, GLfloat t0_max, Vect & LightColor, Vect & LightPos)
	: GraphicsObject(model, ShaderMngr::GetShader(ShaderMngr::TERRAIN))
{
	assert(pTex);
	this->textureID = pTex->getTextureID();
	this->textureID2 = 0;
	this->textureID3 = 0;
	this->textureID4 = 0;
	this->t0_max = t0_max;
	this->texture_deform = deform_tex->getTextureID();
	this->pLightColor = new Vect(LightColor);
	this->pLightPos = new Vect(LightPos);
}

GraphicsObjectTerrain::GraphicsObjectTerrain(Model * model, Texture* deform_tex, Texture * pTex, GLfloat t0_max, GLfloat t1_min, Texture * pTex2, GLfloat t1_max, Vect & LightColor, Vect & LightPos)
	: GraphicsObject(model, ShaderMngr::GetShader(ShaderMngr::TERRAIN))
{
	assert(pTex);
	assert(pTex2);
	assert(t0_max <= t1_min);
	this->textureID = pTex->getTextureID();
	this->t0_max = t0_max;
	this->t1_min = t1_min;
	this->textureID2 = pTex2->getTextureID();
	this->textureID3 = 0;
	this->textureID4 = 0;
	this->t1_max = t1_max;
	this->texture_deform = deform_tex->getTextureID();
	this->pLightColor = new Vect(LightColor);
	this->pLightPos = new Vect(LightPos);
}

GraphicsObjectTerrain::GraphicsObjectTerrain(Model * model, Texture* deform_tex, Texture * pTex, GLfloat t0_max, GLfloat t1_min, Texture * pTex2, GLfloat t1_max, GLfloat t2_min, Texture * pTex3, GLfloat t2_max, Vect & LightColor, Vect & LightPos)
	: GraphicsObject(model, ShaderMngr::GetShader(ShaderMngr::TERRAIN))
{
	assert(pTex);
	assert(pTex2);
	assert(pTex3);
	assert(t0_max <= t1_min);
	assert(t1_min <= t1_max);
	assert(t1_max <= t2_min);
	this->textureID = pTex->getTextureID();
	this->t0_max = t0_max;
	this->t1_min = t1_min;
	this->textureID2 = pTex2->getTextureID();
	this->t1_max = t1_max;
	this->t2_min = t2_min;
	this->textureID3 = pTex3->getTextureID();
	this->textureID4 = 0;
	this->t2_max = t2_max;
	this->texture_deform = deform_tex->getTextureID();
	this->pLightColor = new Vect(LightColor);
	this->pLightPos = new Vect(LightPos);
}

GraphicsObjectTerrain::GraphicsObjectTerrain(Model * model, Texture* deform_tex, Texture * pTex, GLfloat t0_max, GLfloat t1_min, Texture * pTex2, GLfloat t1_max, GLfloat t2_min, Texture * pTex3, GLfloat t2_max, GLfloat t3_min, Texture * pTex4, Vect & LightColor, Vect & LightPos)
	: GraphicsObject(model, ShaderMngr::GetShader(ShaderMngr::TERRAIN))
{
	assert(pTex);
	assert(pTex2);
	assert(pTex3);
	assert(pTex4);
	assert(t0_max <= t1_min);
	assert(t1_min <= t1_max);
	assert(t1_max <= t2_min);
	assert(t2_min <= t2_max);
	assert(t2_max <= t3_min);
	this->textureID = pTex->getTextureID();
	this->t0_max = t0_max;
	this->t1_min = t1_min;
	this->textureID2 = pTex2->getTextureID();
	this->t1_max = t1_max;
	this->t2_min = t2_min;
	this->textureID3 = pTex3->getTextureID();
	this->t2_max = t2_max;
	this->t3_min = t3_min;
	this->textureID4 = pTex4->getTextureID();
	this->texture_deform = deform_tex->getTextureID();
	this->pLightColor = new Vect(LightColor);
	this->pLightPos = new Vect(LightPos);
}

GraphicsObjectTerrain::~GraphicsObjectTerrain()
{
	delete this->pLightColor;
	delete this->pLightPos;
}

void GraphicsObjectTerrain::privSetState()
{
	// Bind the textures	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureID2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureID3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textureID4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texture_deform);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
}

void GraphicsObjectTerrain::privSetDataGPU(Camera * pCam)
{
	// Get camera
	assert(pCam->getType() == Camera::Type::PERSPECTIVE_3D);

	// set the vao
	glBindVertexArray(this->getModel()->getvao());

	// use this shader
	this->pShaderObj->SetActive();

	Matrix world = this->getWorld();
	Matrix view = pCam->getViewMatrix();
	Matrix proj = pCam->getProjMatrix();

	glUniformMatrix4fv(this->pShaderObj->GetLocation("proj_matrix"), 1, GL_FALSE, (float *)&proj);
	glUniformMatrix4fv(this->pShaderObj->GetLocation("view_matrix"), 1, GL_FALSE, (float *)&view);
	glUniformMatrix4fv(this->pShaderObj->GetLocation("world_matrix"), 1, GL_FALSE, (float *)&world);

	glUniform3fv(this->pShaderObj->GetLocation("vLightPos"), 1, (float *)this->pLightPos);
	glUniform4fv(this->pShaderObj->GetLocation("vColor"), 1, (float *)this->pLightColor);

	glUniform1f(this->pShaderObj->GetLocation("t0_max"), t0_max);
	glUniform1f(this->pShaderObj->GetLocation("t1_min"), t1_min);
	glUniform1f(this->pShaderObj->GetLocation("t1_max"), t1_max);
	glUniform1f(this->pShaderObj->GetLocation("t2_min"), t2_min);
	glUniform1f(this->pShaderObj->GetLocation("t2_max"), t2_max);
	glUniform1f(this->pShaderObj->GetLocation("t3_min"), t3_min);

	GLint a[5];
	a[0] = 0;
	a[1] = 1;
	a[2] = 2;
	a[3] = 3;
	a[4] = 4;
	glUniform1iv(this->pShaderObj->GetLocation("tex_object[0]"), 5, a);
}

void GraphicsObjectTerrain::privDraw()
{
	//The draw triangles by index
	glDrawElements(GL_TRIANGLES, 3 * this->getModel()->getTriNum(), GL_UNSIGNED_INT, 0);   //The starting point of the IBO
}

void GraphicsObjectTerrain::privRestoreState()
{
	// undo the states you corrupted
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
}
