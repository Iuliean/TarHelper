CXX =g++
CFLAGS = -Wall -g3
INCLUDE=-Iinclude/
LIBS=

.PHONY:all TarMaker.o Archive.o utils.o

all: TarMaker.o Archive.o utils.o
	ar rcs libTarHelper.a intermediate/Archive.o intermediate/TarMaker.o intermediate/utils.o

examples:all
	$(MAKE) -C example/ all
	./compress
	./extract

TarMaker.o:
	$(CXX) -c -o intermediate/$@ src/TarMaker.cpp $(CFLAGS) $(INCLUDE) $(LIBS)

Archive.o:
	$(CXX) -c -o intermediate/$@ src/Archive.cpp $(CFLAGS) $(INCLUDE) $(LIBS)

utils.o:
	$(CXX) -c -o intermediate/$@ src/utils.cpp $(CFLAGS) $(INCLUDE) $(LIBS)