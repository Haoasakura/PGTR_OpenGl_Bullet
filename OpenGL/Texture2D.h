#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "stb_image.h"

class Texture2D
{
public:

	unsigned int TextureID;
	int Width, Height;

	GLuint Internal_Format; 
	GLuint Image_Format; 
						 
	GLuint Wrap_S;
	GLuint Wrap_T;
	GLuint Filter_Min;
	GLuint Filter_Max;
					   
	Texture2D();
	Texture2D(const GLchar* texturePath);
	~Texture2D();

	void Bind() const;

};

