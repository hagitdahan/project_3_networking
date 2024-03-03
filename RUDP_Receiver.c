#include "RUDP_API.h"
#include "List.h"
//#include "List.c"
//#include "RUDP.c"


int main(int argc,char** argv) {
    int port = DEFAULT_PORT;
    for(int i=0;i<argc;i++) {
        if (strcmp("-p", argv[i]) == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
        }
    }

    printf("Working on Port: %d\n",port);
    List* list = List_alloc();
    //1) Create a UDP connection between the Receiver and the Sender.
    int receiver_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (receiver_socket == -1) {
        printf("Could not create socket : %d\n", errno);
        return -1;
    }

    struct sockaddr_in receiverAddress;
    memset((char *)&receiverAddress, 0, sizeof(receiverAddress));
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(port);
    int ret = inet_pton(AF_INET, (const char *)DEFAULT_IP, &(receiverAddress.sin_addr));
    if (ret <= 0) {
        printf("inet_pton() failed\n");
        return -1;
    }

    struct sockaddr_in senderAddress;
    memset((char *)&senderAddress, 0, sizeof(senderAddress));

    printf("Starting Receiver...\n");

    int bindResult = bind(receiver_socket, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));
    if (bindResult == -1) {
        printf("bind() failed with error code : %d\n", errno);
        close(receiver_socket);
        return -1;
    }

    //2) Get a connection from the sender, by the custom RUDP protocol you’ve built.
    printf("Waiting for RUDP Connection...\n");
    int recvResult = rudp_receive(receiver_socket, &senderAddress);
    if(recvResult<=0){return -1;}
    printf("Sender connected, beginning to receive file...\n");

    //3) Receive the file, measure the time it took and save it.
    int exitMessage = 0;

    while(!exitMessage) {

        size_t totalBytes = 0;
        clock_t start = clock();

        while (1) {

            int receiveResult = rudp_receive(receiver_socket, &senderAddress);
            if(receiveResult > 0){totalBytes+=receiveResult;}

            if (receiveResult == -1) { return -1; }
            else if (receiveResult == 0) {
                exitMessage = 1;
                break; }
            else if (receiveResult == -2) {
                break; }
        }

        clock_t end = clock();
        if(!exitMessage) {
            double interval_in_milliseconds = ((double)(end - start)/ CLOCKS_PER_SEC)*1000.0;
            List_insertLast(list,interval_in_milliseconds,totalBytes);
            printf("Waiting for Sender response...\n");
            //4) Wait for Sender response:
            //a. If Sender resends the file, go back to step 3.
            //b. If Sender sends exit message, go to step 5.
        }
    }
    //5) Print out the times (in milliseconds), and the average
    //bandwidth for each time the file was received.
    printf("----------------------------------\n");
    printf("-         * Statistics *         -\n");
    float avarage_time = List_avarage_time(list);
    float avarage_bandwidth = List_avarage_bandwidth(list);
    List_print(list);
    //6) Calculate the average time and the total average
    //bandwidth.
    printf("- \n");
    printf("- Avarage time: %.3fms\n",avarage_time);
    printf("- Avarage bandwidth: %.3fMB/s\n",avarage_bandwidth);
    free(list);

    //7) Exit.
    close(receiver_socket);
    return 0;
}