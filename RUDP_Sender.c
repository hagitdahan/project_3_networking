#include "RUDP_API.h"
#include "Random_Data_Generator.h"
//#include "RUDP.c"
#define FILE_SIZE 200000

int main(int argc,char** argv) {
    int port = DEFAULT_PORT;
    char* receiver_ip = DEFAULT_IP;
    for(int i=0;i<argc;i++) {
        if (strcmp("-p", argv[i]) == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
        } else if (strcmp("-ip", argv[i]) == 0) {
            size_t len = strlen(argv[i + 1]);
           char* new_ip = (char*) malloc(sizeof(char)*len+1);
            strcpy(new_ip, argv[i + 1]);
            receiver_ip = new_ip;
        }
    }

    printf("Receiver IP: %s, Port: %d\n",receiver_ip,port);
    //1) Read the created file.
    // char *file = util_generate_random_data(2000000);
    // int fileSize = strlen(file);
    // printf("%d",fileSize);
    char * file=util_generate_random_data(FILE_SIZE);
    int fileSize=FILE_SIZE;


    //2) Create a UDP socket between the Sender and the Receiver.
    int sender_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sender_socket < 0){
        printf("Could not create a socket: %d",errno);
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    setsockopt(sender_socket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));

    struct sockaddr_in receiverAddress;
    memset(&receiverAddress, 0, sizeof(receiverAddress));
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(port);
    int rval = inet_pton(AF_INET, (const char *)receiver_ip, &receiverAddress.sin_addr);
    if (rval <= 0) {
        printf("inet_pton() failed");
        return -1;
    }
    struct sockaddr_in fromAddress;
    memset((char *)&fromAddress, 0, sizeof(fromAddress));

    printf("Sending connect message to receiver\n");
    int connectionResult = rudp_connect(sender_socket,&receiverAddress,&fromAddress);
    if(connectionResult <= 0){
        printf("Connction to Receiver Failed\n");
        return -1;
    }
    printf("got ACK connection successful, sending file\n");

    //3) Send the file via the RUDP protocol
    int userChoice = 1;
    while(userChoice) {
        printf("Sending file...\n");

        char data[MESSAGE_SIZE];
        int i;
        for(i=0;i+MESSAGE_SIZE<fileSize;i+=MESSAGE_SIZE) {
            strncpy(data,file+i,MESSAGE_SIZE);
            rudp_sendData(sender_socket, data, &receiverAddress, &fromAddress);
        }
        char* lastData = (char*) malloc(sizeof (char)*fileSize-i);
        strncpy(lastData, file+i,(fileSize-i));
        rudp_sendData(sender_socket, lastData, &receiverAddress, &fromAddress);
        free(lastData);
        char endOfFile[1] = {EOF};
        rudp_sendData(sender_socket, endOfFile, &receiverAddress, &fromAddress);

        //4) User decision: Send the file again?
        //a. If yes, go back to step 3.
        //b. If no, continue to step 5.
        printf("Resend the file?\n");
        scanf("%d",&userChoice);
        if(userChoice == 1){
            int sendChoice = rudp_sendData(sender_socket,"yes",&receiverAddress, &fromAddress);
            if(sendChoice < 0){
                printf("send() failed\n");
                return -1;
            }
        }

        if(userChoice == 0){
            int sendChoice = rudp_sendData(sender_socket,"no",&receiverAddress, &fromAddress);
            if(sendChoice < 0){
                printf("send() failed\n");
                return -1;
            }
        }
    }


    //5) Send an exit message to the receiver.
    printf("Sending disconnect message to receiver\n");
    int diconnectionResult = rudp_disconnect(sender_socket,&receiverAddress,&fromAddress);
    if(diconnectionResult<=0){
        printf("Disconnction to Receiver Failed\n");
        return -1;
    }
    printf("Got Ack from receiver, Exiting...\n");
    //6) Close the TCP connection.
    close(sender_socket);
    //7) Exit.
    return 0;
}