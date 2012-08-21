CC=g++
CFLAGS = `pkg-config --cflags opencv` -I /usr/include/CL/
LIBS = `pkg-config --libs opencv` -L /usr/lib64/ -l OpenCL

SOURCES=metric.cpp core.cpp host_program_opencl.cpp similarityMetric.cpp mse.cpp psnr.cpp ssim.cpp msssim.cpp iqi.cpp mse_opencl.cpp ssim_opencl.cpp msssim_opencl.cpp iqi_opencl.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=metric

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS) 
		$(CC) $(LIBS) $(OBJECTS) -o $@

$(OBJECTS): $(SOURCES)
		$(CC) $(CFLAGS)  -c $(SOURCES)

clean:
		rm -rf metric *.o

