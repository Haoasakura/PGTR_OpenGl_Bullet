#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 color;
uniform float mixRatio;
uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = mix(texture(texture_diffuse1, TexCoords), color, mixRatio);
}