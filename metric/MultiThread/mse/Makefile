CC=g++
CFLAGS = `pkg-config --cflags opencv` -I /usr/include/CL/
LIBS = `pkg-config --libs opencv` -L /usr/lib64/ -l OpenCL

all: metric_cl

metric_cl: metric_cl.o 
	$(CC) $(LIBS) metric_cl.o -o metric_cl  

metric_cl.o: metric_cl.cpp
	$(CC) $(CFLAGS) -c metric_cl.cpp -pg
clean:
	rm -rf metric_cl *.o

