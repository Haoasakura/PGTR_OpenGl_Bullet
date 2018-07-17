/*

03_normal2color.vert: Vertex shader, usa le normali come colori da assegnare ai vertici.

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/
#version 330 core

// posizione vertice in coordinate mondo
layout (location = 0) in vec3 position;
// posizione vertice in coordinate mondo
layout (location = 1) in vec3 normal;

// matrice di modellazione
uniform mat4 modelMatrix;
// matrice di vista
uniform mat4 viewMatrix;
// matrice di proiezione
uniform mat4 projectionMatrix;

// matrice di trasformazione delle normali (= trasposta dell'inversa della model-view)
uniform mat3 normalMatrix;

// la normale trasformata viene impostata come valore di output, da "passare" al fragment shader.
// questo significa che i valori delle normali nei vertici verranno interpolate su tutti i frammenti generati in fase
// di rasterizzazione tra un vertice e l'altro.
out vec3 N;

void main()
{
	
	// applica trasformazioni al vertice
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
	// applico le trasformazioni alle normali
    N = normalize(normalMatrix * normal);
}