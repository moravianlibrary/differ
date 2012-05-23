CC=g++
CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

all: metric

metric: metric.o 
	$(CC) $(LIBS) metric.o -o metric 

metric.o: metric.cpp
	$(CC) $(CFLAGS) -c metric.cpp
clean:
	rm -rf compare *.o

