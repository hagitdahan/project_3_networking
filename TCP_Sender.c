#include "Random_Data_Generator.h"
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_RECEIVER_PORT 4444
#define DEFAULT_RECEIVER_IP_ADDRESS "127.0.0.1"
#define DEFAULT_CC_ALGORITHM "reno"
#define FILE_NAME "File.txt"



int main(int argc,char** argv) {
    int reciver_port = DEFAULT_RECEIVER_PORT;
    char* algo = DEFAULT_CC_ALGORITHM;
    char* reciver_ip = DEFAULT_RECEIVER_IP_ADDRESS;

    for(int i=0;i<argc;i++){
        if(strcmp("-p",argv[i]) == 0 && i+1<argc){
            reciver_port = atoi(argv[i+1]);
        }
        else if(strcmp("-ip",argv[i]) == 0){
            size_t len = strlen(argv[i + 1]);
            reciver_ip = (char*) malloc(sizeof(char)*len+1);
            if(!reciver_ip){return -1;}
            strcpy(reciver_ip, argv[i + 1]);
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

    printf("Algo: %s, Sending to Port: %d, To IP:%s \n",algo,reciver_port,reciver_ip);
    printf("----------Starting Sender-------------\n");


    // 1.Read the created file.
    FILE* file = fopen(FILE_NAME,"r");
    // calculating the size of the file
    fseek(file, 0L, SEEK_END);
    long int filesize = ftell(file);

    //2. Create a TCP socket between the Sender and the Receiver.
    //Create socket
    int sender_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sender_socket < 0){
        printf("socket() failed");
        return -1;
    }

    int reuseAddr = 1;
    #ifdef TCP_CONGECTION
    int sockOpt = setsockopt(sender_socket, SOL_SOCKET, TCP_CONGECTION, algo, sizeof(char)*strlen(algo));
    #else
    int sockOpt = setsockopt(sender_socket, IPPROTO_TCP, SO_REUSEADDR, &reuseAddr, sizeof(int));
    #endif
    if (sockOpt < 0) {
        printf("setsockopt() failed with error code : %d", errno);
        return 1;
    }

    //create receiver address struct and initialize it with 0's
    struct sockaddr_in receiver_address;
    memset(&receiver_address,0,sizeof(receiver_address));

    receiver_address.sin_family = AF_INET;
    // convert port number and set port to socket
    receiver_address.sin_port = htons(reciver_port);
    // convert reciver_ip address and set address to socket
    int set_ip = inet_pton(AF_INET,reciver_ip,&receiver_address.sin_addr);
    // if conversion failed return error
    if (set_ip == -1){
        printf("ibet_pton() failed");
        return -1;
    }

    // connect to receiver
    int connection = connect(sender_socket, (struct sockaddr *) &receiver_address, sizeof(receiver_address));
    // if connection failed close socket and return error
    if (connection == -1) {
        printf("connect() failed");
        close(sender_socket);
        return -1;
    }


    do {
        //3. Send the file
        int send_msg1 = send(sender_socket, file, sizeof(char)*filesize, 0);
        if (send_msg1 < -1) {
            printf("send() failed");
            close(sender_socket);
            return -1;
        }
        printf("Sent %d Bytes\n", send_msg1);
        //4 . User decision: Send the file again?
    } while()
// if user desicion is not to send the file close socket
        else{
            close(sender_socket);
//            printf("Sent Exit messege\n");
            break;
        }

        //User decision: Send the file again?
        printf("Resend file?\n");
        scanf("%d", &resend);

}

    //Exit.
    return 0;
}