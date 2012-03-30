USE_KAKADU=0
CC=g++
CFLAGS=-ltiff -lmhash
ifeq ($(USE_KAKADU),1)
LIBKDU=/home/differ/kakadu/v6_3_1-00857N/lib/Linux-x86-64-gcc/libkdu_v63R.so
KDU_LIBS=/home/differ/kakadu/v6_3_1-00857N/apps/make
endif
CFLAGS+=-g3
ifeq ($(USE_KAKADU),1)
CFLAGS+=-I/home/differ/v6_3_1-00857N/managed/all_includes/
endif

ifeq ($(USE_KAKADU),1)
LINKFLAGS+= /home/xrosecky/v6_3_1-00857N/apps/make/jpx.o /home/xrosecky/v6_3_1-00857N/apps/make/jp2.o
LINKFLAGS+= /home/xrosecky/v6_3_1-00857N/apps/make/kdu_stripe_decompressor.o
endif

ifeq ($(USE_KAKADU),1)
LINKFLAGS+=-lpthread -ltiff -lmhash -ljpeg /home/xrosecky/v6_3_1-00857N/lib/Linux-x86-64-gcc/libkdu_v63R.so
else
LINKFLAGS+=-lpthread -ltiff -lmhash -ljpeg
endif

all: compare

compare: compare.o xmlwriter.o
	$(CC) $(LINKFLAGS) compare.o xmlwriter.o -o compare

xmlwriter.o: xmlwriter.cc
	$(CC) -c xmlwriter.cc

compare.o: compare.cc
	$(CC) $(CFLAGS) -c compare.cc

clean:
	rm -rf compare *.o

