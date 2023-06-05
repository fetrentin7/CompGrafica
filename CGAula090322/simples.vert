#version 330 core

layout (location = 0) in vec3 entrada_pos;
layout (location = 1) in vec2 entrada_pos_text;
layout (location = 2) in vec3 entrada_normal;

out vec2 pos_text;
out vec3 normal;
out vec3 posicao;

uniform float escala;

uniform float anguloRotacaoX;
uniform float anguloRotacaoY;
uniform float anguloRotacaoZ;

uniform float posicaoX;
uniform float posicaoY;

void main()
{
    pos_text = entrada_pos_text;
    normal = entrada_normal;
    posicao = entrada_pos * escala;
    gl_Position = vec4(entrada_pos * escala, 1.0);

    gl_Position.x += posicaoX;
    gl_Position.y += posicaoY;

    float aux1 = (cos(anguloRotacaoZ) * cos(anguloRotacaoY)) * gl_Position.x + ((cos(anguloRotacaoZ) * sin(anguloRotacaoY) * sin(anguloRotacaoX)) - (sin(anguloRotacaoZ) * cos(anguloRotacaoX))) * gl_Position.y + ((cos(anguloRotacaoZ) * sin(anguloRotacaoY) * cos(anguloRotacaoX)) + (sin(anguloRotacaoZ) * sin(anguloRotacaoX))) * gl_Position.z;
    float aux2 = (sin(anguloRotacaoZ) * cos(anguloRotacaoY)) * gl_Position.x + ((sin(anguloRotacaoZ) * sin(anguloRotacaoY) * sin(anguloRotacaoX)) + (cos(anguloRotacaoZ) * cos(anguloRotacaoX))) * gl_Position.y + ((sin(anguloRotacaoZ) * sin(anguloRotacaoY) * cos(anguloRotacaoX)) - (cos(anguloRotacaoZ) * sin(anguloRotacaoX))) * gl_Position.z;
    gl_Position.z = (-sin(anguloRotacaoY)) * gl_Position.x + (cos(anguloRotacaoY) * sin(anguloRotacaoX)) * gl_Position.y + (cos(anguloRotacaoY) * cos(anguloRotacaoX)) * gl_Position.z;
	gl_Position.x = aux1;
	gl_Position.y = aux2;
} 