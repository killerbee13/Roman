CXXFLAGS=-g --std=c++11 -Wall -D_DEBUG
LDFLAGS=-lgmpxx -lgmp -ltecla -lncurses

objects=main.o input.o convert.o

Roman: $(objects)
	$(CXX) -o Roman $(objects) $(LDFLAGS)

input.o: input.cpp input.h FRC.h

convert.o: convert.cpp roman.h

main.o: main.cpp strings.h input.h roman.h FRC.h

objectsO=mainO.o inputO.o convertO.o
optsO=--std=c++11 -Wall -O3 -msse2

opt: $(objectsO)
	$(CXX) -o RomanO $(objectsO) $(LDFLAGS)

inputO.o: input.cpp input.h FRC.h
	$(CXX) $(optsO) -c -o inputO.o input.cpp

convertO.o: convert.cpp roman.h FRC.h
	$(CXX) $(optsO) -c -o convertO.o convert.cpp

mainO.o: main.cpp strings.h input.h roman.h FRC.h
	$(CXX) $(optsO) -c -o mainO.o main.cpp

clean: 
	rm $(objects) Roman $(objectsO) RomanO