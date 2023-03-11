CXX =g++
CFLAGS = -Wall -g3 -DFMT_HEADER_ONLY
INCLUDE=-I../include/
LIBS=-L../ -lTarHelper -larchive -lfmt 

all:extract compress

extract:
	$(CXX) -o ../extract extract.cpp $(INCLUDE) $(LIBS) $(CFLAGS)

compress:
	$(CXX) -o ../compress compress.cpp $(INCLUDE) $(LIBS) $(CFLAGS)