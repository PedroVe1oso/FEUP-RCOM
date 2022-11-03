// Link layer protocol implementation

#include "../include/link_layer_utils.h"

long total = 0,just_data=0, aux=0, total_aux=0;
struct timespec ts_start, ts_end, just_data_start, just_data_end, aux_start, aux_end;

void Restore_Global_Variables(){ // important to reiniciate alarm and STOP to be used again

    STOP = FALSE;
    flagAttempts=1;
    alarmEnabled=1;
    flag_error=0;

}

void Alarm_Handler(){

    stats.Total_Lost_Frames++; // frame enviado mas foi perdido

    if(flagAttempts > ll.nRetransmissions){
        flag_error = 1;
        flagAttempts=0;
    }

    flagAttempts++;
    if(!flag_error) stats.Total_Retransmissions++; // frame que vai ser retransmitido
    alarmEnabled = 1;

}

int Set_Serial_Port(LinkLayer connectionParameters){

    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0) {
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

    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
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

void Check_State_Machine(unsigned char word, int *state, unsigned char *compare){

    int Sequence_Variable = SequenceNumber ^ 0x01;

    switch(*state){
        case 0:
            if(word==compare[0]){ // FLAG
                *state = 1;
            }
            break;
        case 1:
            if(word == compare[1]){ // A_T or A_R
                *state = 2;
            }
            else if(word != compare[0]){
                *state = 0;
            }
            break;
        case 2:
            if(word == compare[2]){ // C_SET C_DISC C_UA // C_RR C_REJ C_S
                *state = 3;
            }
            else if(word == (C(Sequence_Variable))){
                REPLICA = 1;
            }
            else if(word == C_REJ(SequenceNumber)){
                printf("REJ RECEIVED - RETRANSMITTING\n");
                alarm(0); // Cancela o alarme
                Alarm_Handler(); // Chama a funcao alarme
                stats.Total_Retransmissions++;
            }
            else if(word == compare[0]){
                *state = 1;
            }
            else{
                *state = 0;
            }
            break;
        case 3:
            if(word == compare[3]){ // BCC1
                if(word == (A_T ^ C(SequenceNumber))){ // particular case where check of FLAG not needed - TYPE I FRAME
                    *state = 0;
                    STOP = TRUE;
                }
                else *state = 4;
            }
            else if(word == compare[0]){
                *state = 1;
            }
            else if(word == (A_R ^ C_REJ(SequenceNumber))){
                alarmEnabled = 1;
            }
            else{
                *state = 0;
            }
            break;
        case 4:
            if(word == compare[4]) STOP  = TRUE; // FLAG
            *state = 0;
            break;
    }

}

int Send_And_Receive_Frame(unsigned char*sendThis, int sendSize, unsigned char *receivedThis){

    unsigned char word;
    int state = 0;
    int res;
    STOP = FALSE;
    unsigned char BCC1;
    unsigned char BCC2;

    while(flagAttempts <= ll.nRetransmissions && alarmEnabled == 1){

        //PROBABILITY SET TO 0 -> CHANGE TO IMPLEMENT ERRORS!
        if(number_for_errors == 0){
            number_for_errors = (rand() % (100 - 1 + 1)) + 1; // 100 (upper) e 1(lower) NOLINT(cert-msc50-cpp)
            if(number_for_errors <= PROBABILITY){ // PROBABILITY in % so if the number is lower then 11 (1-10 = 10% in 100) we have 10% probability
                BCC1 = sendThis[3];
                BCC2 = sendThis[sendSize-1];
                sendThis[3] = 0x0F; // ERROR IN BCC1
                sendThis[sendSize-1] = 0x0F;  // ERROR IN BCC2
            }
            else number_for_errors = -1;
        }
        else if(number_for_errors > 0){ // REJ RECEIVED -> RETRANSMITTING WITH CORRECT VALUES
            number_for_errors = -1;
            sendThis[3] = BCC1;
            sendThis[sendSize-1] = BCC2;
        }


        res = write(fd,sendThis,sendSize);
        if(res < 0){
            perror("Error writing to serial port\n");
            return -1;
        }
        if(type_error == 2){
            sendThis[1] = A_T;
            type_error = -1;
        }
        else if(type_error == 3){
            sendThis[sendSize-2] ^= 0x0F;
            type_error = -1;
        }


        stats.Total_Transmissions++;
        alarm(ll.timeout);

        alarmEnabled = 0;
        state = 0;
        // se receber direito Check_State_Machine faz STOP = TRUE e alarmEnabled = 0 logo sai do while "exterior"
        while(STOP == FALSE && alarmEnabled == 0){
            res = read(fd,&word,1);
            if(res < 0){
                perror("Error reading command from serial port");
                return -1;
            }
            else if(res>0){
                //printf("Serial read() - %02x\n", word);
                Check_State_Machine(word,&state,receivedThis); // read from RECEIVER, side = 1
                if(STOP == TRUE && type_error == 4){
                    STOP = FALSE;
                    alarmEnabled = 1;
                    flagAttempts++;
                    type_error = -1;
                }
            }
            // Caso nao receba nada -> ESPERA TIMEOUT ALARME
        }
    }


    if(flag_error == 1){ // podia-se usar STOP  != TRUE -> so fica True quando recebe tudo corretamente
        Restore_Global_Variables();
        return -1;
    }
    else if(STOP == TRUE){
        stats.Total_Receptions++;
        alarm(0);  // CANCELA ALARME
        Restore_Global_Variables();
        return TRUE;
    }
}

int Open_Transmitter(){

    unsigned char SET[] = {FLAG,A_T,C_SET,A_T ^ C_SET,FLAG};
    unsigned char UA_R[] = {FLAG,A_T,C_UA,A_T ^ C_UA,FLAG};


    // Implement function to send SET and check the UA received
    if(Send_And_Receive_Frame(SET,5,UA_R) == TRUE){
        return TRUE;
    }

    return -1;

}

int Open_Receiver(){

    unsigned char word;
    int state=0;

    unsigned char SET[] = {FLAG,A_T,C_SET,A_T ^ C_SET,FLAG};
    int res = 0;

    while(STOP  == FALSE){
        res = read(fd,&word,1); // read 1 word
        if(res>0){
            Check_State_Machine(word,&state,SET);
        }
        else if(res < 0){
            perror("Error writing to serial port");
            return -1;
        }
    }

    total = clock_gettime(CLOCK_MONOTONIC, &ts_start); // time of receiver begins after receive the 1st frame

    if(STOP  == TRUE){
        stats.Total_Receptions++;
        Restore_Global_Variables();
        unsigned char UA_R[] = {FLAG,A_T,C_UA,A_T ^ C_UA,FLAG};

        if(write(fd,UA_R,5) < 0){
            perror("Error writing to serial port");
            return -1;
        }
        stats.Total_Transmissions++;
        return TRUE;
    }

    printf("ERROR DETECTED: SET NOT RECEIVED\n");
    GENERAL_ERROR = 1; // CANT COMUNICATE WITH RECEIVER SO CLOSE THE CONNECTION
    return -1;

}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    // First -> Set Serial Port
    fd = Set_Serial_Port(connectionParameters);

    // Ativa o alarm, alarmEnabled = 1 e dá update ao número de tentativas de conexão
    (void) signal(SIGALRM, Alarm_Handler);


    // CHECK TRANSMISSION

    int return_value;
    ll = connectionParameters;

    if(ll.role == LlTx){ // Transmitter
        return_value = Open_Transmitter();
    }
    else if(ll.role == LlRx){ // Receiver
        return_value = Open_Receiver();
    }
    else {
        return -1;
    }

    // RETURN THE RESULT

    if(return_value == TRUE){
        Restore_Global_Variables();
        return 0;
    }
    else{
        return -1;
    }
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{

    if(buf == NULL){
        return -1;
    }

    if(bufSize > MAX_PAYLOAD_SIZE){
        return -1;
    }


    aux = clock_gettime(CLOCK_MONOTONIC, &aux_start);


    unsigned char BCC2 = buf[0];
    for(int i = 1; i <bufSize; i++) BCC2 = BCC2 ^ buf[i];

    unsigned char *BCC2_buffer;
    int BCC2_buffer_size = 2;

    if(BCC2 == FLAG || BCC2 == ESC){ // verificar byte stuffing do BCC2
        BCC2_buffer = (unsigned char*)malloc(3);
        BCC2_buffer[0] = ESC;
        BCC2_buffer[1] = BCC2 ^ 0x20;
        BCC2_buffer[2] = FLAG;
        BCC2_buffer_size = 3;
    }
    else{ // no need for byte stuffing
        BCC2_buffer = (unsigned char*)malloc(2);
        BCC2_buffer[0] = BCC2;
        BCC2_buffer[1] = FLAG;
    }

    unsigned char *buffer_with_byteStuffing =  (unsigned char *)malloc(bufSize*2);
    int buffer_with_byteStuffing_size = 0;

    for(int i = 0; i<bufSize; i++){
        if(buf[i] == FLAG || buf[i] == ESC){
            buffer_with_byteStuffing[buffer_with_byteStuffing_size+1] = buf[i] ^ 0x20;
            buffer_with_byteStuffing[buffer_with_byteStuffing_size] = ESC;
            buffer_with_byteStuffing_size++;
        }
        else{
            buffer_with_byteStuffing[buffer_with_byteStuffing_size] = buf[i];
        }
        buffer_with_byteStuffing_size++;
    }
    buffer_with_byteStuffing = (unsigned char*) realloc(buffer_with_byteStuffing,buffer_with_byteStuffing_size);


    int buffer_encapsulated_size = buffer_with_byteStuffing_size + BCC2_buffer_size + 4;
    unsigned char *buffer_encapsulated = (unsigned char *)malloc(buffer_encapsulated_size);

    unsigned char header[] = {FLAG, A_T, C(SequenceNumber) ,A_T ^ C(SequenceNumber)};


    int i = 0;
    while((i<4) || (i<buffer_with_byteStuffing_size)){ // PREENCHIMENTO DO BUFFER
        if(i<4) {buffer_encapsulated[i] = header[i];}
        if(i< buffer_with_byteStuffing_size) {buffer_encapsulated[i+4] = buffer_with_byteStuffing[i];}
        if(i< BCC2_buffer_size) {buffer_encapsulated[i+4+buffer_with_byteStuffing_size] = BCC2_buffer[i];}
        i++;
    }

    stats.Total_bytes_sent += sizeof(unsigned char)*buffer_encapsulated_size;

    int Sequence_Variable = SequenceNumber ^ 0x01;

    unsigned char RR[] = {FLAG,A_T,C_RR(Sequence_Variable),A_T ^ C_RR(Sequence_Variable),FLAG};

    // JUST FOR ERROR CHECK


    if(type_error == 0){

        printf("Select the type of error in the next frame: \n");

        printf("1 - No error \n");
        printf("2 - Error in Header -> Test timeout and ability to receiver understand an error \n");
        printf("3 - Error in Data field -> Test the calculus of BCC2 \n");
        printf("4 - Replica -> ability to receiver understand replicas \n");

        scanf("%d",&type_error);


        just_data = clock_gettime(CLOCK_MONOTONIC, &just_data_start);

        if(type_error != 1 && type_error < 5) printf("ERROR SELECTED -> %d \n",type_error);

        switch(type_error){
            case 1:
                type_error = -1;
                break;
            case 2:
                buffer_encapsulated[1] = 0x0F;
                break;
            case 3:
                buffer_encapsulated[buffer_encapsulated_size-2] ^= 0x0F;
                break;
            case 4:
                type_error = 4;
                break;
            default:
                type_error = -1;
                break;
        }
        fflush(stdout);
    }


    aux = clock_gettime(CLOCK_MONOTONIC, &aux_end);
    total_aux += (aux_start.tv_nsec - aux_end.tv_nsec);

    stats.Total_I_Frames++;
    if(Send_And_Receive_Frame(buffer_encapsulated,buffer_encapsulated_size,RR) == TRUE){
        free(buffer_with_byteStuffing);
        free(BCC2_buffer);
        SequenceNumber ^= 0x01; // XOR with 1 to alternate between 0 and 1 -> for the next frame
        stats.Total_bytes_sent_without_losses += sizeof(unsigned char)*buffer_encapsulated_size;
        free(buffer_encapsulated);
        return sizeof(unsigned char)*buffer_encapsulated_size; // return number of bytes written
    }
    else{
        printf("MAX TRANSMISSIONS ACHIEVED -> IGNORE FROM NOW \n");
        GENERAL_ERROR = 1; // CANT COMUNICATE WITH RECEIVER -> SO CLOSE CONNECTION IN A RAPID WAY
        return -1;
    }

}

int Receive_Data(char *packet){

    int res, state = 0;
    int FLAG_FOUND = FALSE;
    int stuffing_detected = 0, buf_SIZE=0;
    int count_words = 0;

    unsigned char word, BCC2_received;
    unsigned char header[] = {FLAG, A_T, C(SequenceNumber) ,A_T ^ C(SequenceNumber)};
    unsigned char *auxiliar = (unsigned char *)malloc(MAX_PAYLOAD_SIZE);

    STOP = FALSE;

    while(FLAG_FOUND == FALSE){
        res = read(fd,&word,1);
        count_words++;
        if(res < 0){
            perror("Error writing to serial port");
            return -1;
        }
        else if(res > 0 && STOP == FALSE){
            Check_State_Machine(word,&state,header); // se receber mal header -> paro de ler logo sai do while...pode trazer atrasos corrigir se necessario
            if(REPLICA) break; // CASO REPLICA
            if(word == FLAG) count_words=0;
            if(count_words > 4) break;
        }
        else if(res > 0 && STOP == TRUE){ // HEADER CONFIRMED and STATE = 0;
            if(word == FLAG){ // last Byte
                FLAG_FOUND = TRUE;
                BCC2_received = auxiliar[state-1];
                buf_SIZE = state-1;
            }
            else if(word == ESC){ // byte stuffing...
                stuffing_detected = 1;
            }
            else if(stuffing_detected == 1){ // previous byte was stuffed
                auxiliar[state] = word ^ 0x20;
                stuffing_detected = 0;
                state++;
            }
            else{ // ELE SE NAO HOUVER FLAG FICA PRESO AQUI a espera dela
                auxiliar[state] = word;
                state++;
            }
        }
    }

    if(!(REPLICA || STOP) == 1){
        free(auxiliar);
        return 0;
    }

    unsigned char BCC2_calculated = 0;

    for(int i = 0; i<buf_SIZE;i++) BCC2_calculated = BCC2_calculated ^ auxiliar[i];

    int compare = (BCC2_calculated == auxiliar[buf_SIZE]);

    if( (compare==1) || REPLICA == 1){
        if(REPLICA){
            REPLICA = 0;
            stats.Total_Replicates++;
            printf("REPLICA FOUND\n");
        }
        else{
            SequenceNumber ^= 0x01; // PASSA DE 0 a 1 pela primeira vez se enviar corretamente
            for(int i = 0; i< buf_SIZE;i++){
                packet[i] = auxiliar[i];
            }
        }

        free(auxiliar);

        unsigned char RR[] = {FLAG,A_T,C_RR(SequenceNumber),A_T ^ C_RR(SequenceNumber),FLAG}; // Receiver Ready / positive ACK
        if(write(fd,RR,5) < 0){
            //perror("Error writing to serial port");
            return -1;
        }

        stats.Total_Receptions++;
        stats.Total_RR_sent++;
        stats.Total_bytes_sent+=5;
        stats.Total_Transmissions++;
        return sizeof(unsigned char)*buf_SIZE;
    }

    free(auxiliar);
    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{

    if(packet == NULL){
        return -1;
    }


    aux = clock_gettime(CLOCK_MONOTONIC, &aux_start);

    if(type_error == 0){
        just_data = clock_gettime(CLOCK_MONOTONIC, &just_data_start);
        type_error = -1;
    }

    int bytes_read;

    while(1){
        bytes_read = Receive_Data(packet);
        if(bytes_read > 0){
            aux = clock_gettime(CLOCK_MONOTONIC, &aux_end);
            total_aux += aux_start.tv_nsec - aux_end.tv_nsec;
            return bytes_read;
        }
        else if (bytes_read == 0){
            if(STOP == TRUE){
                printf("ERROR DATA FIELD -> SENDING A REJ FRAME\n");
                stats.Total_REJ_sent++;
                unsigned char REJ[] = {FLAG,A_T,C_REJ(SequenceNumber),A_T ^ C_REJ(SequenceNumber),FLAG}; // Reject / negative ACK
                if(write(fd,REJ,5) < 0){
                    perror("Error writing to serial port");
                    return -1;
                }
                stats.Total_bytes_sent+=5;
                stats.Total_Retransmissions++;
            }
        }
        else return -1;
    }
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{


    if(GENERAL_ERROR) return -1; // RAPID WAY OF CLOSING CONNECTION

    just_data = clock_gettime(CLOCK_MONOTONIC, &just_data_end);


    if(ll.role == LlRx){
        if(Close_Receiver() == TRUE){
            if(showStatistics) Print_Statistics();
            return TRUE;
        }
    }
    else if(Close_Transmitter() == TRUE){
        if(showStatistics) Print_Statistics();
        return TRUE;
    }

    return -1;
}

int Close_Transmitter(){

    // envia e recebe DISC, caso tudo corra bem envia UA
    unsigned char DISC_T[] = {FLAG, A_T, C_DISC, A_T ^ C_DISC, FLAG};
    unsigned char DISC_R[] = {FLAG, A_R, C_DISC, A_R ^ C_DISC, FLAG};


    if(Send_And_Receive_Frame(DISC_T,5,DISC_R) == TRUE){
        unsigned char UA_T[] = {FLAG,A_R,C_UA,A_R ^ C_UA,FLAG}; // enviado pelo transmissor
        if(write(fd,UA_T,5) < 0){
            perror("Error writing to serial port");
            return -1;
        }
        stats.Total_Transmissions++;
        return TRUE;
    }

    return -1;
}

int Close_Receiver(){

    unsigned char word;

    unsigned char DISC_T[] = {FLAG, A_T, C_DISC, A_T ^ C_DISC, FLAG};
    unsigned char DISC_R[] = {FLAG, A_R, C_DISC, A_R ^ C_DISC, FLAG};
    unsigned char UA_T[] = {FLAG,A_R,C_UA,A_R ^ C_UA,FLAG}; // enviado pelo transmissor
    int state=0,res=0;
    STOP = FALSE;

    // recebe DISC
    while(STOP  == FALSE){
        res = read(fd,&word,1); // read 1 word
        if(res>0){
            Check_State_Machine(word,&state,DISC_T);
        }
        else return -1;
    }
    stats.Total_Receptions++;

    STOP = FALSE;
    res = write(fd,DISC_R,5);
    if(res < 0 ) return -1;

    stats.Total_Transmissions++;

    while(STOP == FALSE){
        res = read(fd,&word,1); // read 1 word
        if(res>0){
            Check_State_Machine(word,&state,UA_T);
        }
        else return -1;
    }
    stats.Total_Receptions++;

    return TRUE;
}

void Print_Statistics(){


    printf(" ---------- STATISTICS ----------\n");

    //end time before call function
    total = clock_gettime(CLOCK_MONOTONIC, &ts_end);


    printf("Total time of the conection = %ld seconds\n", ts_end.tv_sec - ts_start.tv_sec);


    if(ll.role == LlTx) printf("Total time of encapsulating and stuffing the data = %.2f seconds\n", total_aux/1E9 );
    else if(ll.role == LlRx) printf("Total time of extracting the data from the frame = %.2f seconds\n", total_aux/1E9 );


    printf("Total Transmissions: %d\n",stats.Total_Transmissions);
    printf("Total Receptions: %d\n",stats.Total_Receptions);
    printf("Total Retransmissions: %d\n",stats.Total_Retransmissions);
    printf("Total Frames Lost: %d\n",stats.Total_Lost_Frames);
    printf("Total Replicas Received: %d\n",stats.Total_Replicates);


    if(ll.role == LlTx) {
        printf("Total I Frames Sent: %d\n",stats.Total_I_Frames);
        printf("Total bytes sent: %d\n",stats.Total_bytes_sent);
        printf("Total bytes sent with confirmation: %d\n",stats.Total_bytes_sent_without_losses);
        printf("In %d bytes sent, %d %% were received \n",stats.Total_bytes_sent,(100*stats.Total_bytes_sent_without_losses/stats.Total_bytes_sent));
    }
    else if(ll.role == LlRx){
        printf("Total RR Frames Sent: %d\n",stats.Total_RR_sent);
        printf("Total REJ Frames Sent: %d\n",stats.Total_REJ_sent);
    }
    printf("---------- Closing Connection ----------\n");
}