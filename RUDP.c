#include "RUDP_API.h"

typedef struct MessageHeader{
    unsigned short length;
    unsigned short checksum;
    char flags;//SYN/ACK/FIN
    char data[BUFFER_SIZE];
}Message;

// Sender Functions

/**
 * receive Ack from srcAddress
 * @param socket
 * @param srcAddress
 * @return -2: timeout, -1: error, 0: disconnected, 1: Received
 */
int rudp_receiveACK(int socket,struct sockaddr_in* srcAddress){
    Message buffer;
    memset(&buffer, 0, sizeof(Message));

    socklen_t srcAddressLen = sizeof(*srcAddress);
    int receiveACK = recvfrom(socket, &buffer, sizeof(Message), 0, (struct sockaddr *) &srcAddress,
                              &srcAddressLen);

    if (receiveACK == -1) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return -2;
        }
        else{
            printf("recvfrom() failed with error code : %d", errno);
            close(socket);
            return -1;
        }
    }
    if(receiveACK == 0){
        return 0;
    }

    if(buffer.flags == 'A'){
        return 1;
    }
    return 0;
}

/**
 * send connect and wait for ack, if not sent send again
 * @param socket
 * @param destAddress
 * @param srcAddress
 * @return -1: error, 0: disconnected, 1: received
 */
int rudp_connect(int socket,struct sockaddr_in* destAddress,struct sockaddr_in* srcAddress){
    Message SYN;
    memset(&SYN,0,sizeof(Message));
    SYN.length=0;
    strcpy(SYN.data,"");
    SYN.checksum = 0;
    SYN.flags = 'S';

    while(1) {

        int sendSYN = sendto(socket, &SYN, sizeof(Message), 0, (struct sockaddr *) destAddress, sizeof(*destAddress));
        if (sendSYN == -1) {
            printf("sendto() failed with error code  : %d", errno);
            close(socket);
            return -1;
        }

        int ACKresult = rudp_receiveACK(socket, srcAddress);
        if (ACKresult != -2) {
            return ACKresult;
        }

        printf("Timeout occurred, sending connect again\n");
    }
}
/**
 * send disconnect and wait for ack, if not sent send again
 * @param socket
 * @param destAddress
 * @param srcAddress
 * @return -1: error, 0: disconnected, 1: received
 */
int rudp_disconnect(int socket,struct sockaddr_in* destAddress,struct sockaddr_in* srcAddress){
    Message FIN;
    memset(&FIN,0,sizeof(Message));
    FIN.length=0;
    strcpy(FIN.data,"");
    FIN.checksum = 0;
    FIN.flags = 'F';

    while (1) {

        int sendFIN = sendto(socket, &FIN, sizeof(Message), 0, (struct sockaddr *) destAddress, sizeof(*destAddress));
        if (sendFIN == -1) {
            printf("sendto() failed with error code  : %d\n", errno);
            close(socket);
            return -1;
        }
        int ACKresult = rudp_receiveACK(socket, srcAddress);
        if (ACKresult != -2) {
            return ACKresult;
        }

        printf("Timeout occurred, sending disconnect again\n");
    }
}


/**
 * function send data to and wait to receive Ack if didnt get send again
 * @param socket
 * @param data
 * @param destAddress
 * @param srcAddress
 * @return -1: failure, 1: successful, 0: sender closed
 */
int rudp_sendData(int socket,char* data,struct sockaddr_in* destAddress,struct sockaddr_in* srcAddress){
    Message Data;
    memset(&Data,0,sizeof(Message));
    Data.length = sizeof(char)* strlen(data);
    strcpy(Data.data,data);
    Data.checksum = calculate_checksum(Data.data,Data.length);
    Data.flags = 'M';

   while(1) {

       int sendData = sendto(socket, &Data, sizeof(Message), 0, (struct sockaddr *) destAddress, sizeof(*destAddress));
       if (sendData < 0) {
           printf("sendto() failed with error code  : %d\n", errno);
           return -1;
       }

       int ACKresult = rudp_receiveACK(socket,srcAddress);

       if(ACKresult != -2) {
           return ACKresult;
       }

       printf("Timeout occurred, sending file again\n");
   }
}

// Receiver Functions
/**
 * function that sends ACK to destAddress
 * @param socket
 * @param destAddress
 * @return -1: failed\n 1: successful
 */
int rudp_sendACK(int socket,struct sockaddr_in* destAddress){
//    int i=0;
//    printf("send ACK?\n");
//    scanf("%d",&i);
    Message ACK;
    memset(&ACK,0,sizeof(Message));
    ACK.length=0;
    strcpy(ACK.data,"");
    ACK.checksum = 0;
    ACK.flags = 'A';
    int sendACK = sendto(socket, &ACK, sizeof(Message), 0, (struct sockaddr *)destAddress, sizeof(*destAddress));
    if (sendACK == -1) {
        printf("sendto() failed with error code  : %d\n", errno);
        return -1;
    }
    return 1;
}

/**
 * recieve data from sender and sends ack
 * @param socket
 * @param senderAddress
 * @return -1: failure, 0: exit message, -2: EOF, -3:bad packet >0:Data
 */
int rudp_receive(int socket,struct sockaddr_in* senderAddress){
    Message buffer;
    memset(&buffer,0,sizeof(Message));

    // Recieve Data
    socklen_t senderAddressLen = sizeof(*senderAddress);
    int recvData = recvfrom(socket, &buffer, sizeof(buffer), 0, (struct sockaddr *)senderAddress,&senderAddressLen);
    if (recvData < 0){
        printf("recvfrom() failed with error code : %d", errno);
        close(socket);
        return -1;
    }

    int ACKResult;

    switch (buffer.flags) {

        case 'S':
            printf("Connection request received, sending ACK.\n");
            char clientIPAddrReadable[32] = {'\0'};
            inet_ntop(AF_INET, &senderAddress->sin_addr, clientIPAddrReadable, sizeof(clientIPAddrReadable));
            ACKResult = rudp_sendACK(socket,senderAddress);
            if(ACKResult < 0){return -1;}
            return 1;

        case 'F':
            printf("Sender sent exit message.\n");
            ACKResult = rudp_sendACK(socket,senderAddress);
            if(ACKResult < 0){return -1;}
            printf("ACK Sent. Exiting...\n");
            return 0;

        case 'M':
            if(buffer.checksum == calculate_checksum(buffer.data,buffer.length)){
                ACKResult = rudp_sendACK(socket,senderAddress);
                if(ACKResult < 0){return -1;}
                if(buffer.data[0]==EOF){
                    printf("File transfer completed.\n");
                    printf("ACK Sent.\n");
                    return -2;
                }

                return buffer.length;
            }
            else{
                printf("Packet is not OK not sending ACK\n");
                return -3;
            }
    }
    return -1;
}

/*
* @brief A checksum function that returns 16 bit checksum for data.
* @param data The data to do the checksum for.
* @param bytes The length of the data in bytes.
* @return The checksum itself as 16 bit unsigned number.
* @note This function is taken from RFC1071, can be found here:
* @note https://tools.ietf.org/html/rfc1071
* @note It is the simplest way to calculate a checksum and is not very strong.
* However, it is good enough for this assignment.
* @note You are free to use any other checksum function as well.
* You can also use this function as such without any change.
*/
unsigned short int calculate_checksum(void *data, unsigned int bytes) {
    unsigned short int *data_pointer = (unsigned short int *)data;
    unsigned int total_sum = 0;
// Main summing loop
    while (bytes > 1) {
        total_sum += *data_pointer++;
        bytes -= 2;
    }
// Add left-over byte, if any
    if (bytes > 0)
        total_sum += *((unsigned char *)data_pointer);
// Fold 32-bit sum to 16 bits
    while (total_sum >> 16)
        total_sum = (total_sum & 0xFFFF) + (total_sum >> 16);
    return (~((unsigned short int)total_sum));
}
