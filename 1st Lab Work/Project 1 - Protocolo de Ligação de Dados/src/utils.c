//
// All helper functions definition
//

#include "../include/link_layer.h"
#include "../include/utils.h"
#include "../include/macro.h"

int Set_Serial_Port(LinkLayer connectionParameters){

    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd <0) {
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if ( tcgetattr(fd,&oldtio) == -1) {
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio,sizeof(&newtio));  // preenche a newtio
    newtio.c_cflag = DEFAULT_BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME]    = 10;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd,TCIOFLUSH);

    // Set new port settings
    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    return fd;

    //Serial Port is set
}

void alarmHandler(){

    if(flagAttempts > ll.nRetransmissions){
        flagAttempts=0;
    }

    flagAttempts++;
    alarmEnabled = TRUE;
}

int Open_Transmitter(){

    unsigned char SET[] = {FLAG,A_T,C_SET,A_T ^ C_SET,FLAG};
    unsigned char UA_R[] = {FLAG,A_T,C_UA,A_T ^ C_UA,FLAG};


    // Implement function to send SET and check the UA received
    if(/*Send_And_Receive_Frame(SET,5,UA_R) == TRUE*/){
        return TRUE;
    }

    return -1;

}

