/*

05_uv2color.frag: shader a falsi colori, usa i valori di U e V come canali R e G del frammento

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/

#version 330 core

// variabile di output dello shader
out vec4 colorFrag;

// il valore è stato calcolato per-vertex nel vertex shader
in vec2 interp_UV;


void main(){
	// uso le coordinate UV come canali R e G
	colorFrag = vec4(interp_UV,0.0,1.0); 
}