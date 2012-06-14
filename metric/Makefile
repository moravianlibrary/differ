CC=g++
CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

all: metric

metric: metric.o 
	$(CC) $(LIBS) metric.o -o metric -pg 

metric.o: metric.cpp
	$(CC) $(CFLAGS) -c metric.cpp -pg
clean:
	rm -rf compare *.o

