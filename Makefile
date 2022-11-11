#**********************
#*
#* Progam Name: MP1. Membership Protocol.
#*
#* Current file: Makefile
#* About this file: Build Script.
#* 
#***********************

CFLAGS =  -Wall -g -std=c++17

all: Application

Application: MP1Node.o EmulNet.o Application.o Log.o Member.o
	g++ -o Application MP1Node.o EmulNet.o Application.o Log.o Member.o ${CFLAGS}

MP1Node.o: MP1Node.cpp MP1Node.h Log.h Member.h EmulNet.h GlobalTime.h
	g++ -c MP1Node.cpp ${CFLAGS}

EmulNet.o: EmulNet.cpp EmulNet.h
	g++ -c EmulNet.cpp ${CFLAGS}

Application.o: Application.cpp Application.h Config.h Log.h EmulNet.h Member.h MP1Node.h GlobalTime.h
	g++ -c Application.cpp ${CFLAGS}

Log.o: Log.cpp Log.h EmulNet.h GlobalTime.h
	g++ -c Log.cpp ${CFLAGS}

Member.o: Member.cpp Member.h EmulNet.h
	g++ -c Member.cpp ${CFLAGS}

clean:
	rm -rf *.o Application dbg.log msgcount.log stats.log machine.log
