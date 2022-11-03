#ifndef _LINK_LAYER_UTILS_H_
#define _LINK_LAYER_UTILS_H_

#include "link_layer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


// Delimitação e cabeçalho

// Todas as tramas são delimitadas por flags ( 01111110 )
#define FLAG 0x7E // 01111110
// ESC é usado para substituir um octeto idêntico à FLAG que exista no interior da trama
#define ESC         0x7D    // 01111101

// Campo de Endereço
#define A_T         0x03    // 00000011 Comandos enviados pelo Transmissor e Respostas enviadas pelo Receptor (T->R) -> (R->T)
#define A_R         0x01    // 00000001 Comandos enviados pelo Receptor e Respostas enviadas pelo Transmissor (R->T) -> (T->R)

// Campos de controlo

#define C(S) (S<<1) // C Control field 0 0 0 0 0 0 S 0 -> sequence numbers in I frames = N(s)

// Tramas I, SET e DISC são designadas Comandos e as restantes (UA, RR e REJ) Respostas
#define C_SET       0x03    // 00000011 set up
#define C_DISC      0x0B    // 00001011 disconnect
#define C_UA        0x07    // 00000111 unnumbered acknowledgment
#define C_RR(R) ((R<<5) + 1) // RR (receiver ready / positive ACK) - 0 0 R 0 0 0 0 1
#define C_REJ(R) ((R<<5) + 5) //  REJ (reject / negative ACK) - 0 0 R 0 0 1 0 1

#define PROBABILITY 0

typedef struct{

    int Total_Transmissions;
    int Total_Receptions;
    int Total_Retransmissions;
    int Total_Lost_Frames;
    int Total_bytes_sent;
    int Total_bytes_sent_without_losses;
    int Total_Replicates;
    int Total_RR_sent;
    int Total_REJ_sent;
    int Total_I_Frames;

} Statistics;


LinkLayer ll;
Statistics stats;

int REPLICA=0;
int fd;
int SequenceNumber = 0;
int GENERAL_ERROR = 0;
int STOP = FALSE;
int type_error = 0;
int number_for_errors = 0;

// alarm 
unsigned char flagAttempts=1;
unsigned char alarmEnabled=1;
unsigned char flag_error=0;


//Restore the global variables in order tu use in the next transmissions
void Restore_Global_Variables();

//Alarm function handler
void Alarm_Handler();

//Set serial port
int Set_Serial_Port(LinkLayer connectionParameters);

//Send a frame (sendThis) wait for the expected response (receivedThis)
int Send_And_Receive_Frame(unsigned char*sendThis, int sendSize, unsigned char *receivedThis);

//Open the connection for the transmitter
int Open_Transmitter();

//Open the connection for the receiver
int Open_Receiver();

//Receive , Destuff, Encapsulate and Analyse the existence errors of the data
int Receive_Data(char *packet);

//Close the connection for Transmitter
int Close_Transmitter();

//Close the connection for Receiver
int Close_Receiver();

//Print the statistics in the end of the connection
void Print_Statistics();

#endif // _LINK_LAYER_UTILS_H_