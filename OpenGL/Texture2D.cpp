#pragma once

#include "Texture2D.h"

Texture2D::Texture2D() {}

Texture2D::Texture2D(const GLchar* texturePath) : Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), Filter_Min(GL_LINEAR_MIPMAP_LINEAR), Filter_Max(GL_LINEAR)
{
	glGenTextures(1, &TextureID);

	int nrComponents;
	unsigned char *data = stbi_load(texturePath, &Width, &Height, &nrComponents, 0);
	if (data)
	{
		if (nrComponents == 1)
			Image_Format = GL_RED;
		else if (nrComponents == 3)
			Image_Format = GL_RGB;
		else if (nrComponents == 4)
			Image_Format = GL_RGBA;

		Internal_Format = Image_Format;
		glBindTexture(GL_TEXTURE_2D, TextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, Internal_Format, Width, Height, 0, Image_Format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_Min);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_Max);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << texturePath << std::endl;
		stbi_image_free(data);
	}
}

Texture2D::~Texture2D() {}

void Texture2D::Bind() const
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);
}