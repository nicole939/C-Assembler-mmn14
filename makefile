assembler: linked_list.o file_reading.o functions.o main.o 
	gcc -g -ansi -Wall -pedantic linked_list.o file_reading.o functions.o main.o  -o assembler
main.o: main.c file_reading.h
	gcc -c -ansi -Wall -pedantic main.c -o main.o
file_reading.o: file_reading.c linked_list.h functions.h file_reading.h
	gcc -c -ansi -Wall -pedantic file_reading.c -o file_reading.o
linked_list.o: linked_list.c linked_list.h functions.h
	gcc -c -ansi -Wall -pedantic linked_list.c -o linked_list.o
functions.o: functions.c
	gcc -c -ansi -Wall -pedantic functions.c -o functions.o
