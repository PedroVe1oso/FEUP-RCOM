// All helper functions will be declared here
//

#ifndef _UTILS_H_
#define _UTILS_H_

#include "../include/link_layer.h"


// Delimitação e cabeçalho

// Todas as tramas são delimitadas por flags ( 01111110 )
#define FLAG 0x7E // 01111110
// ESC é usado para substituir um octeto idêntico à FLAG que exista no interior da trama
#define ESC 0x7D // 01111101
#define ESC_FLAG 0x5E
#define ESC_ESC 0x5D

// A (Campo de Endereço)
#define A_T 0x03 // 00000011 Comandos enviados pelo Transmissor e Respostas enviadas pelo Receptor (T->R) -> (R->T)
#define A_R 0x01 // 00000001 Comandos enviados pelo Receptor e Respostas enviadas pelo Transmissor (R->T) -> (T->R)

#endif // _UTILS_H_