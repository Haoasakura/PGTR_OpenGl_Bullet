/*

02_flatten.frag : Fragment shader, uguale a 01_fullcolor.frag

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/
		
#version 330 core
                
// il colore da assegnare è passato dall'applicazione allo shader
uniform vec3 colorIn;

// variabile di output dello shader
out vec4 colorFrag;

void main()
{    
    // assegno il colore passato dall'applicazione al frammento
    colorFrag = vec4(colorIn,1.0);
}