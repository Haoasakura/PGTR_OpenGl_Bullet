/*

04_wave.vert: Vertex shader, crea un effetto di deformazione ciclico sui vertici del solido, utilizzando
    la funzione seno, applicata al tempo di esecuzione dell'applicazione.
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

// l'applicazione deve passare il tempo di esecuzione: definisco una variabile uniform, che verrà
// passata allo shader dall'applicazione
uniform float timer;

// l'applicazione deve passare il peso da applicare al displacement
uniform float weight;

// la normale trasformata viene impostata come valore di output, da "passare" al fragment shader.
// questo significa che i valori delle normali nei vertici verranno interpolate su tutti i frammenti generati in fase
// di rasterizzazione tra un vertice e l'altro.
out vec3 N;


void main()
{
	// modifico le posizioni dei vertici a partire dalla posizione originale,
	// modificata da un valore calcolato a partire dal tempo di esecuzione
	// In questo modo ottengo un effetto "wobble" alternato.
	
	// spiazzamento su base del tempo, pesato e modificato da 2 pesi
	float disp = weight*sin(timer)+weight;
	// la vecchia posizione è modificata di disp lungo la normale
	vec3 newPos = position + disp*normal;

	// applica trasformazioni al vertice
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(newPos, 1.0f);
	// applico le trasformazioni alle normali
    N = normalize(normalMatrix * normal);
}
