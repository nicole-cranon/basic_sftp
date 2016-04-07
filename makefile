# nicole cranon <nicole.cranon@ucdenver.edu>
# csci 4761, spring 2016
# Introduction to Computer Networks 
# makefile

CC=g++
CFLAGS=-c -Wall -g

server: server.o 
	$(CC) server.o -o server

client: client.o
	$(CC) client.o -o client

cleanserver:
	rm *.o -f; rm server -f

cleanclient:
	rm *.o -f; rm client -f