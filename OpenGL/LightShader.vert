#version 330 core

layout (location = 0) in vec3 aPositionVertex;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;


uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;


out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

void main()
{
	Normal = normalize(mat3(transpose(inverse(modelMatrix))) * aNormal);  
	FragPos = vec3(modelMatrix * vec4(aPositionVertex, 1.0));
	TexCoords = aTexCoords;

	gl_Position = projectionMatrix * viewMatrix * vec4(FragPos, 1.0);
} 