/*

02_flatten.vert: Vertex shader, modifica una delle coordinate dei vertici, "schiacciando"
   il modello. 

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/

#version 330 core

// posizione vertice in coordinate mondo
layout (location = 0) in vec3 position;

// matrice di modellazione
uniform mat4 modelMatrix;
// matrice di vista
uniform mat4 viewMatrix;
// matrice di proiezione
uniform mat4 projectionMatrix;

void main()
{
	// Copio la posizione in una variabile locale, e azzero la coordinata Z
	vec3 flattened = position;
	flattened.z = 0.0;
	// applica trasformazioni alla variabile di apporggio
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( flattened, 1.0 );
}