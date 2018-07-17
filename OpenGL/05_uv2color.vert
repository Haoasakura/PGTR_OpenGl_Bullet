/*

05_uv2color.vert: come 00_basic.vert, ma gestisce anche le coordinate UV. 
Riceve quelle del modello e le assegna a una variabile
   di output in modo da avere poi i valori interpolati per-fragment

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/

#version 330 core

// posizione vertice in coordinate mondo
layout (location = 0) in vec3 position;
// coordinate texture
layout (location = 2) in vec2 UV;

// matrice di modellazione
uniform mat4 modelMatrix;
// matrice di vista
uniform mat4 viewMatrix;
// matrice di proiezione
uniform mat4 projectionMatrix;

out vec2 interp_UV;

void main()
{
	// assegnando i valori per-vertex delle UV a una variabile con qualificatore "out", 
	// i valori verranno interpolati su tutti i frammenti generati in fase
	// di rasterizzazione tra un vertice e l'altro.
	interp_UV = UV;
	
	// applica trasformazioni al vertice
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
}
