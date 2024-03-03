
all: TCP_Sender TCP_Receiver RUDP_Receiver RUDP_Sender

TCP_Sender: Random_Data_Generator.o TCP_Sender.o
	gcc -Wall -g Random_Data_Generator.o TCP_Sender.o -o TCP_Sender

TCP_Receiver : TCP_Receiver.o List.o
	gcc -Wall -g TCP_Receiver.o List.o -o TCP_Receiver

TCP_Receiver.o: TCP_Receiver.c List.h
	gcc -Wall -g -c TCP_Receiver.c

List.o: List.c List.h
	gcc -Wall -g -c List.c

Random_Data_Generator.o: Random_Data_Generator.c Random_Data_Generator.h
	gcc -Wall -g -c Random_Data_Generator.c

TCP_Sender.o: TCP_Sender.c Random_Data_Generator.h
	gcc -Wall -g -c TCP_Sender.c



RUDP_Sender: RUDP_Sender.o Random_Data_Generator.o RUDP.o
	gcc -Wall RUDP_Sender.o RUDP.o Random_Data_Generator.o -o RUDP_Sender

RUDP_Receiver: RUDP_Receiver.o RUDP.o List.o
	gcc -Wall RUDP_Receiver.o RUDP.o List.o  -o RUDP_Receiver

RUDP_Receiver.o: RUDP_Receiver.c List.h RUDP_API.h Random_Data_Generator.h
	gcc -Wall -c RUDP_Receiver.c

RUDP_Sender.o: RUDP_Sender.c RUDP_API.h Random_Data_Generator.h
	gcc -Wall -c RUDP_Sender.c

RUDP.o: RUDP.c RUDP_API.h
	gcc -Wall -c RUDP.c

clean:
	rm *.o
	rm TCP_Sender
	rm TCP_Receiver
	rm RUDP_Receiver
	rm RUDP_Sender

