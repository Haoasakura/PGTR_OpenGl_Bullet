/*

04_wave.frag : Fragment shader, uguale a 03_normal2color.frag

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/
		
#version 330 core

// variabile di output dello shader
out vec4 colorFrag;

// il valore è stato calcolato per-vertex nel vertex shader
in vec3 N;


void main(){
	// uso la normale come colore
	colorFrag = vec4(N,1.0); 
}