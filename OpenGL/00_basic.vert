/*

00_basic.vert : Vertex shader di base

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/


#version 330 core

// posizione vertice in coordinate mondo
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 TexCoords;

out vec2 TexCord;

// matrice di modellazione
uniform mat4 modelMatrix;
// matrice di vista
uniform mat4 viewMatrix;
// matrice di proiezione
uniform mat4 projectionMatrix;

void main()
{
    // applica trasformazioni al vertice
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
    TexCord=TexCoords;
}