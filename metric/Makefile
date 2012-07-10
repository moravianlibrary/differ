APP_PREFIX=metric
INCLUDE_DIR=$(HOME)/Project/opcv/include/opencv
LIB_DIR=$(HOME)/Project/opcv/lib
CC = $(APP_PREFIX)-gcc
CPP = g++
LD= $(APP_PREFIX)-ld
CXXFLAGS = -I$(INCLUDE_DIR)
CXXFLAGS+= -g -pg -fsigned-char
LDFLAGS = -g -pg -static -L$(LIB_DIR) -lcv -lhighgui -lcxcore -lml -lcvaux -lm -lstdc++ -lpthread -ldl -fopenmp -ltiff -ljpeg -lpng -ljasper -lz
all: metric.o
	$(CPP) metric.o $(LDFLAGS) -o metric
metric.o:
	$(CPP) $(CXXFLAGS) -c metric.cpp
clean:
	rm -f metric metric.o
