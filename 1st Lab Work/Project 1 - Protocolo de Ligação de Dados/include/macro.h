//
// Created by pedro on 29-10-2022.
//

#ifndef _MACRO_H_
#define _MACRO_H_

// Delimitação e cabeçalho

// Todas as tramas são delimitadas por flags ( 01111110 )
#define FLAG 0x7E // 01111110
// ESC é usado para substituir um octeto idêntico à FLAG que exista no interior da trama
#define ESC         0x7D    // 01111101
#define ESC_FLAG    0x5E
#define ESC_ESC     0x5D

// Campo de Endereço
#define A_T         0x03    // 00000011 Comandos enviados pelo Transmissor e Respostas enviadas pelo Receptor (T->R) -> (R->T)
#define A_R         0x01    // 00000001 Comandos enviados pelo Receptor e Respostas enviadas pelo Transmissor (R->T) -> (T->R)

// Campos de controlo
// Tramas I, SET e DISC são designadas Comandos e as restantes (UA, RR e REJ) Respostas
#define C_SET       0x03    // 00000011 set up
#define C_DISC      0x0B    // 00001011 disconnect
#define C_UA        0x07    // 00000111 unnumbered acknowledgment

// Alarm variables
unsigned char flagAttempts = 0;
unsigned char alarmEnabled = FALSE;

#endif //_MACRO_H_
