
all: TCP_Sender TCP_Receiver

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

clean:
	rm *.o
	rm TCP_Sender
	rm TCP_Receiver

