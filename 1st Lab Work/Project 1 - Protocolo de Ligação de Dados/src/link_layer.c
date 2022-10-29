// Link layer protocol implementation

#include <signal.h>
#include <time.h>

#include "../include/link_layer.h"
#include "../include/utils.h"
#include "../include/macro.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    // First -> Set Serial Port
    // TODO
    //  implement function to set serial port
    //  fd = Set_Serial_Port(connectionParameters);

    // Ativa o alarm, alarmEnabled = TRUE e dá update ao número de tentativas de conexão
    (void) signal(SIGALRM, alarmHandler);
    
    
    // CHECK TRANSMISSION 
    
    int return_value;
    ll = connectionParameters;

    if(ll.role == LlTx){ // Transmitter
        // TODO
        //  implement Open_Transmitter()
        //  return_value = Open_Transmitter();
    }
    else if(ll.role == LlRx){ // Receiver
        // TODO
        //  open receiver helper function
        //  return_value = Open_Receiver();
    }
    else {
        return -1;
    }
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    return 1;
}
