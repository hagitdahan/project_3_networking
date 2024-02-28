#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "List.h"


#define DEFAULT_RECEIVER_PORT 4444
#define DEFAULT_CC_ALGORITHM "reno"
#define BUFFER_SIZE 4096

int main(int argc,char** argv) {
    int reciver_port = DEFAULT_RECEIVER_PORT;
    char* algo = DEFAULT_CC_ALGORITHM;
    for(int i=0;i<argc;i++){
        if(strcmp("-p",argv[i]) == 0 && i+1<argc){
            reciver_port = atoi(argv[i+1]);
        }
        else if(strcmp("-algo",argv[i]) == 0 && i+1<argc){
            if(strcmp(argv[i+1],"reno") == 0){
                algo = (char*) malloc(sizeof(char)*5);
                if(!algo){
                    return -1;
                }
                strcpy(algo,"reno");

            }
            else if(strcmp(argv[i+1],"cubic")==0){
                algo = (char*) malloc(sizeof(char)*6);
                if(!algo){
                    return -1;
                }
                strcpy(algo,"cubic");
            }
        }
    }
    printf("Algo: %s, Listening Port: %d\n",algo,reciver_port);
    printf("----------Starting Receiver-------------\n");

    List* intervals = List_alloc();
    //Create a TCP connection between the Receiver and the Sender.

    int receiver_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (receiver_socket < 0) {
        printf("socket() failed");
        return 1;
    }

    //Resue the address if the receiver socket on was closed
    int reuseAddr = 1;
    #ifdef TCP_CONGECTION
    int sockOpt = setsockopt(receiver_socket, SOL_SOCKET, TCP_CONGECTION, algo, sizeof(char)*strlen(algo));
    #else
    int sockOpt = setsockopt(receiver_socket, IPPROTO_TCP, SO_REUSEADDR, &reuseAddr, sizeof(int));
    #endif
    if (sockOpt < 0) {
        printf("setsockopt() failed with error code : %d", errno);
        return 1;
    }

    //create receiver address struct and initialize it with 0's
    struct sockaddr_in receiver_address;
    memset(&receiver_address, 0, sizeof(receiver_address));

    //Fill data of sockaddr_in struct
    receiver_address.sin_family = AF_INET;
    receiver_address.sin_addr.s_addr = INADDR_ANY;
    receiver_address.sin_port = htons(reciver_port);

    //bind the receiver socket to port and address
    int socketBind = bind(receiver_socket, (struct sockaddr *) &receiver_address, sizeof(receiver_address));
    if (socketBind < 0) {
        printf("bind() failed");
        close(receiver_socket);
        return -1;
    }

    // listen for 1 connection
    int socketListen = listen(receiver_socket, 1);
    if (socketListen < 0) {
        printf("listen() failed");
        close(receiver_socket);
        return -1;
    }

    struct sockaddr_in sender_address;
    socklen_t sender_address_len = sizeof(sender_address);



    memset(&sender_address, 0, sizeof(sender_address));
    sender_address_len = sizeof(sender_address);

    struct timeval start,stop;

    // Loop for getting the sender requests to send file
    while(1) {

        printf("Waiting for TCP connection...\n");
        //Get a connection from the sender.
        int sender_socket = accept(receiver_socket, (struct sockaddr *) &sender_address, &sender_address_len);
        if (sender_socket < 0) {
            printf("accept() failed");
            close(receiver_socket);
            return -1;
        }

        printf("Sender connected, beginning to receive file...\n");
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        long int amount_of_data=0;
        gettimeofday(&start, NULL);

        // Loop for a smaller buffer then the file
        while (1) {

            int dataReceived = recv(sender_socket, buffer, BUFFER_SIZE, 0);
            amount_of_data += dataReceived;
            memset(buffer, 0, BUFFER_SIZE);

            if (dataReceived < 0) {
                printf("recv() failed");
                close(receiver_socket);
                close(sender_socket);
                return -1;
            }

            // if sender stopped sending data, the file transfer completed
            if (dataReceived == 0) {
                close(sender_socket);
                break;
            }
        }

        gettimeofday(&stop, NULL);

        // if no data is sent then sender closed the connection instead of passing data
        if(amount_of_data == 0){
            printf("Sender sent an exit messege\n");
            break;
        }

        printf("File transfer completed. size: %ld\n",amount_of_data);
        float interval = (float) ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec) / 1000;
        List_insertLast(intervals, interval, amount_of_data);
    }

    //Print out the times (in milliseconds), and the average bandwidth for each time the file was received.

    printf("----------------------------------\n");
    printf("-         * Statistics *         -\n");
    float avarage_time = List_avarage_time(intervals);
    float avarage_bandwidth = List_avarage_bandwidth(intervals);
    List_print(intervals);
    //Calculate the average time and the total average bandwidth.
    printf("- \n");
    printf("- Avarage time: %fms\n",avarage_time);
    printf("- Avarage bandwidth: %fMB/s\n",avarage_bandwidth);

    //Exit
    List_free(intervals);
    close(receiver_socket);
    printf("Receiver end.\n");
    return 0;
}