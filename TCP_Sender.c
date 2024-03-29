#include "Random_Data_Generator.h"
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_RECEIVER_PORT 4444
#define DEFAULT_RECEIVER_IP_ADDRESS "127.0.0.1"
#define DEFAULT_CC_ALGORITHM "reno"
#define FILE_SIZE 3000000




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


    //Create a TCP socket between the Sender and the Receiver.
    //Create socket
    int sender_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    // if cant create socket return error
    if(sender_socket == -1){
                printf("socket() failed");
        return -1;
    }

    
    //setting TCP_CONGECTION
    int sockOpt = setsockopt(sender_socket, IPPROTO_TCP, TCP_CONGESTION, algo,strlen(algo)+1);
    
    if (sockOpt ==-1) {
        printf("setsockopt() failed with error code : %d", errno);
        close(sender_socket);
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
        printf("inet_pton() failed");
        return -1;
    }

    // connect to receiver
    printf("Try to connect to receiver...\n");
    int connection = connect(sender_socket, (struct sockaddr *) &receiver_address, sizeof(receiver_address));
    // if connection failed close socket and return error
    if (connection == -1) {
        perror("connect(2)");
        close(sender_socket);
        return -1;
        }
    printf("Connection successful!\n");
    
    
    int choice=1;
    
    while(choice){
        // Generate a random file
        char * file=util_generate_random_data(FILE_SIZE);

        // Send the file
        int send_msg1 = send(sender_socket, file, sizeof(char)*strlen(file), 0);
            
        if (send_msg1 == -1) {
            printf("send() failed");
                close(sender_socket);
             return -1;
        }
        //let the receiver know that the file is over
        char endOfFile[1] = {EOF};
        int send_eof=send(sender_socket,endOfFile,2,0);
        if(send_eof < 0){
            printf("send() failed\n");
            return -1;
        }

        free(file);
        
         //User decision: Send the file again?
        printf("Resend file press 1 if yes 0 if not?\n");
        scanf("%d", &choice);

        // If send the file send 'y' if not send 'n'
        if(choice==1) {
            int sendsync=send(sender_socket,"y",strlen("y"),0);
            if(sendsync<0){
            printf("send() failed\n");
                return -1;
            }
        }
        else{
            int sendsync=send(sender_socket,"n",strlen("n"),0);
            if(sendsync<0){
                printf("send() failed\n");
            }
            break;
        }
    }
    //Send an exit message to the receiver
    char exitMessage[] = " Exit";
    int send_exit=send(sender_socket,exitMessage,strlen(exitMessage),0);
    if(send_exit < 0){
        close(sender_socket);
        return -1;
    }
        
        
    // Close the TCP connection
    close(sender_socket);     

    return 0;
}