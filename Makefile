CXX =g++
CFLAGS = -Wall -g3
INCLUDE=-Iinclude/
LIBS=-larchive

.PHONY:all TarMaker.o Archive.o utils.o

all: TarMaker.o Archive.o utils.o
	ar rcs TarHelper.a intermediate/Archive.o intermediate/TarMaker.o intermediate/utils.o
TarMaker.o:
	$(CXX) -c -o intermediate/$@ src/TarMaker.cpp $(CFLAGS) $(INCLUDE) $(LIBS)

Archive.o:
	$(CXX) -c -o intermediate/$@ src/Archive.cpp $(CFLAGS) $(INCLUDE) $(LIBS)

utils.o:
	$(CXX) -c -o intermediate/$@ src/utils.cpp $(CFLAGS) $(INCLUDE) $(LIBS)