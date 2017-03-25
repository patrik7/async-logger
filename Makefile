

main: FileLogger.o main.o
	g++ -g -o main FileLogger.o main.o -pthread -L/usr/lib -lboost_thread -lboost_system

FileLogger.o: FileLogger.cpp FileLogger.hpp Logger.hpp
	g++ -g -c -pthread FileLogger.cpp -o FileLogger.o -lboost_thread -lboost_system

main.o: main.cpp FileLogger.hpp
	g++ -g -c -pthread main.cpp -o main.o

clean:
	rm *.o
