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
#include "List.h"
#include <sys/time.h>


#define DEFAULT_RECEIVER_PORT 4444
#define DEFAULT_CC_ALGORITHM "reno"
#define BUFFER_SIZE 4096


int main(int argc,char** argv) {
    struct timeval start, end;
    double cpu_time_use;
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

    // //Resue the address if the receiver socket on was closed
    // int reuseAddr = 1;
    // #ifdef TCP_CONGECTION
    int sockOpt = setsockopt(receiver_socket, IPPROTO_TCP, TCP_CONGESTION, algo, sizeof(char)*strlen(algo));
    // #else
    // int sockOpt = setsockopt(receiver_socket, IPPROTO_TCP, SO_REUSEADDR, &reuseAddr, sizeof(int));
    // #endif
    if (sockOpt < 0) {
         printf("setsockopt() failed with error code : %d", errno);
         close(receiver_socket);
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
    if (socketBind <0) {
        printf("bind() failed");
        close(receiver_socket);
        return -1;
    }

    // listen for 1 connection
    int socketListen = listen(receiver_socket, 1);
    if (socketListen ==-1) {
        printf("listen() failed");
        close(receiver_socket);
        return -1;
    }

    struct sockaddr_in sender_address;
    socklen_t sender_address_len = sizeof(sender_address);



    memset(&sender_address, 0, sizeof(sender_address));
    sender_address_len = sizeof(sender_address);

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
    int run=1;

    // Loop for getting the sender requests to send file
    int measureTime = 1;
    while(run) {
        int amount_of_data=0;
        if(measureTime){gettimeofday(&start,NULL);}
        int byte_recv=1;

        // Loop for a smaller buffer then the file
        while (byte_recv) {
            byte_recv= recv(sender_socket, buffer, BUFFER_SIZE, 0);
            amount_of_data += byte_recv;
        
            if(buffer[byte_recv-2]==EOF){
                amount_of_data-=2;
                break;
            }
            if(byte_recv==0){//strncmp(buffer,"DONE",sizeof("DONE"))==0){
                //amount_of_data-=sizeof("DONE")-1;
                break;
            } 
            
            if(strcmp(buffer,"Exit")==0){
                run=0;
                break;
            }
            memset(buffer, 0, BUFFER_SIZE);
            //printf("Checking if.. %s\n",buffer);
            
        }
        
        gettimeofday(&end,NULL);
        
        if (byte_recv < 0) {
            printf("recv() failed");
            close(receiver_socket);
            close(sender_socket);
            return -1;
        }

        float interval_in_seconds = (float)(end.tv_sec - start.tv_sec) + (float)(end.tv_usec - start.tv_usec)/1000000;
    
        if(run ){
            List_insertLast(intervals, interval_in_seconds, amount_of_data);
            //printf("File transfer completed. size: %d\n",amount_of_data);
        }

        memset(buffer,0,strlen(buffer));
        
        int recvChocie = recv(sender_socket, buffer, 4, 0);
        printf("%d",recvChocie);
        if(recvChocie < 0){
            printf("recv() failed;\n");
            return -1;
        }
        
        if(recvChocie == 2){
            printf("stop measure time\n");
            measureTime=0;}
        
        printf("%s",buffer);        
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