//
// All helper functions definition
//

#include "../include/utils.h"
#include "../include/macro.h"

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

