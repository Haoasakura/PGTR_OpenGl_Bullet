/*

01_fullcolor.frag : Fragment shader di base, assegna un colore uniforme a tutti i fragment. Il colore è passato dall'applicazione come uniform

NB) utilizzare "00_basic.vert" come vertex shader

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/

#version 330 core

in vec2 TexCord;
// variabile di output dello shader
out vec4 colorFrag;

// il colore da assegnare è passato dall'applicazione allo shader
uniform vec3 colorIn;
uniform sampler2D texture1;

void main()
{
    vec2 repeated_Uv = mod(TexCord, 1.0);
    // assegno il colore passato dall'applicazione al frammento
    colorFrag = texture(texture1, repeated_Uv) * vec4(colorIn,1.0);
}